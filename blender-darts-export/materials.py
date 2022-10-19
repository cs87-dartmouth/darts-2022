import numpy as np
import bpy

from . import textures

RoughnessMode = {'GGX': 'ggx', 'BECKMANN': 'beckmann',
                 'ASHIKHMIN_SHIRLEY': 'blinn', 'MULTI_GGX': 'ggx'}


def make_two_sided(ctx, bsdf):
    if ctx.force_two_sided:
        ctx.info(
            f"  Wrapping '{bsdf['type']}' material in a 'two sided' adapter.")
        return {
            'type': 'two sided',
            'both sides': bsdf
        }
    else:
        return bsdf


def convert_diffuse_material(ctx, current_node):
    params = {
        'type': 'lambertian',
        'albedo': textures.convert_texture_node(ctx, current_node.inputs['Color']),
        'roughness': textures.convert_texture_node(ctx, current_node.inputs['Roughness'])
    }
    return make_two_sided(ctx, params)


def convert_glossy_material(ctx, current_node):
    params = {}

    if ctx.glossy_mode == 'rough conductor':
        if current_node.distribution != 'SHARP':
            params.update({
                'type': ctx.glossy_mode,
                'distribution': RoughnessMode[current_node.distribution],
            })

            params.update({
                'roughness': textures.convert_texture_node(ctx,
                                                           current_node.inputs['Roughness']),
            })
            if 'Anisotropy' in current_node.inputs:
                params.update({
                    'anisotropy': textures.convert_texture_node(ctx,
                                                                current_node.inputs['Anisotropy']),
                })
            if 'Rotation' in current_node.inputs:
                params.update({
                    'rotation': textures.convert_texture_node(ctx, current_node.inputs['Rotation']),
                })

        else:
            params.update({
                'type': 'conductor'
            })

        params.update({
            'color': textures.convert_texture_node(ctx, current_node.inputs['Color']),
        })

    elif ctx.glossy_mode == 'blinn-phong' or ctx.glossy_mode == 'phong':
        if current_node.inputs['Roughness'].is_linked:
            raise NotImplementedError(
                "Phong and Blinn-Phong roughness parameter doesn't support textures in Darts")
        else:
            def roughness_to_blinn_exponent(alpha):
                return max(2.0 / (alpha * alpha) - 1.0, 0.0)

            roughness = roughness_to_blinn_exponent(
                pow(current_node.inputs['Roughness'].default_value, 2))
            if ctx.glossy_mode == 'phong':
                roughness = roughness / 4

        if current_node.distribution != 'SHARP':
            params.update({
                'type': ctx.glossy_mode,
                'exponent': roughness,
                'distribution': RoughnessMode[current_node.distribution],
            })
        else:
            params.update({
                'type': 'metal',
                'roughness': 0
            })

        params.update({
            'albedo': textures.convert_texture_node(ctx, current_node.inputs['Color']),
        })
    elif ctx.glossy_mode == 'metal':
        params.update({
            'type': ctx.glossy_mode,
            'roughness': textures.convert_texture_node(ctx,
                                                       current_node.inputs['Roughness']) if current_node.distribution != 'SHARP' else 0,
            'albedo': textures.convert_texture_node(ctx, current_node.inputs['Color']),
        })

    return make_two_sided(ctx, params)


def convert_glass_material(ctx, current_node):
    params = {}

    if current_node.inputs['IOR'].is_linked:
        ctx.report(
            {'WARNING'}, f"{current_node.name}: Textured IOR values are not supported in Darts. Using the default value instead.")

    ior = current_node.inputs['IOR'].default_value

    roughness = textures.convert_texture_node(ctx,
                                              current_node.inputs['Roughness'])

    if roughness and current_node.distribution != 'SHARP':
        params.update({
            'type': 'rough dielectric',
            'roughness': roughness,
            'distribution': RoughnessMode[current_node.distribution],
        })
    else:
        params['type'] = 'thin dielectric' if ior == 1.0 else 'dielectric'

    params['ior'] = ior
    params['reflectance'] = params['transmittance'] = textures.convert_texture_node(
        ctx, current_node.inputs['Color'])

    return params


