/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/surface_group.h>

SurfaceGroup::SurfaceGroup(const json &j) : XformedSurface(j)
{
    //
    // parse the children
    //
    if (j.contains("children"))
    {
        for (auto &s : j["children"])
        {
            auto child = DartsFactory<Surface>::create(s);
            child->add_to_parent(this, child, j);
            child->build();
        }
    }
}

void SurfaceGroup::add_child(shared_ptr<Surface> surface)
{
    m_surfaces.push_back(surface);
    m_bounds.enclose(m_surfaces.back()->bounds());
}

bool SurfaceGroup::intersect(const Ray3f &ray_, HitInfo &hit) const
{
    // transform the ray into local object space
    auto ray          = m_xform.inverse().ray(ray_);
    bool hit_anything = false;

    // This is a linear intersection test that iterates over all primitives
    // within the scene. It's the most naive intersection test and hence very
    // slow if you have many primitives.
    for (auto surface : m_surfaces)
    {
        if (surface->intersect(ray, hit))
        {
            hit_anything = true;
            ray.maxt     = hit.t;
        }
    }

    // transform the hit information back
    hit.p  = m_xform.point(hit.p);
    hit.gn = normalize(m_xform.normal(hit.gn));
    hit.sn = normalize(m_xform.normal(hit.sn));

    // record closest intersection
    return hit_anything;
}

Box3f SurfaceGroup::local_bounds() const
{
    return m_bounds;
}


DARTS_REGISTER_CLASS_IN_FACTORY(Surface, SurfaceGroup, "group")