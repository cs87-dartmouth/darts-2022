/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/material.h>
#include <darts/scene.h>

/// A smooth dielectric surface that reflects and refracts light according to the specified index of refraction #ior.
/// \ingroup Materials
class Dielectric : public Material
{
public:
    Dielectric(const json &j = json::object());

    bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const override;


    float ior; ///< The (relative) index of refraction of the material
};

Dielectric::Dielectric(const json &j) : Material(j)
{
    ior = j.value("ior", ior);
}

bool Dielectric::scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
{
    // TODO: Implement dielectric scattering
    return false;
}


DARTS_REGISTER_CLASS_IN_FACTORY(Material, Dielectric, "dielectric")