def convert_emitter_material(ctx, current_node):
    if current_node.inputs['Strength'].is_linked:
        raise NotImplementedError(
            "Only default emitter strength value is supported")
    else:
        radiance = current_node.inputs['Strength'].default_value

    if current_node.inputs['Color'].is_linked:
        raise NotImplementedError(
            "Only default emitter color is supported")  # TODO: rgb input
    else:
        radiance = [
            x * radiance for x in current_node.inputs['Color'].default_value[:]]

    if np.sum(radiance) == 0:
        ctx.report(
            {'WARN'}, "  Emitter has zero emission, this may cause Darts to fail! Creating a 'diffuse' material instead.")
        return {'type': 'diffuse', 'albedo': ctx.color(0)}

    return {
        'type': 'diffuse_light',
        'emit': ctx.color(radiance),
    }


def convert_transparent_material(ctx, current_node):

    return {
        'type': 'transparent',
        'color': textures.convert_texture_node(ctx, current_node.inputs['Color'])
    }


def convert_mix_material(ctx, current_node):
    if not current_node.inputs[1].is_linked or not current_node.inputs[2].is_linked:
        raise NotImplementedError("Mix shader is not linked to two materials")

    mat_I = current_node.inputs[1].links[0].from_node
    mat_II = current_node.inputs[2].links[0].from_node

    return {
        'type': 'blend',
        'amount': textures.convert_texture_node(ctx, current_node.inputs['Fac']),
        'a': cycles_material_to_dict(ctx, mat_I),
        'b': cycles_material_to_dict(ctx, mat_II)
    }


def convert_add_material(ctx, current_node):
    if not current_node.inputs[1].is_linked or not current_node.inputs[2].is_linked:
        raise NotImplementedError("Add shader is not linked to two materials")

    mat_I = current_node.inputs[1].links[0].from_node
    mat_II = current_node.inputs[2].links[0].from_node

    return {
        'type': 'add',
        'a': cycles_material_to_dict(ctx, mat_I),
        'b': cycles_material_to_dict(ctx, mat_II)
    }


def wrap_with_bump_or_normal_map(ctx, node, nested):
    if (ctx.use_normal_maps or ctx.use_bump_maps) and 'Normal' in node.inputs and node.inputs['Normal'].is_linked:
        normal_socket = node.inputs['Normal']
        normal_node = normal_socket.links[0].from_node

        if normal_node.inputs['Strength'].is_linked:
            raise NotImplementedError(
                "Only default normal/bump map strength value is supported")

        if normal_node.bl_idname == "ShaderNodeNormalMap":
            if not ctx.use_normal_maps:
                return nested

            if normal_node.space != 'TANGENT':
                raise NotImplementedError(
                    f"Darts only supports tangent-space normal maps")
            wrapper = {
                'type': 'normal map',
                'normals': textures.convert_texture_node(ctx, normal_node.inputs['Color'].links[0].from_socket),
                'strength': normal_node.inputs['Strength'].default_value,
            }
        elif normal_node.bl_idname == "ShaderNodeBump":
            if not ctx.use_bump_maps:
                return nested
            wrapper = {
                'type': 'bump map',
                'height': textures.convert_texture_node(ctx, normal_node.inputs['Height'].links[0].from_socket),
                'strength': normal_node.inputs['Strength'].default_value,
                'distance': normal_node.inputs['Distance'].default_value,
                'invert': normal_node.invert,
            }
        else:
            raise NotImplementedError(
                "Only normal map and bump nodes supported for 'Normal' input")

        if 'name' in nested:
            wrapper['name'] = nested.pop('name')

        wrapper['nested'] = nested

        ctx.info(f"  Wrapping material with a '{wrapper['type']}'.")
        return wrapper
    else:
        return nested


