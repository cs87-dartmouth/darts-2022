import numpy as np
from mathutils import Matrix
from collections.abc import Iterable
import os


def dummy_color(ctx):
    return ctx.color([0.0, 1.0, 0.3])


def convert_vector_node(ctx, socket):

    if not socket.is_linked or not ctx.enable_mapping:
        return None

    socket = ctx.follow_link(socket)

    def export_coord_node(ctx, socket):
        from_node = socket.from_node
        if from_node.bl_idname != 'ShaderNodeTexCoord':
            raise NotImplementedError(
                f"Unsupported node type: {from_node.bl_idname}. Expecting a 'ShaderNodeTexCoord'")
        if from_node.object is not None:
            ctx.report(
                {'WARNING'}, "Darts does not currently support texture coordinates from other objects. Ignoring.")

        ctx.info(
            f"Writing '{socket.from_socket.name.lower()}' coordinate node")
        return {'coordinate': socket.from_socket.name.lower()}

    def export_mapping_node(ctx, socket):
        from_node = socket.from_node

        params = {'vector type': from_node.vector_type.lower()}
        ctx.info(f"Writing '{params['vector type']}' mapping node.")

        if from_node.inputs['Location'].is_linked or from_node.inputs['Rotation'].is_linked or from_node.inputs['Scale'].is_linked:
            raise NotImplementedError(
                "Location, Rotation, and Scale inputs shouldn't be linked")

        L = from_node.inputs['Location'].default_value
        R = from_node.inputs['Rotation'].default_value
        S = from_node.inputs['Scale'].default_value
        M = Matrix.LocRotScale(L, R, S)
        if M != Matrix.Identity(4):
            params["transform"] = ctx.transform_matrix(M.inverted())

        if not from_node.inputs['Vector'].is_linked:
            raise NotImplementedError(
                f"The node {from_node.bl_idname} should be linked with a 'ShaderNodeTexCoord'")
        params.update(export_coord_node(
            ctx, ctx.follow_link(from_node.inputs['Vector']).links[0]))

        return params

    params = {'type': 'transform'}

    from_node = socket.links[0].from_node
    if from_node.bl_idname == 'ShaderNodeTexCoord':
        params.update(export_coord_node(ctx, socket.links[0]))
    elif from_node.bl_idname == 'ShaderNodeMapping':
        params.update(export_mapping_node(ctx, socket.links[0]))
    else:
        raise NotImplementedError(
            f"Unsupported node type: {from_node.bl_idname}. Expecting either a 'ShaderNodeTexCoord' or a 'ShaderNodeMapping'")

    return params


def add_vector_node_field(ctx, params, socket):
    mapping = convert_vector_node(ctx, socket)
    if mapping is not None:
        params["mapping"] = mapping


def export_texture(ctx, image):
    """
    Return the path to a texture.
    Ensure the image is on disk and of a correct type

    image : The Blender Image object
    """

    texture_exts = {
        'BMP': '.bmp',
        'HDR': '.hdr',
        'JPEG': '.jpg',
        'JPEG2000': '.jpg',
        'PNG': '.png',
        'OPEN_EXR': '.exr',
        'OPEN_EXR_MULTILAYER': '.exr',
        'TARGA': '.tga',
        'TARGA_RAW': '.tga',
    }

    convert_format = {
        'CINEON': 'EXR',
        'DPX': 'EXR',
        'TIFF': 'PNG',
        'IRIS': 'PNG'
    }

    textures_folder = os.path.join(ctx.directory, "textures")
    if image.file_format in convert_format:
        ctx.info(
            f"Image format of '{image.name}' is not supported. Converting it to {convert_format[image.file_format]}.")
        image.file_format = convert_format[image.file_format]
    original_name = os.path.basename(image.filepath)
    # Try to remove extensions from names of packed files to avoid stuff like 'Image.png.001.png'
    if original_name != '' and image.name.startswith(original_name):
        base_name, _ = os.path.splitext(original_name)
        name = image.name.replace(
            original_name, base_name, 1)  # Remove the extension
        name += texture_exts[image.file_format]
    else:
        name = f"{image.name}{texture_exts[image.file_format]}"
    if ctx.write_texture_files:
        target_path = os.path.join(textures_folder, name)
        if not os.path.isdir(textures_folder):
            os.makedirs(textures_folder)
        old_filepath = image.filepath
        image.filepath_raw = target_path
        image.save()
        image.filepath_raw = old_filepath
    return f"textures/{name}"


