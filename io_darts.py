import math
import os
import shutil
from xml.dom.minidom import Document

import bpy
import bpy_extras
from bpy.props import BoolProperty, IntProperty, StringProperty
from bpy_extras.io_utils import ExportHelper
from mathutils import Matrix
from mathutils import Vector
from math import degrees
import json

bl_info = {
    "name": "Darts",
    "author": "Wojciech Jarosz, Shaojie Jiao, Adrien Gruson, Delio Vicini, Tizian Zeltner",
    "version": (0, 2),
    "blender": (2, 80, 0),
    "location": "File > Export > Darts exporter (.json)",
    "description": "Export Darts scene format (.json)",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export",
    'warning': 'alpha'}


class DartsWriter:
    '''
    Writes a blender scene to a Darts-compatible json scene file.
    '''

    def __init__(self,
                 context,
                 filepath,
                 report,
                 use_selection,
                 use_visibility,
                 use_mesh_modifiers,
                 use_normals,
                 use_uvs,
                 use_materials,
                 use_triangles):
        self.context = context
        self.filepath = filepath
        self.report = report
        self.working_dir = os.path.dirname(self.filepath)

        self.use_selection = use_selection
        self.use_visibility = use_visibility
        self.use_mesh_modifiers = use_mesh_modifiers
        self.use_normals = use_normals
        self.use_uvs = use_uvs
        self.use_materials = use_materials
        self.use_triangles = use_triangles

    def export_camera(self, b_camera, b_scene):
        print('exporting camera')

        # default camera
        cam_json = {
            "transform": {
                "from": [
                    1, 1, 1
                ],
                "at": [
                    0, 0, 0
                ],
                "up": [0, 1, 0]
            },
            "vfov": 30.0,
            "resolution": [1280, 1280]
        }

        # resolution & fov
        res_x = b_scene.render.resolution_x
        res_y = b_scene.render.resolution_y

        # extract fov
        sensor_fit = b_camera.data.sensor_fit
        if (sensor_fit == 'AUTO' and res_x >= res_y) or sensor_fit == 'HORIZONTAL':
            cam_json["vfov"] = degrees(b_camera.data.angle_x) * res_y / res_x
        else:  # sensor_fit == 'VERTICAL':
            cam_json["vfov"] = degrees(b_camera.data.angle_y)

        self.report(
            {'INFO'}, f"fov {degrees(b_camera.data.angle_x)}, {degrees(b_camera.data.angle_y)}, {degrees(b_camera.data.angle)}")

        percent = b_scene.render.resolution_percentage / 100.0
        cam_json["resolution"][0] = int(res_x * percent)
        cam_json["resolution"][1] = int(res_y * percent)

        # up, lookat, and position
        up = b_camera.matrix_world.to_quaternion() @ Vector((0.0, 1.0, 0.0))
        direction = b_camera.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0))
        loc = b_camera.location.copy()

        up[0], up[1], up[2] = up[0], up[2], -up[1]
        direction[0], direction[1], direction[2] = direction[0], direction[2], -direction[1]
        loc[0], loc[1], loc[2] = loc[0], loc[2], -loc[1]

        # set the values and return
        for i in range(0, 3):
            cam_json["transform"]["from"][i] = loc[i]
            cam_json["transform"]["up"][i] = up[i]
            cam_json["transform"]["at"][i] = loc[i] + direction[i]
        return cam_json

    # adds default values to make the scene complete
    def make_misc(self, jso):
        json_sampler = {
            "type": "independent",
            "samples": 100}
        json_background = [5, 5, 5]
        json_accelerator = {"type": "bbh"}
        json_materials = [
            {
                "type": "lambertian",
                "name": "default",
                "albedo": 0.2
            }
        ]
        json_integrator = {
            "type": "ao"}

        jso["sampler"] = json_sampler
        jso["integrator"] = json_integrator
        jso["background"] = json_background
        jso["accelerator"] = json_accelerator
        jso["materials"] = json_materials

    # export meshes to "meshes/" and then point to them in the scene file
    def export_mesh(self, mesh):
        print('exporting mesh')

        # first, save the selection, then select the single mesh we want to export
        viewport_selection = self.context.selected_objects
        bpy.ops.object.select_all(action='DESELECT')

        obj_name = mesh.name + ".obj"
        obj_path = os.path.join(self.working_dir, 'meshes', obj_name)
        mesh.select_set(True)
        bpy.ops.export_scene.obj(filepath=obj_path,
                                 use_selection=True,
                                 use_mesh_modifiers=self.use_mesh_modifiers,
                                 use_normals=self.use_normals,
                                 use_uvs=self.use_uvs,
                                 use_materials=self.use_materials,
                                 use_triangles=self.use_triangles,
                                 use_edges=False,
                                 use_smooth_groups=False,
                                 check_existing=False)

        # using a default lambertian material
        obj_json = {
            "type": "mesh",
            "name": "",
            "filename": "",
            "material": "default"
        }

        obj_json["name"] = mesh.name
        obj_json["filename"] = "meshes/%s.obj" % mesh.name

        # Now restore the previous selection
        for ob in viewport_selection:
            ob.select_set(True)
        mesh.select_set(False)

        return obj_json

    def write(self):
        """Main method to write the blender scene into Darts format"""

        data_all = {}

        # only exporting one camera
        cameras = [cam for cam in self.context.scene.objects
                   if cam.type in {'CAMERA'}]

        if(len(cameras) == 0):
            print("WARN: No camera to export")
        else:
            if(len(cameras) > 1):
                print("WARN: Multiple cameras found, only exporting the active one")

        data_cam = self.export_camera(
            self.context.scene.camera, self.context.scene)
        data_all["camera"] = data_cam

        # adding defaults
        self.make_misc(data_all)

        # export meshes
        if not os.path.exists(self.working_dir + "/meshes"):
            os.makedirs(self.working_dir + "/meshes")

        if self.use_selection and self.use_visibility:
            objects = [
                obj for obj in self.context.selected_objects if obj in self.context.visible_objects]
        elif self.use_selection:
            objects = self.context.selected_objects
        elif self.use_visibility:
            objects = self.context.visible_objects
        else:
            objects = self.context.scene.objects

        meshes = [obj for obj in objects if obj.type in {
            'MESH', 'FONT', 'SURFACE', 'META'}]
        print(meshes)

        # point to the exported meshes
        data_all["surfaces"] = []

        for mesh in meshes:
            cur_obj_json = self.export_mesh(mesh)
            data_all["surfaces"].append(cur_obj_json)

        # write the json file
        with open(self.filepath, "w") as dump_file:
            exported_json_string = json.dumps(data_all, indent=4)
            dump_file.write(exported_json_string)
            dump_file.close