def cycles_material_to_dict(ctx, node, name=None):
    ''' Converting one material from Blender to Darts format'''

    cycles_converters = {
        "ShaderNodeBsdfDiffuse": convert_diffuse_material,
        'ShaderNodeBsdfGlossy': convert_glossy_material,
        'ShaderNodeBsdfAnisotropic': convert_glossy_material,
        'ShaderNodeBsdfGlass': convert_glass_material,
        'ShaderNodeMixShader': convert_mix_material,
        'ShaderNodeAddShader': convert_add_material,
        'ShaderNodeEmission': convert_emitter_material,
        'ShaderNodeBsdfTransparent': convert_transparent_material,
    }

    params = {}
    if name is not None:
        params['name'] = material_name(name)

    if node.bl_idname in cycles_converters:
        ctx.info(f"Converting a '{node.bl_idname}' Blender material.")
        params.update(cycles_converters[node.bl_idname](ctx, node))
        ctx.info(f"  Created a '{params['type']}' material.")
    else:
        raise NotImplementedError(
            f"Node type: {node.bl_idname} is not supported in Darts")

    return wrap_with_bump_or_normal_map(ctx, node, params)


def material_name(b_name):
    return f"{b_name.replace(' ', '_')}"


def get_dummy_material(ctx, name):
    params = {
        'type': 'lambertian',
        'albedo': ctx.color([1.0, 0.0, 0.3]),
    }
    if name is not None:
        params['name'] = material_name(name)
    return params


def convert_material(ctx, b_mat):
    ''' Converting one material from Blender / Cycles to Darts'''

    if b_mat.use_nodes:
        try:
            output_node_id = 'Material Output'
            if output_node_id in b_mat.node_tree.nodes:
                output_node = b_mat.node_tree.nodes[output_node_id]
                surface_node = output_node.inputs['Surface'].links[0].from_node
                mat_params = cycles_material_to_dict(ctx,
                                                     surface_node, b_mat.name_full)
                if 'Displacement' in output_node.inputs and output_node.inputs['Displacement'].is_linked:
                    ctx.report(
                        {'WARNING'}, f"Material '{b_mat.name_full}': Displacement maps are not supported. Consider converting them to bump maps first")
            else:
                raise NotImplementedError("Cannot find material output node")
        except NotImplementedError as e:
            ctx.report(
                {'WARNING'}, f"Export of material '{b_mat.name_full}' failed: {e.args[0]}. Exporting a dummy material instead.")
            mat_params = get_dummy_material(ctx, b_mat.name_full)
    else:
        mat_params = get_dummy_material(ctx, b_mat.name_full)
        mat_params.update({'albedo': ctx.color(b_mat.diffuse_color)})

    return mat_params


def export(ctx, meshes):
    """Write out the materials to Darts format"""

    ctx.info(f"Writing default lambertian material")
    mat_json = [
        {
            "type": "lambertian",
            "name": "default",
            "albedo": 0.2
        }
    ]

    if ctx.material_mode == "LAMBERTIAN":
        for mat in bpy.data.materials:
            # skip any materials that aren't being used
            if mat.name_full == "Dots Stroke" or mat.users == 0:
                continue

            ctx.info(f"Writing placeholder for material '{mat.name_full}'")
            name = mat.name_full
            params = get_dummy_material(
                ctx, mat.name_full)
            mat_json.append(params)

    elif ctx.material_mode == "CONVERT":
        for mesh in meshes:
            if not mesh.data or not mesh.data.materials:
                continue
            ctx.info(
                f"Object '{mesh.name_full}' of type '{mesh.type}' has {len(mesh.data.materials)} materials.")
            for mat in mesh.data.materials:

                # skip any materials that aren't being used
                if not mat:
                    continue
                if mat.name_full == "Dots Stroke" or mat.users == 0:
                    ctx.info(f"Skipping unused material '{mat.name_full}'")
                    continue

                # skip if we've already exported this material
                mat_id = f"mat-{mat.name_full}"
                if mat_id in ctx.already_exported.keys():
                    ctx.info(
                        f"Skipping previously exported material '{mat.name_full}'.")
                    continue

                ctx.info(
                    f"Exporting material '{mat.name_full}', with {mat.users} users.")

                mat_params = convert_material(ctx, mat)
                ctx.already_exported[mat_id] = mat_params

                mat_json.append(mat_params)

    return mat_json