def convert_image_texture_node(ctx, out_socket):
    node = out_socket.node
    params = {
        'type': 'image'
    }

    # get the relative path to the copied texture from the full path to the original texture
    params['filename'] = export_texture(ctx, node.image)

    if node.image.colorspace_settings.name in ['Non-Color', 'Raw', 'Linear']:
        # non color data, tell Darts not to apply gamma conversion to it
        params['raw'] = True
    elif node.image.colorspace_settings.name != 'sRGB':
        ctx.report(
            {'WARNING'}, f"Texture '{node.image}' uses {node.image.colorspace_settings.name} colorspace. Darts only supports sRGB textures for color data.")

    if out_socket.name == "Color":
        params['output'] = "color"
    elif out_socket.name == "Alpha":
        params['output'] = "alpha"
    else:
        raise NotImplementedError(
            f"Unrecognized output socket '{out_socket.name}' on image texture node")

    params['interpolation'] = node.interpolation.lower()

    if node.extension == 'CLIP':
        ctx.report(
            {'WARNING'}, f"'CLIP' extension mode behaves differently in Blender than in Darts.")
    modes = {"REPEAT": "repeat", "EXTEND": "CLAMP", "CLIP": "black"}
    params['wrap mode x'] = params['wrap mode y'] = modes[node.extension]

    if node.projection != 'FLAT':
        ctx.report(
            {'WARNING'}, f"Darts only supports Blender's 'FLAT' project mode. Ignoring {node.projection}.")

    add_vector_node_field(ctx, params, node.inputs['Vector'])

    return params


def convert_checker_texture_node(ctx, out_socket):
    if not ctx.enable_checker:
        return dummy_color(ctx)

    node = out_socket.node
    if node.inputs['Scale'].is_linked:
        raise NotImplementedError(
            "Textured 'Scale' values are not supported")
    params = {
        'type': 'checker',
        'scale': 1.0 / node.inputs['Scale'].default_value,
        'odd': convert_texture_node(ctx, node.inputs['Color1']),
        'even': convert_texture_node(ctx, node.inputs['Color2']),
    }

    add_vector_node_field(ctx, params, node.inputs['Vector'])

    return params


def convert_brick_texture_node(ctx, out_socket):
    if not ctx.enable_brick:
        return dummy_color(ctx)

    node = out_socket.node
    params = {
        'type': 'brick',
        'offset': node.offset,
        'offset frequency': node.offset_frequency,
        'squash': node.squash,
        'squash frequency': node.squash_frequency,
        'color1': convert_texture_node(ctx, node.inputs['Color1']),
        'color2': convert_texture_node(ctx, node.inputs['Color2']),
        'mortar': convert_texture_node(ctx, node.inputs['Mortar']),
        'mortar size': convert_texture_node(ctx, node.inputs['Mortar Size']),
        'mortar smooth': convert_texture_node(ctx, node.inputs['Mortar Smooth']),
        'bias': convert_texture_node(ctx, node.inputs['Bias']),
        'brick width': convert_texture_node(ctx, node.inputs['Brick Width']),
        'row height': convert_texture_node(ctx, node.inputs['Row Height']),
        'output': 'float' if out_socket.name == 'Fac' else 'color'
    }

    add_vector_node_field(ctx, params, node.inputs['Vector'])

    return params


def convert_wave_texture_node(ctx, out_socket):
    if not ctx.enable_wave:
        return ctx.color(0.5)

    node = out_socket.node
    profiles = {"SIN": "sine", "SAW": "saw", "TRI": "triangle"}
    params = {
        'type': 'wave',
        'wave type': node.wave_type.lower(),
        'direction': node.bands_direction.lower() if node.wave_type == 'BANDS' else node.rings_direction.lower(),
        'profile': profiles[node.wave_profile],
        'scale': convert_texture_node(ctx, node.inputs['Scale']),
        'distortion': convert_texture_node(ctx, node.inputs['Distortion']),
        'detail': convert_texture_node(ctx, node.inputs['Detail']),
        'detail scale': convert_texture_node(ctx, node.inputs['Detail Scale']),
        'detail roughness': convert_texture_node(ctx, node.inputs['Detail Roughness']),
        'phase offset': convert_texture_node(ctx, node.inputs['Phase Offset']),
    }

    add_vector_node_field(ctx, params, node.inputs['Vector'])

    return params