class DartsExporter(bpy.types.Operator, ExportHelper):
    """Export as a Darts scene"""
    bl_idname = "export_scene.darts"
    bl_label = "Darts Export"

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={'HIDDEN'})

    use_selection: BoolProperty(
        name="Selection Only",
        description="Export selected objects only",
        default=False,
    )
    use_visibility: BoolProperty(
        name="Visible Only",
        description="Export visible objects only",
        default=True,
    )
    use_mesh_modifiers: BoolProperty(
        name="Apply Modifiers",
        description="Apply modifiers",
        default=True,
    )
    use_normals: BoolProperty(
        name="Write Normals",
        description="Export one normal per vertex and per face, to represent flat faces and sharp edges",
        default=True,
    )
    use_uvs: BoolProperty(
        name="Include UVs",
        description="Write out the active UV coordinates",
        default=True,
    )
    use_materials: BoolProperty(
        name="Write Materials",
        description="Write out the MTL file",
        default=True,
    )
    use_triangles: BoolProperty(
        name="Triangulate Faces",
        description="Convert all faces to triangles",
        default=True,
    )

    def execute(self, context):
        converter = DartsWriter(context, self.filepath,
                                self.report,
                                self.use_selection,
                                self.use_visibility,
                                self.use_mesh_modifiers,
                                self.use_normals,
                                self.use_uvs,
                                self.use_materials,
                                self.use_triangles)

        converter.write()

        print(converter.filepath)

        self.report({'INFO'}, "Scene exported successfully!")

        return {'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(DartsExporter.bl_idname, text="Darts scene (.json)")


def register():
    bpy.utils.register_class(DartsExporter)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(DartsExporter)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()
