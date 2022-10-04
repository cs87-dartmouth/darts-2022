/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/material.h>
#include <darts/scene.h>

/// A perfectly diffuse (%Lambertian) material. \ingroup Materials
class Lambertian : public Material
{
public:
    Lambertian(const json &j = json::object());

    bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const override;


    Color3f albedo = Color3f(0.8f); ///< The diffuse color (fraction of light that is reflected per color channel).
};

Lambertian::Lambertian(const json &j) : Material(j)
{
    albedo = j.value("albedo", albedo);
}

bool Lambertian::scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
{
    // TODO: Implement Lambertian reflection
    //       You should assign the albedo to ``attenuation'', and
    //       you should assign the scattered ray to ``scattered''
    //       The origin of the scattered ray should be at the hit point,
    //       and the scattered direction is the shading normal plus a random
    //       point on a sphere (please look at the text book for this)

    //       You can get the hit point using hit.p, and the shading normal using hit.sn

    //       Hint: You can use the function random_in_unit_sphere() to get a random
    //       point in a sphere. IMPORTANT: You want to add a random point *on*
    //       a sphere, not *in* the sphere (the text book gets this wrong)
    //       If you normalize the point, you can force it to be on the sphere always, so
    //       add normalize(random_in_unit_sphere()) to your shading normal
    return false;
}


DARTS_REGISTER_CLASS_IN_FACTORY(Material, Lambertian, "lambertian")

/**
    \file
    \brief Lambertian Material
*/
