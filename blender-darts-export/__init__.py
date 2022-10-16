if "bpy" in locals():
    import importlib
    if "scene" in locals():
        importlib.reload(scene)
    if "materials" in locals():
        importlib.reload(materials)
    if "textures" in locals():
        importlib.reload(textures)
    if "lights" in locals():
        importlib.reload(lights)
    if "geometry" in locals():
        importlib.reload(geometry)
    if "camera" in locals():
        importlib.reload(camera)

import bpy
import bpy_extras
from bpy.props import BoolProperty, IntProperty, StringProperty, EnumProperty
from bpy_extras.io_utils import ExportHelper

bl_info = {
    "name": "Darts",
    "author": "Wojciech Jarosz, Baptiste Nicolet, Shaojie Jiao, Adrien Gruson, Delio Vicini, Tizian Zeltner",
    "version": (0, 2, 5),
    "blender": (2, 80, 0),
    "location": "File > Export > Darts exporter (.json)",
    "description": "Export Darts scene format (.json)",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export",
    'warning': 'alpha'}


class DartsExporter(bpy.types.Operator, ExportHelper):
    """Export as a Darts scene"""
    bl_idname = "export_scene.darts"
    bl_label = "Darts Export"

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={'HIDDEN'})

    verbose: BoolProperty(
        name="Verbose output",
        description="Print out extra info in Blender's Info Log",
        default=False,
    )
    use_selection: BoolProperty(
        name="Selection Only",
        description="Export selected objects only",
        default=False,
    )
    use_visibility: BoolProperty(
        name="Visible Only",
        description="We export only objects that are visible for render. If checked, also exclude any objects hidden in the viewport",
        default=True,
    )
    write_obj_files: BoolProperty(
        name="Write OBJ files",
        description="Uncheck this to write out the Darts scene file, but not write out any OBJs to disk",
        default=True,
    )

    # Scene-wide settings
    integrator: EnumProperty(
        name="Integrator",
        items=(
            ('none', "None", "Do not include an integrator at all"),
            ('ao', "ao", "Use the ambient occlusion integrator"),
            ('normals', "normals", "Use the normals integrator"),
            ('path tracer mis', "path tracer mis",
             "Use the MIS-based path tracing integrator"),
        ),
        default="path tracer mis",
    )
    sampler: EnumProperty(
        name="Sampler",
        items=(
            ('none', "None", "Do not include a sampler at all"),
            ('independent', "independent", "Independent sampler"),
            ('stratified', "stratified", "Stratified sampler"),
            ('cmj', "cmj", "Correlated multi-jittered sampler"),
            ('oa', "oa", "Orthogonal array sampler"),
            ('psobol2d', "psobol2d", "Padded 2D Sobol sampler"),
            ('ssobol', "ssobol", "Stochastic Sobol sampler"),
        ),
        default="independent",
    )
    use_lights: BoolProperty(
        name="Export lights",
        description="Export Blender lights",
        default=True,
    )

    # Geometry/OBJ export-related settings
    mesh_mode: EnumProperty(
        name="Convert to",
        items=(
            ('SINGLE', "a single mesh",
             "Write all scene geometry out as a single mesh (OBJ file)"),
            ('SPLIT', "one mesh per Blender object",
             "Write each Blender object out as a separate mesh (OBJ file)")
        ),
        default="SINGLE",
    )

    # Material-related settings
    material_mode: EnumProperty(
        name="Materials",
        items=(
            ('OFF', "None",
             "Do not write materials to OBJ. Include a single default material in the Darts scene to use for all surfaces"),
            ('ONE', "One default material",
             "Write materials to OBJ and a single default material in the Darts scene to use for all surfaces"),
            ('LAMBERTIAN', "Lambertian placeholder materials",
             "Write OBJ materials and create a placeholder lambertian material in Darts for each Blender material"),
            ('CONVERT', "Convert Blender materials",
             "Write OBJ materials and create an approximately equivalent Darts material for each Blender material"),
        ),
        default="CONVERT",
    )
    glossy_mode: EnumProperty(
        name="Glossy as",
        items=(
            ('metal', "metal",
             "Convert Blender's `Glossy` material to a `metal` Darts material"),
            ('phong', "phong",
             "Convert Blender's `Glossy` material to a `phong` Darts material"),
            ('blinn-phong', "blinn-phong",
             "Convert Blender's `Glossy` material to a `blinn-phong` Darts material"),
            ('rough conductor', "(rough) conductor",
             "Convert Blender's `Glossy` material to a `conductor` or `rough conductor` Darts material"),
        ),
        default="rough conductor",
    )
    force_two_sided: BoolProperty(
        name="Force two-sided materials",
        description="Wraps all opaque materials in a 'two sided' adapter. Otherwise the back side of opaque materials will be black in Darts by default",
        default=True,
    )
    use_normal_maps: BoolProperty(
        name="Use normal maps",
        description="Wraps the Darts material in a 'normal map' adapter if the Blender material has a normal map",
        default=True,
    )
    use_bump_maps: BoolProperty(
        name="Use bump maps",
        description="Wraps the Darts material in a 'bump map' adapter if the Blender material has a bump map",
        default=False,
    )

    # Texture-related settings
    enable_background: BoolProperty(
        name="Background",
        description="Export background color (constant or envmap). When disabled, Darts' background is set to a fixed 5",
        default=True,
    )
    write_texture_files: BoolProperty(
        name="Write textures",
        description="Uncheck this to write out the Darts scene file, but not write out any textures to disk",
        default=True,
    )
    enable_mapping: BoolProperty(
        name="Texture coords",
        description="Convert Blender Texture Coordinate and Mapping nodes to a Darts 'mapping' field",
        default=True,
    )
    enable_fresnel: BoolProperty(
        name="Fresnel",
        description="Convert Blender Fresnel shader to a Darts 'fresnel' texture. When disabled, Fresnel nodes are converted to a fixed 0.5",
        default=True,
    )
    enable_layer_weight: BoolProperty(
        name="Layer Weight",
        description="Convert Blender Layer Weight shader to a Darts 'layer weight' texture. When disabled, Layer Weight nodes are converted to a fixed 0.5",
        default=True,
    )
    enable_mix_rgb: BoolProperty(
        name="Mix",
        description="Convert Blender Mix RGB shader to a Darts 'blend' texture",
        default=True,
    )
    enable_noise: BoolProperty(
        name="Noise",
        description="Convert Blender Noise Texture shader to a Darts 'noise' texture. When disabled, Noise textures are converted to a fixed 0.5",
        default=True,
    )
    enable_checker: BoolProperty(
        name="Checker",
        description="Convert Blender Checker Texture shader to a Darts 'checker' texture",
        default=True,
    )
    enable_brick: BoolProperty(
        name="Brick",
        description="Convert Blender Brick Texture shader to a Darts 'brick' texture",
        default=True,
    )
    enable_blackbody: BoolProperty(
        name="Blackbody",
        description="Convert Blender Blackbody shader to a Darts 'blackbody' texture",
        default=True,
    )

    def execute(self, context):
        from . import scene

        keywords = self.as_keywords(ignore=("check_existing",
                                            "filter_glob"
                                            ))
        converter = scene.SceneWriter(context,
                                      self.report,
                                      **keywords)
        converter.write()

        return {'FINISHED'}

    def draw(self, context):
        pass


