/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/material.h>
#include <darts/scene.h>

/// A metallic material that reflects light into the (potentially rough) mirror reflection direction.
/// \ingroup Materials
class Metal : public Material
{
public:
    Metal(const json &j = json::object());

    bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const override;


    Color3f albedo = Color3f(0.8f); ///< The reflective color (fraction of light that is reflected per color channel).
    float   roughness = 0.f; ///< A value between 0 and 1 indicating how smooth vs. rough the reflection should be.
};

Metal::Metal(const json &j) : Material(j)
{
    albedo    = j.value("albedo", albedo);
    roughness = clamp(j.value("roughness", roughness), 0.f, 1.f);
}

bool Metal::scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
{
    // TODO: Implement metal reflection
    //       This function proceeds similar to the lambertian material, except that the
    //       scattered direction is different.
    //       Instead of adding a point on a sphere to the normal as before, you should add the point
    //       to the *reflected ray direction*.
    //       You can reflect a vector by the normal using reflect(vector, hit.sn); make sure the vector is normalized.
    //       Unlike before you can't just use random_in_unit_sphere directly; the sphere should be scaled by
    //       roughness. (see text book). In other words, if roughness is 0, the scattered direction should just be the
    //       reflected direction.
    //
    //       This procedure could produce directions below the surface. Handle this by returning false if the scattered
    //       direction and the shading normal point in different directions (i.e. their dot product is negative)
    return false;
}


DARTS_REGISTER_CLASS_IN_FACTORY(Material, Metal, "metal")

/**
    \file
    \brief Metal Material
*/
