/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/mesh.h>
#include <darts/progress.h>
#include <darts/stats.h>
#include <darts/triangle.h>
#include <filesystem/resolver.h>
#include <fstream>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cpp file
#include <tiny_obj_loader.h>

STAT_RATIO("Geometry/Triangles per mesh", num_triangles, num_tri_meshes);
STAT_MEMORY_COUNTER("Memory/Triangles", triangle_bytes);

Mesh::Mesh(const json &j)
{
    string filename = get_file_resolver().resolve(j.at("filename").get<string>()).str();

    std::ifstream is(filename);
    if (is.fail())
        throw DartsException("Unable to open OBJ file '{}'!", filename);

    Progress progress(fmt ::format("Loading '{}'", filename));

    xform = j.value("transform", xform);
    string warn;
    string err;

    struct UserData
    {
        Mesh            *mesh;
        uint32_t         current_material_idx;
        map<string, int> material_map;
        string           material_prefix;
    } data;

    data.mesh                 = this;
    data.current_material_idx = 0;

    data.material_prefix = j.value("material prefix", "");
    if (data.material_prefix != "")
        spdlog::info("Prepending the string \"{}\" to all mesh material names", data.material_prefix);

    // create a default material used for any faces that don't have a material set
    // this will be the material with index 0
    auto default_material = DartsFactory<Material>::find(j);
    materials.push_back(default_material);

    tinyobj::callback_t cb;

    cb.vertex_cb = [](void *user_data, float x, float y, float z, float w)
    {
        UserData *data = reinterpret_cast<UserData *>(user_data);
        Mesh     *mesh = data->mesh;
        mesh->vs.push_back(mesh->xform.point(Vec3f(x, y, z)));
        mesh->bbox.enclose(mesh->vs.back());
    };

    cb.normal_cb = [](void *user_data, float x, float y, float z)
    {
        UserData *data = reinterpret_cast<UserData *>(user_data);
        Mesh     *mesh = data->mesh;
        // printf("vn[%ld] = %f, %f, %f\n", mesh->ns.size(), x, y, z);

        mesh->ns.push_back(normalize(mesh->xform.normal(Vec3f(x, y, z))));
    };

    cb.texcoord_cb = [](void *user_data, float x, float y, float z)
    {
        UserData *data = reinterpret_cast<UserData *>(user_data);
        Mesh     *mesh = data->mesh;
        // spdlog::info("vt[{}] = {}, {}, {}", mesh->uvs.size(), x, y, z);

        mesh->uvs.push_back(Vec2f(x, y));
    };

    cb.index_cb = [](void *user_data, tinyobj::index_t *indices, int num_indices)
    {
        // NOTE: the value of each index is raw value. For example, the application must manually adjust the index with
        // offset (e.g. v_indices.size()) when the value is negative (which means relative index). Also, the first index
        // starts with 1, not 0. See fixIndex() function in tiny_obj_loader.h for details. Also, 0 is set for the index
        // value which does not exist in .obj
        UserData *data = reinterpret_cast<UserData *>(user_data);
        Mesh     *mesh = data->mesh;

        // just create a naive triangle fan from the first vertex
        tinyobj::index_t idx0 = indices[0], idx1 = indices[1], idx2;

        tinyobj::fixIndex(idx0.vertex_index, int(mesh->vs.size()), &idx0.vertex_index);
        tinyobj::fixIndex(idx0.normal_index, int(mesh->ns.size()), &idx0.normal_index);
        tinyobj::fixIndex(idx0.texcoord_index, int(mesh->uvs.size()), &idx0.texcoord_index);

        tinyobj::fixIndex(idx1.vertex_index, int(mesh->vs.size()), &idx1.vertex_index);
        tinyobj::fixIndex(idx1.normal_index, int(mesh->ns.size()), &idx1.normal_index);
        tinyobj::fixIndex(idx1.texcoord_index, int(mesh->uvs.size()), &idx1.texcoord_index);
        for (int i = 2; i < num_indices; i++)
        {
            idx2 = indices[i];
            tinyobj::fixIndex(idx2.vertex_index, int(mesh->vs.size()), &idx2.vertex_index);
            tinyobj::fixIndex(idx2.normal_index, int(mesh->ns.size()), &idx2.normal_index);
            tinyobj::fixIndex(idx2.texcoord_index, int(mesh->uvs.size()), &idx2.texcoord_index);

            mesh->Fv.push_back({idx0.vertex_index, idx1.vertex_index, idx2.vertex_index});
            mesh->Fn.push_back({idx0.normal_index, idx1.normal_index, idx2.normal_index});
            mesh->Ft.push_back({idx0.texcoord_index, idx1.texcoord_index, idx2.texcoord_index});
            mesh->Fm.push_back(data->current_material_idx);

            idx1 = idx2;
        }
    };

    cb.usemtl_cb = [](void *user_data, const char *name, int material_idx)
    {
        UserData *data = reinterpret_cast<UserData *>(user_data);
        Mesh     *mesh = data->mesh;

        string full_name = data->material_prefix + name;

        // check if we've already added a material with this name to the mesh
        auto it = data->material_map.find(full_name);
        if (it != data->material_map.end())
            data->current_material_idx = it->second;
        else
        {
            // try to find a material with the given name in the scene description and add it to the mesh's materials
            try
            {
                data->mesh->materials.push_back(
                    DartsFactory<Material>::find(json::object({{"material", string(full_name)}})));
                data->material_map[full_name] = data->current_material_idx = data->mesh->materials.size() - 1;
            }
            catch (const std::exception &e)
            {
                spdlog::warn("When parsing OBJ file: {}\n\tUsing default material instead.\n", e.what());
                data->material_map[full_name] = data->current_material_idx = 0;
            }
        }
    };

    cb.mtllib_cb = [](void *user_data, const tinyobj::material_t *materials, int num_materials)
    {
        UserData *data = reinterpret_cast<UserData *>(user_data);
        Mesh     *mesh = data->mesh;
    };

    bool ret = tinyobj::LoadObjWithCallback(is, cb, &data, nullptr, &warn, &err);

    progress.set_done();

    if (!warn.empty())
        spdlog::warn("{}\n", warn);

    if (!err.empty() || !ret)
        throw DartsException("Unable to open OBJ file '{}'!\n\t{}", filename, err);

    spdlog::debug(
        R"(
    # of vertices         = {}
    # of normals          = {}
    # of texcoords        = {}
    # of vertex indices   = {}
    # of normal indices   = {}
    # of texcoord indices = {}
    # of materials        = {}
    xform : {}"
    min: {}
    max: {}
    bottom: {}

)",
        vs.size(), ns.size(), uvs.size(), Fv.size(), Fn.size(), Ft.size(), materials.size(),
        indent(fmt::format("{}", xform.m), string("    xform : ").length()), bbox.min, bbox.max,
        (bbox.min + bbox.max) / 2.f - Vec3f(0, bbox.diagonal()[1] / 2.f, 0));

    ++num_tri_meshes;
    num_triangles += Fv.size();
    triangle_bytes += size();
}

size_t Mesh::size() const
{
    return vs.capacity() * sizeof(Vec3f) + ns.capacity() * sizeof(Vec3f) + uvs.capacity() * sizeof(Vec2f) +
           Fv.capacity() * sizeof(Vec3i) + Fn.capacity() * sizeof(Vec3i) + Ft.capacity() * sizeof(Vec3i) +
           Fm.capacity() * sizeof(uint32_t);
}

void Mesh::add_to_parent(Surface *parent, shared_ptr<Surface> self, const json &j)
{
    auto mesh = std::dynamic_pointer_cast<Mesh>(self);

    if (!mesh && mesh->empty())
        return;

    for (auto index : range(mesh->Fv.size()))
        parent->add_child(make_shared<Triangle>(j, mesh, int(index)));
}

DARTS_REGISTER_CLASS_IN_FACTORY(Surface, Mesh, "mesh")

/**
    \file
    \brief Mesh Surface
*/