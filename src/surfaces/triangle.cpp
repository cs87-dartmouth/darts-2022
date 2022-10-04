/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/mesh.h>
#include <darts/stats.h>
#include <darts/triangle.h>


Triangle::Triangle(const json &j) : m_face_idx(0)
{
    // "positions" field is required
    if (!j.contains("positions") || !j.at("positions").is_array() || j.at("positions").size() != 3)
        throw DartsException("required \"positions\" field should be an array of three Vec3s");

    auto mesh       = make_shared<Mesh>();
    mesh->Fv        = {{0, 1, 2}};
    mesh->Fm        = {0};
    auto m          = DartsFactory<Material>::find(j);
    mesh->materials = {m};
    mesh->xform     = j.value("transform", mesh->xform);
    mesh->vs        = {mesh->xform.point(j["positions"][0]), mesh->xform.point(j["positions"][1]),
                       mesh->xform.point(j["positions"][2])};

    // now check for optional normals and uvs
    if (j.contains("normals") && j.at("normals").is_array())
    {
        if (j.at("normals").size() == 3)
        {
            mesh->ns = {mesh->xform.normal(j["normals"][0]), mesh->xform.normal(j["normals"][1]),
                        mesh->xform.normal(j["normals"][2])};
            mesh->Fn = mesh->Fv;
        }
        else
            spdlog::warn("optional \"normals\" field should be an array of three Vec3s, skipping");
    }

    if (j.contains("uvs") && j.at("uvs").is_array())
    {
        if (j.at("uvs").size() == 3)
        {
            mesh->uvs = {j["uvs"][0], j["uvs"][1], j["uvs"][2]};
            mesh->Ft  = mesh->Fv;
        }
        else
            spdlog::warn("optional \"uvs\" field should be an array of three Vec2s, skipping");
    }

    m_mesh = mesh;
}

Triangle::Triangle(const json &j, shared_ptr<const Mesh> mesh, uint32_t tri_number) :
    m_mesh(mesh), m_face_idx(tri_number)
{
}

STAT_RATIO("Intersections/Triangle intersection tests per hit", num_tri_tests, num_tri_hits);

bool Triangle::intersect(const Ray3f &ray, HitInfo &hit) const
{
    ++num_tri_tests;

    auto iv0 = m_mesh->Fv[m_face_idx].x, iv1 = m_mesh->Fv[m_face_idx].y, iv2 = m_mesh->Fv[m_face_idx].z;
    auto p0 = m_mesh->vs[iv0], p1 = m_mesh->vs[iv1], p2 = m_mesh->vs[iv2];

    const Vec3f *n0 = nullptr, *n1 = nullptr, *n2 = nullptr;
    if (m_mesh->Fn.size() > m_face_idx)
    {
        auto in0 = m_mesh->Fn[m_face_idx].x, in1 = m_mesh->Fn[m_face_idx].y, in2 = m_mesh->Fn[m_face_idx].z;
        if (in0 >= 0 && in1 >= 0 && in2 >= 0)
        {
            n0 = &m_mesh->ns[in0];
            n1 = &m_mesh->ns[in1];
            n2 = &m_mesh->ns[in2];
        }
    }
    const Vec2f *t0 = nullptr, *t1 = nullptr, *t2 = nullptr;
    if (m_mesh->Ft.size() > m_face_idx)
    {
        auto it0 = m_mesh->Ft[m_face_idx].x, it1 = m_mesh->Ft[m_face_idx].y, it2 = m_mesh->Ft[m_face_idx].z;
        if (it0 >= 0 && it1 >= 0 && it2 >= 0)
        {
            t0 = &m_mesh->uvs[it0];
            t1 = &m_mesh->uvs[it1];
            t2 = &m_mesh->uvs[it2];
        }
    }

    return single_triangle_intersect(ray, p0, p1, p2, n0, n1, n2, t0, t1, t2, hit,
                                     m_mesh->materials[m_mesh->Fm[m_face_idx]].get());
}

// Ray-Triangle intersection
// p0, p1, p2 - Triangle vertices
// n0, n1, n2 - optional per vertex normal data
// t0, t1, t2 - optional per vertex texture coordinates
bool single_triangle_intersect(const Ray3f &ray, const Vec3f &p0, const Vec3f &p1, const Vec3f &p2, const Vec3f *n0,
                               const Vec3f *n1, const Vec3f *n2, const Vec2f *t0, const Vec2f *t1, const Vec2f *t2,
                               HitInfo &hit, const Material *material)
{
    ++g_num_total_intersection_tests;
    // TODO: Implement ray-triangle intersection
    // TODO: If the ray misses the triangle, you should return false
    //       You can pick any ray triangle intersection routine you like.
    //       I recommend you follow "Approach 3" from lecture, which is the
    //       Moller-Trumbore algorithm

    put_your_code_here("Insert your ray-triangle intersection code here");
    return false;

    // First, check for intersection and fill in the hit distance t
    float t = 0.0f;
    // You should also compute the u/v (i.e. the alpha/beta barycentric coordinates) of the hit point
    // (Moller-Trumbore gives you this for free)
    float u, v;

    // TODO: If you successfully hit the triangle, you should check if the distance t lies
    //       within the ray's mint/maxt, and return false if it does not

    // TODO: Fill in the gn with the geometric normal of the triangle (i.e. normalized cross product of
    // two edges)
    Vec3f gn = Vec3f(0.0f);

    // Compute the shading normal
    Vec3f sn;
    if (n0 != nullptr && n1 != nullptr && n2 != nullptr)
    { // Do we have per-vertex normals available?
        // We do -> dereference the pointers
        Vec3f normal0 = *n0;
        Vec3f normal1 = *n1;
        Vec3f normal2 = *n2;

        // TODO: You should compute the shading normal by
        //       doing barycentric interpolation of the per-vertex normals (normal0/1/2)
        //       Make sure to normalize the result
        sn = Vec3f(0.0f);
    }
    else
    {
        // We don't have per-vertex normals - just use the geometric normal
        sn = gn;
    }

    // Because we've hit the triangle, fill in the intersection data
    hit.t   = t;
    hit.p   = ray(t);
    hit.gn  = gn;
    hit.sn  = sn;
    hit.uv  = Vec2f(u, v);
    hit.mat = material;
    ++num_tri_hits;
    return true;
}

Box3f Triangle::bounds() const
{
    // all mesh vertices have already been transformed to world space,
    // so just bound the triangle vertices
    Box3f result;
    result.enclose(vertex(0));
    result.enclose(vertex(1));
    result.enclose(vertex(2));

    // if the triangle lies in an axis-aligned plane, expand the box a bit
    auto diag = result.diagonal();
    for (int i = 0; i < 3; ++i)
    {
        if (diag[i] < 1e-4f)
        {
            result.min[i] -= 5e-5f;
            result.max[i] += 5e-5f;
        }
    }
    return result;
}


DARTS_REGISTER_CLASS_IN_FACTORY(Surface, Triangle, "triangle")

/**
    \file
    \brief Triangle Surface
*/
