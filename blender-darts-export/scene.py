import os
import bpy
from mathutils import Matrix
from mathutils import Vector
import json

from . import materials
from . import lights
from . import geometry
from . import camera


class SceneWriter:
    '''
    Writes a Blender scene to a Darts-compatible json scene file.
    '''

    def __init__(self,
                 context,
                 report,
                 filepath,
                 write_obj_files,
                 write_texture_files,
                 use_selection,
                 use_visibility,
                 integrator,
                 sampler,
                 use_background,
                 use_lights,
                 use_mesh_modifiers,
                 use_normals,
                 use_uvs,
                 mesh_mode,
                 use_triangles,
                 material_mode,
                 glossy_mode,
                 use_normal_maps,
                 force_two_sided):
        self.context = context
        self.report = report

        self.write_obj_files = write_obj_files

        self.use_selection = use_selection
        self.use_visibility = use_visibility

        self.integrator = integrator
        self.sampler = sampler
        self.use_background = use_background
        self.use_lights = use_lights

        self.use_mesh_modifiers = use_mesh_modifiers
        self.use_normals = use_normals
        self.use_uvs = use_uvs
        self.mesh_mode = mesh_mode
        self.use_triangles = use_triangles

        self.material_mode = material_mode
        self.write_texture_files = write_texture_files
        self.use_normal_maps = use_normal_maps
        self.force_two_sided = force_two_sided
        self.glossy_mode = glossy_mode

        self.filepath = filepath
        self.directory = os.path.dirname(filepath)
        self.exported_mats = {}

    def color(self, value):
        '''
        Given a color value, format it for the scene dict.

        Params
        ------

        value: value of the color: can be a an rgb triplet, a single number
        '''
        col = {}

        if isinstance(value, (float, int)):
            col = value
        else:
            value = list(value)
            if any(not isinstance(x, (float, int, tuple)) for x in value):
                raise ValueError(f"Unknown color entry: {value}")
            if any(type(value[i]) != type(value[i+1]) for i in range(len(value)-1)):
                raise ValueError(f"Mixed types in color entry {value}")
            totitems = len(value)
            if isinstance(value[0], (float, int)):
                if totitems == 3 or totitems == 4:
                    col = value[:3]
                elif totitems == 1:
                    col = value[0]
                else:
                    raise ValueError(
                        f"Expected color items to be 1,3 or 4 got {len(value)}: {value}")

        if not col:
            col = 0

        return col

    def export_texture(self, image):
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

        textures_folder = os.path.join(self.directory, "textures")
        if image.file_format in convert_format:
            self.report(
                {'WARNING'}, f"Image format of '{image.name}' is not supported. Converting it to {convert_format[image.file_format]}.")
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
        if self.write_texture_files:
            target_path = os.path.join(textures_folder, name)
            if not os.path.isdir(textures_folder):
                os.makedirs(textures_folder)
            old_filepath = image.filepath
            image.filepath_raw = target_path
            image.save()
            image.filepath_raw = old_filepath
        return f"textures/{name}"

    def transform_matrix(self, matrix):
        if len(matrix) == 4:
            mat = matrix
        else:  # 3x3
            mat = matrix.to_4x4()
        self.report({'INFO'}, "Writing matrix")
        return {'matrix': list(i for j in mat for i in j)}

    def make_misc(self):
        """Adds default values to make the scene complete"""

        params = {}

        params["sampler"] = {
            "samples": self.context.scene.cycles.samples
        }

        if (self.sampler != 'none'):
            params["sampler"].update({
                "type": self.sampler
            })

        if (self.integrator != 'none'):
            params["integrator"] = {
                "type": self.integrator}

        if self.use_background:
            params["background"] = materials.convert_background(self,
                                                                self.context.scene.world)
        else:
            params["background"] = 5

        params["accelerator"] = {"type": "bbh"}

        return params

    def write(self):
        """Main method to write the blender scene into Darts format"""

        # Switch to object mode before exporting stuff, so everything is defined properly
        if bpy.ops.object.mode_set.poll():
            bpy.ops.object.mode_set(mode='OBJECT')

        data_all = {}

        data_all["camera"] = camera.export_camera(self, self.context.scene)

        # adding defaults
        data_all.update(self.make_misc())

        # start with just the objects visible to the renderer
        objects = [
            obj for obj in self.context.scene.objects if not obj.hide_render]

        # figure out the list based on the export settings
        if self.use_selection:
            objects = [obj for obj in objects if obj.select_get()]
        if self.use_visibility:
            objects = [obj for obj in objects if obj.visible_get()]

        meshes = [obj for obj in objects
                  if obj.type in {'MESH', 'FONT', 'SURFACE', 'META'}]

        # export the materials
        data_all["materials"] = materials.export_materials(self, meshes)

        # export meshes
        data_all["surfaces"] = geometry.export_meshes(self, meshes)

        # export lights
        if self.use_lights:
            b_lights = [obj for obj in objects if obj.type in {'LIGHT'}]
            data_all["surfaces"].extend(
                lights.export_light(self, b_lights))

        # write the json file
        with open(self.filepath, "w") as dump_file:
            exported_json_string = json.dumps(data_all, indent=4)
            dump_file.write(exported_json_string)
            dump_file.close

        self.report(
            {'INFO'}, f"Scene exported successfully to '{self.filepath}'!")
