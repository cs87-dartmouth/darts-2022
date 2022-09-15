/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/factory.h>
#include <darts/fwd.h>
#include <stdlib.h>

/** \addtogroup Materials
    @{
*/


/// A base class used to represent surface material properties.
class Material
{
public:
    /// Default constructor which accepts a #json object of named parameters
    Material(const json &j = json::object());

    /// Free all memory
    virtual ~Material() = default;

    /**
       \brief Compute the scattered direction scattered at a surface hitpoint.

       The base Material does not scatter any light, so it simply returns false.

       \param  [in] ray             incoming ray
       \param  [in] hit             the ray's intersection with the surface
       \param  [in] attenuation     how much the light should be attenuated
       \param  [in] scattered       the direction light should be scattered
       \return bool                 True if the surface scatters light
     */
    virtual bool scatter(const Ray3f &ray, const HitInfo &hit, Color3f &attenuation, Ray3f &scattered) const
    {
        return false;
    }

    /**
       Compute the amount of emitted light at the surface hitpoint.

       The base Material class does not emit light, so it simply returns black.

       \param  [in] ray		    the incoming ray
       \param  [in] hit		    the ray's intersection with the surface
       \return			        the emitted color
     */
    virtual Color3f emitted(const Ray3f &ray, const HitInfo &hit) const
    {
        return Color3f(0, 0, 0);
    }


};

/**
    Calculates the unpolarized fresnel reflection coefficient for a dielectric material. Handles incidence from either
    side (i.e. `cos_theta_i<0` is allowed).

    \param [in] cos_theta_i
        Cosine of the angle between the normal and the incident ray
    \param [in] ext_ior
        Refractive index of the side that contains the surface normal
    \param [in] int_ior
        Refractive index of the interior
    \return
        The Fresnel reflection coefficient.
*/
float fresnel_dielectric(float cos_theta_i, float ext_ior, float int_ior);

/**
    Generates a refracted direction, assuming refraction is possible based on the incident direction and refractive
    indices.

    \param [in]  v          The incident ray's direction (points at surface)
    \param [in]  n          The surface normal of the incident ray
    \param [in]  eta        The ratio of the index of refraction on the incident side to the index of refraction on the
                            transmitted side
    \param [out] refracted  The returned, un-normalized, refracted direction
    \return                 True if refraction is possible, False if there is total internal reflection
*/
bool refract(const Vec3f &v, const Vec3f &n, float eta, Vec3f &refracted);

/**
    Reflects a vector, \p v, over another vector, \p n.

    \param [in] v   The incident ray's direction (points at surface)
    \param [in] n   The surface normal at the hit point
    \return Vec3f   The reflected vector
*/
inline Vec3f reflect(const Vec3f &v, const Vec3f &n)
{
    return v - 2 * dot(v, n) * n;
}

/** @}*/

/**
    \file
    \brief Class #Material
*/