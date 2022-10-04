import os
import bpy


def write_obj(ctx, obj_name):
    if ctx.write_obj_files:
        # new, faster C++ OBJ exporter
        bpy.ops.wm.obj_export(filepath=os.path.join(ctx.directory, 'meshes', obj_name + ".obj"),
                              export_selected_objects=True,
                              apply_modifiers=ctx.use_mesh_modifiers,
                              export_normals=ctx.use_normals,
                              export_uv=ctx.use_uvs,
                              export_materials=ctx.material_mode != "OFF",
                              export_triangulated_mesh=ctx.use_triangles,
                              check_existing=False,
                              forward_axis='Y', up_axis='Z')
        # legacy python exporter
        # bpy.ops.export_scene.obj(filepath=os.path.join(ctx.directory, 'meshes', obj_name + ".obj"),
        #                          use_selection=True,
        #                          use_mesh_modifiers=ctx.use_mesh_modifiers,
        #                          use_normals=ctx.use_normals,
        #                          use_uvs=ctx.use_uvs,
        #                          use_materials=ctx.material_mode != "OFF",
        #                          use_triangles=ctx.use_triangles,
        #                          use_edges=False,
        #                          use_smooth_groups=False,
        #                          check_existing=False,
        #                          axis_forward='Y', axis_up='Z')
    return {
        "type": "mesh",
        "name": obj_name,
        "filename": f"meshes/{obj_name}.obj",
        "material": "default"
    }

# export meshes to "meshes/" and then point to them in the scene file


def write_mesh(ctx, mesh):

    # first, save the selection, then select the single mesh we want to export
    viewport_selection = ctx.context.selected_objects
    bpy.ops.object.select_all(action='DESELECT')

    mesh.select_set(True)

    obj_json = write_obj(ctx, mesh.name)

    # Now restore the previous selection
    bpy.ops.object.select_all(action='DESELECT')
    for ob in viewport_selection:
        ob.select_set(True)

    return obj_json

# export meshes to "meshes/" and then point to them in the scene file


def write_meshes(ctx, meshes):

    # first, save the selection, then select the single mesh we want to export
    viewport_selection = ctx.context.selected_objects
    bpy.ops.object.select_all(action='DESELECT')

    for mesh in meshes:
        mesh.select_set(True)

    obj_name, _ = os.path.splitext(ctx.filepath)
    obj_name = os.path.basename(obj_name)

    obj_json = write_obj(ctx, obj_name)

    # Now restore the previous selection
    bpy.ops.object.select_all(action='DESELECT')
    for ob in viewport_selection:
        ob.select_set(True)

    return obj_json


def export_meshes(ctx, meshes):
    if not os.path.exists(ctx.directory + "/meshes"):
        os.makedirs(ctx.directory + "/meshes")

    surfaces_json = []
    if ctx.mesh_mode == "SINGLE":
        surfaces_json.append(write_meshes(ctx, meshes))
    else:
        for mesh in meshes:
            surfaces_json.append(write_mesh(ctx, mesh))

    return surfaces_json
