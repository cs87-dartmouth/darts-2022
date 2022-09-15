/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/surface.h>

/// A sphere centered at the origin with radius \ref m_radius. \ingroup Surfaces
class Sphere : public XformedSurfaceWithMaterial
{
public:
    Sphere(float radius, shared_ptr<const Material> material, const Transform &xform = Transform());
    Sphere(const json &j = json::object());

    bool intersect(const Ray3f &ray, HitInfo &hit) const override;

protected:
    float m_radius = 1.0f; ///< The radius of the sphere
};

/**
    \file
    \brief Class #Sphere
*/
