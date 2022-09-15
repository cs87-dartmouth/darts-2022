/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#include <darts/scene.h>
#include <darts/stats.h>

/// A quad spanning from -\ref m_size / 2 to \ref m_size / 2 in the (x,y)-plane at z=0. \ingroup Surfaces
class Quad : public XformedSurfaceWithMaterial
{
public:
    Quad(const json &j = json::object());

    bool intersect(const Ray3f &ray, HitInfo &hit) const override;

protected:
    Vec2f m_size = Vec2f(1.f); ///< The extent of the quad in the (x,y) plane
};

Quad::Quad(const json &j) : XformedSurfaceWithMaterial(j)
{
    m_size = j.value("size", m_size);
    m_size /= 2.f;

}

STAT_RATIO("Intersections/Quad intersection tests per hit", num_quad_tests, num_quad_hits);

bool Quad::intersect(const Ray3f &ray, HitInfo &hit) const
{
    ++num_quad_tests;

    // compute ray intersection (and ray parameter), continue if not hit
    auto tray = m_xform.inverse().ray(ray);
    if (tray.d.z == 0)
        return false;
    auto t = -tray.o.z / tray.d.z;
    auto p = tray(t);

    if (m_size.x < std::abs(p.x) || m_size.y < std::abs(p.y))
        return false;

    // check if computed param is within ray.mint and ray.maxt
    if (t < tray.mint || t > tray.maxt)
        return false;

    // project hitpoint onto plane to reduce floating-point error
    p.z = 0;

    // if hit, set intersection record values
    hit.t  = t;
    hit.p  = m_xform.point(p);
    hit.gn = hit.sn = normalize(m_xform.normal({0, 0, 1}));
    hit.mat         = m_material.get();
    hit.uv = Vec2f(0.0f, 0.0f); /* TODO: Compute proper UV coordinates */

    ++num_quad_hits;
    return true;
}




DARTS_REGISTER_CLASS_IN_FACTORY(Surface, Quad, "quad")

/**
    \file
    \brief Class #Quad
*/