def convert_noise_texture_node(ctx, out_socket):
    if not ctx.enable_noise:
        return ctx.color(0.5)

    node = out_socket.node
    dims = {"1D": 1, "2D": 2, "3D": 3, "4D": 4}
    params = {
        'type': 'noise',
        'scale': convert_texture_node(ctx, node.inputs['Scale']),
        'detail': convert_texture_node(ctx, node.inputs['Detail']),
        'roughness': convert_texture_node(ctx, node.inputs['Roughness']),
        'distortion': convert_texture_node(ctx, node.inputs['Distortion']),
        'dimensions': dims[node.noise_dimensions],
        'output': 'float' if out_socket.name == 'Fac' else 'color'
    }

    if dims[node.noise_dimensions] == 4:
        params['w'] = convert_texture_node(ctx, node.inputs['W'])

    add_vector_node_field(ctx,
                          params, node.inputs['Vector'] if dims[node.noise_dimensions] != 1 else node.inputs['W'])

    return params


def convert_layer_weight_node(ctx, out_socket):
    if not ctx.enable_layer_weight:
        return ctx.color(0.5)

    node = out_socket.node
    if out_socket.name == "Fresnel":
        if node.inputs['Blend'].is_linked:
            ctx.report(
                {'WARNING'}, "Textured Blend values for Fresnel are not currently supported in Darts. Using the default value instead.")
        return {
            'type': 'fresnel',
            'ior': 1.0 / max(1.0 - node.inputs['Blend'].default_value, 1e-5)
        }
    elif out_socket.name == "Facing":
        return {
            'type': 'facing',
            'blend': convert_texture_node(ctx,
                                          node.inputs['Blend'])
        }
    else:
        raise NotImplementedError(
            "Unsupported Layer Weight type, expecting either 'Fresnel' or 'Facing'")


def convert_mix_rgb_node(ctx, out_socket):
    if not ctx.enable_mix_rgb:
        return dummy_color(ctx)

    node = out_socket.node
    return {
        'type': 'mix',
        'blend type': node.blend_type.lower().replace('_', ' '),
        'clamp': node.use_clamp,
        'color1': convert_texture_node(ctx, node.inputs['Color1']),
        'color2': convert_texture_node(ctx, node.inputs['Color2']),
    }


def convert_clamp_node(ctx, out_socket):
    # if not ctx.enable_mix_rgb:
    #     return dummy_color(ctx)

    node = out_socket.node
    return {
        'type': 'clamp',
        'clamp type': node.clamp_type.lower(),
        'value': convert_texture_node(ctx, node.inputs['Value']),
        'min': convert_texture_node(ctx, node.inputs['Min']),
        'max': convert_texture_node(ctx, node.inputs['Max']),
    }


def convert_fresnel_node(ctx, out_socket):
    if not ctx.enable_fresnel:
        return ctx.color(0.5)

    node = out_socket.node
    if node.inputs['IOR'].is_linked:
        ctx.report(
            {'WARNING'}, "Textured IOR values are not supported in Darts. Using the default value instead.")

    return {
        'type': 'fresnel',
        'ior': node.inputs['IOR'].default_value
    }


def convert_rgb_node(ctx, out_socket):
    node = out_socket.node
    return ctx.color(node.color)


def convert_blackbody_node(ctx, out_socket):
    if not ctx.enable_blackbody:
        return dummy_color(ctx)

    node = out_socket.node
    return {
        'type': 'blackbody',
        'temperature': convert_texture_node(ctx, node.inputs['Temperature']),
    }


def convert_wavelength_node(ctx, out_socket):
    if not ctx.enable_wavelength:
        return dummy_color(ctx)

    node = out_socket.node
    return {
        'type': 'wavelength',
        'wavelength': convert_texture_node(ctx, node.inputs['Wavelength']),
    }


def convert_coord_texture_node(ctx, out_socket):
    if not ctx.enable_coord:
        return dummy_color(ctx)

    node = out_socket.node
    params = {
        'type': 'coord',
    }

    add_vector_node_field(ctx, params, out_socket)

    return params


