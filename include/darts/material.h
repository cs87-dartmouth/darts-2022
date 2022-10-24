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

/// Convenience data structure used to pass multiple parameters to the evaluation and sampling routines in #Material
struct ScatterRecord
{
    Color3f attenuation;         ///< Attenuation to apply to the traced ray
    Vec3f   wo;                  ///< The sampled outgoing direction
    bool    is_specular = false; ///< Flag indicating whether the ray has a degenerate PDF
};


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

    /**
        Return whether or not this Material is emissive.
        This is primarily used to create a global list of emitters for sampling.
    */
    virtual bool is_emissive() const
    {
        return false;
    }

    /**
       Sample a scattered direction at the surface hitpoint \p hit.

       If it is not possible to evaluate the pdf of the material (e.g.\ it is
       specular or unknown), then set \c srec.is_specular to true, and populate
       \c srec.wo and \c srec.attenuation just like we did previously in the
       #scatter() function. This allows you to fall back to the way we did
       things with the #scatter() function, i.e.\ bypassing #pdf()
       evaluations needed for explicit Monte Carlo integration in your
       #Integrator, but this also precludes the use of MIS or mixture sampling
       since the pdf is unknown.

       \param  [in] wi      The incoming ray direction (points at surface)
       \param  [in] hit     The incoming ray's intersection with the surface
       \param  [out] srec   Populate \p srec.wo, \p srec.is_specular, and \p srec.attenuation
       \param  [in] rv      A 2D random variables in \f$[0,1)^2\f$ to use when generating the sample
       \param  [in] rv1     A 1D random variable in \f$[0,1)\f$ to use when generating the sample
       \return bool         True if the surface scatters light
    */
    virtual bool sample(const Vec3f &wi, const HitInfo &hit, ScatterRecord &srec, const Vec2f &rv, float rv1) const
    {
        return false;
    }

    /**
       Evaluate the material response for the given pair of directions.

       For non-specular materials, this should be the BSDF multiplied by the
       cosine foreshortening term.

       Specular contributions should be excluded.

       \param  [in] wi          The incoming ray direction
       \param  [in] scattered   The outgoing ray direction
       \param  [in] hit         The shading hit point
       \return Color3f          The evaluated color
    */
    virtual Color3f eval(const Vec3f &wi, const Vec3f &scattered, const HitInfo &hit) const
    {
        return Color3f(0.0f);
    }

    /**
       Compute the probability density that #sample() will generate \p scattered (given \p wi).

       \param  [in] wi          The incoming ray direction
       \param  [in] scattered   The outgoing ray direction
       \param  [in] hit         The shading hit point
       \return float            A probability density value in solid angle measure around \c hit.p.
    */
    virtual float pdf(const Vec3f &wi, const Vec3f &scattered, const HitInfo &hit) const
    {
        return 0.0f;
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