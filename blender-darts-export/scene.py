import os
import time
import bpy
from mathutils import Matrix
from mathutils import Vector
from mathutils import Euler
from mathutils import Quaternion
from math import degrees
import json

from . import materials
from . import textures
from . import lights
from . import geometry
from . import camera

SUPPORTED_TYPES = {"MESH", "CURVE", "FONT", "META",
                   "EMPTY", "SURFACE"}  # Formats we can save as .obj


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
                 verbose,
                 use_selection,
                 use_visibility,
                 integrator,
                 sampler,
                 use_lights,
                 mesh_mode,
                 material_mode,
                 glossy_mode,
                 use_normal_maps,
                 use_bump_maps,
                 force_two_sided,
                 enable_background,
                 enable_blackbody,
                 enable_brick,
                 enable_checker,
                 enable_coord,
                 enable_fresnel,
                 enable_layer_weight,
                 enable_mapping,
                 enable_musgrave,
                 enable_mix_rgb,
                 enable_noise,
                 enable_voronoi,
                 enable_wave,
                 enable_wavelength):
        self.context = context
        self.report = report

        self.write_obj_files = write_obj_files

        self.verbose = verbose
        self.use_selection = use_selection
        self.use_visibility = use_visibility

        self.integrator = integrator
        self.sampler = sampler
        self.use_lights = use_lights

        self.mesh_mode = mesh_mode

        self.material_mode = material_mode
        self.write_texture_files = write_texture_files
        self.use_normal_maps = use_normal_maps
        self.use_bump_maps = use_bump_maps
        self.force_two_sided = force_two_sided
        self.glossy_mode = glossy_mode

        self.enable_background = enable_background
        self.enable_mapping = enable_mapping
        self.enable_fresnel = enable_fresnel
        self.enable_layer_weight = enable_layer_weight
        self.enable_mix_rgb = enable_mix_rgb
        self.enable_noise = enable_noise
        self.enable_musgrave = enable_musgrave
        self.enable_voronoi = enable_voronoi
        self.enable_checker = enable_checker
        self.enable_brick = enable_brick
        self.enable_blackbody = enable_blackbody
        self.enable_wavelength = enable_wavelength
        self.enable_wave = enable_wave
        self.enable_coord = enable_coord

        self.filepath = filepath
        self.directory = os.path.dirname(filepath)
        self.already_exported = {}

    def info(self, message):
        if self.verbose:
            self.report({'INFO'}, message)

    def follow_link(self, socket):
        '''
        Recursively follow a link potentially via reroute nodes
        '''
        if socket.is_linked and socket.links[0].from_node.bl_idname == 'NodeReroute':
            return self.follow_link(socket.links[0].from_node.inputs[0])
        else:
            return socket

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

    def transform_matrix(self, matrix):
        if len(matrix) == 4:
            mat = matrix
        else:  # 3x3
            mat = matrix.to_4x4()

        loc, rot, sca = mat.decompose()
        eul = rot.to_euler()
        self.info(
            f"Writing matrix: {mat}, as\n\t{loc}\n\t{tuple(degrees(a) for a in eul)}\n\t{sca}")

        params = []
        if eul[:] != (0, 0, 0):
            params.extend([{'rotate': (degrees(eul.x), 1, 0, 0)},
                           {'rotate': (degrees(eul.y), 0, 1, 0)},
                           {'rotate': (degrees(eul.z), 0, 0, 1)}])
        if sca[:] != (1, 1, 1):
            params.append({'scale': sca[:]})
        if loc[:] != (0, 0, 0):
            params.append({'translate': loc[:]})
        return params

        # return {'matrix': list(i for j in mat for i in j)}

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

        if self.enable_background:
            params["background"] = textures.convert_background(self,
                                                               self.context.scene.world)
        else:
            params["background"] = 5

        params["accelerator"] = {"type": "bbh"}

        return params

    def write(self):
        """Main method to write the blender scene into Darts format"""

        start = time.perf_counter()

        # Switch to object mode before exporting stuff, so everything is defined properly
        if bpy.ops.object.mode_set.poll():
            bpy.ops.object.mode_set(mode='OBJECT')

        data_all = {}

        data_all["camera"] = camera.export(self, self.context.scene)

        # adding defaults
        data_all.update(self.make_misc())

        # start with just the objects visible to the renderer
        objects = [o for o in self.context.scene.objects if not o.hide_render]

        # figure out the list based on the export settings
        if self.use_selection:
            objects = [o for o in objects if o.select_get()]
        if self.use_visibility:
            objects = [o for o in objects if o.visible_get()]

        b_meshes = [o for o in objects if o.type in SUPPORTED_TYPES]

        # export the materials
        data_all["materials"] = materials.export(self, b_meshes)

        # export meshes
        data_all["surfaces"] = geometry.export(self, b_meshes)

        # export lights
        if self.use_lights:
            b_lights = [o for o in objects if o.type in {'LIGHT'}]
            data_all["surfaces"].extend(lights.export(self, b_lights))

        # write the json file
        with open(self.filepath, "w") as dump_file:
            exported_json_string = json.dumps(data_all, indent=4)
            dump_file.write(exported_json_string)
            dump_file.close

        end = time.perf_counter()
        self.report(
            {'INFO'}, f"Scene exported successfully to '{self.filepath}' in {end-start} s!")
