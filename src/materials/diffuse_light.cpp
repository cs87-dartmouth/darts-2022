/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/material.h>
#include <darts/scene.h>

/// A material that emits light equally in all directions from the front side of a surface. \ingroup Materials
class DiffuseLight : public Material
{
public:
    DiffuseLight(const json &j = json::object());

    /// Returns a constant Color3f if the ray hits the surface on the front side.
    Color3f emitted(const Ray3f &ray, const HitInfo &hit) const override;

    bool is_emissive() const override
    {
        return true;
    }


    Color3f emit; ///< The emissive color of the light
};

DiffuseLight::DiffuseLight(const json &j) : Material(j)
{
    emit = j.value("emit", emit);
}

Color3f DiffuseLight::emitted(const Ray3f &ray, const HitInfo &hit) const
{
    // only emit from the normal-facing side
    if (dot(ray.d, hit.sn) > 0)
        return Color3f(0, 0, 0);
    else
        return emit;
}


DARTS_REGISTER_CLASS_IN_FACTORY(Material, DiffuseLight, "diffuse_light")

/**
    \file
    \brief DiffuseLight emissive Material
*/