class DARTS_PT_export_include(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Include"
    bl_parent_id = "FILE_PT_operator"

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_darts"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator

        layout.prop(operator, 'verbose')

        sublayout = layout.column(heading="Limit to")
        sublayout.prop(operator, "use_selection")
        sublayout.prop(operator, "use_visibility")


class DARTS_PT_export_scene(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Scene"
    bl_parent_id = "FILE_PT_operator"

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_darts"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator

        layout.prop(operator, 'integrator')
        layout.prop(operator, 'sampler')
        layout.prop(operator, 'use_lights')


class DARTS_PT_export_geometry(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Geometry"
    bl_parent_id = "FILE_PT_operator"

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_darts"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator

        layout.prop(operator, 'mesh_mode')
        layout.prop(operator, "write_obj_files")


class DARTS_PT_export_materials(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Material conversion"
    bl_parent_id = "FILE_PT_operator"

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_darts"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator

        layout.prop(operator, 'material_mode')

        sublayout = layout.column()
        sublayout.enabled = (operator.material_mode == 'CONVERT')
        sublayout.prop(operator, "use_normal_maps")
        sublayout.prop(operator, "use_bump_maps")
        sublayout.prop(operator, "force_two_sided")
        sublayout.prop(operator, 'glossy_mode')


class DARTS_PT_export_textures(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Texture conversion"
    bl_parent_id = "FILE_PT_operator"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_darts"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator

        sublayout = layout.column(heading="Enable")

        sublayout.prop(operator, "write_texture_files")
        sublayout.prop(operator, 'enable_mapping')
        sublayout.prop(operator, 'enable_background')
        sublayout.prop(operator, 'enable_fresnel')
        sublayout.prop(operator, "enable_layer_weight")
        sublayout.prop(operator, "enable_mix_rgb")
        sublayout.prop(operator, "enable_noise")
        sublayout.prop(operator, "enable_checker")
        sublayout.prop(operator, 'enable_brick')
        sublayout.prop(operator, 'enable_blackbody')


def menu_func_export(self, context):
    self.layout.operator(DartsExporter.bl_idname, text="Darts scene (.json)")


classes = (
    DartsExporter,
    DARTS_PT_export_include,
    DARTS_PT_export_scene,
    DARTS_PT_export_geometry,
    DARTS_PT_export_materials,
    DARTS_PT_export_textures
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)

    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

    for cls in classes:
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