def convert_texture_node(ctx, socket):
    texture_converters = {
        "ShaderNodeTexImage": convert_image_texture_node,
        'ShaderNodeFresnel': convert_fresnel_node,
        'ShaderNodeLayerWeight': convert_layer_weight_node,
        'ShaderNodeMixRGB': convert_mix_rgb_node,
        'ShaderNodeClamp': convert_clamp_node,
        'ShaderNodeRGB': convert_rgb_node,
        'ShaderNodeTexNoise': convert_noise_texture_node,
        'ShaderNodeTexChecker': convert_checker_texture_node,
        'ShaderNodeTexBrick': convert_brick_texture_node,
        'ShaderNodeTexWave': convert_wave_texture_node,
        'ShaderNodeBlackbody': convert_blackbody_node,
        'ShaderNodeWavelength': convert_wavelength_node,
        'ShaderNodeTexCoord': convert_coord_texture_node,
        'ShaderNodeMapping': convert_coord_texture_node,
    }

    params = None
    if socket.is_linked:
        s = ctx.follow_link(socket)
        node = s.links[0].from_node
        from_socket = s.links[0].from_socket

        if node.bl_idname in texture_converters:
            ctx.info(f"Converting a '{node.bl_idname}' Blender shader node.")
            params = texture_converters[node.bl_idname](ctx, from_socket)
            if params and isinstance(params, Iterable) and 'type' in params:
                ctx.info(f"  Created a '{params['type']}' texture.")
        else:
            raise NotImplementedError(
                f"Shader node type {node.bl_idname} is not supported")
    else:
        if socket.name == 'Roughness':  # roughness values in blender are remapped with a square root
            params = pow(socket.default_value, 2)
        else:
            params = ctx.color(socket.default_value)

    return params


def convert_background(ctx, world):
    """
    convert environment lighting. Constant emitter and envmaps are supported

    Params
    ------
    surface_node: the final node of the shader
    ignore_background: whether we want to export blender's default background or not
    """

    try:
        if world is None:
            raise NotImplementedError('No Blender world to export')

        if world.use_nodes and world.node_tree is not None:

            output_node_id = 'World Output'
            if output_node_id not in world.node_tree.nodes:
                raise NotImplementedError(
                    f'Failed to export world: Cannot find world output node. world nodes: {world.node_tree.nodes.keys()}')

            output_node = world.node_tree.nodes[output_node_id]
            if not output_node.inputs['Surface'].is_linked:
                return 0

            surface_node = ctx.follow_link(
                output_node.inputs['Surface']).links[0].from_node
            if 'Strength' not in surface_node.inputs:
                raise NotImplementedError(
                    "Expecting a material with a 'Strength' parameter for a background")

            if surface_node.inputs['Strength'].is_linked:
                raise NotImplementedError(
                    "Only default emitter 'Strength' value is supported")

            strength = surface_node.inputs['Strength'].default_value
            if strength == 0:  # Don't add an emitter if it emits nothing
                ctx.info('Ignoring envmap with zero strength.')
                return 0

            if surface_node.bl_idname in ['ShaderNodeBackground', 'ShaderNodeEmission']:
                socket = surface_node.inputs['Color']
                if socket.is_linked:
                    socket = ctx.follow_link(socket)
                    color_node = socket.links[0].from_node
                    if color_node.bl_idname == 'ShaderNodeTexEnvironment':
                        params = {
                            'type': 'envmap',
                            'filename': export_texture(ctx, color_node.image),
                            'scale': strength
                        }

                        add_vector_node_field(
                            ctx, params, color_node.inputs['Vector'])

                        return params
                    elif color_node.bl_idname == 'ShaderNodeRGB':
                        color = color_node.color
                    else:
                        raise NotImplementedError(
                            f"Node type {color_node.bl_idname} is not supported. Consider using an environment texture or RGB node instead")
                else:
                    color = socket.default_value

                # Not an envmap
                radiance = [x * strength for x in color[:3]]
                if np.sum(radiance) == 0:
                    ctx.info("Ignoring background emitter with zero emission.")
                    return 0
                return ctx.color(radiance)
            else:
                raise NotImplementedError(
                    f"Only Background and Emission nodes are supported as final nodes for background export, got '{surface_node.name}'")
        else:
            # Single color field for emission, no nodes
            return ctx.color(world.color)

    except NotImplementedError as err:
        ctx.report(
            {'WARNING'}, f"Error while converting background: {err.args[0]}. Using default.")
        return 5
