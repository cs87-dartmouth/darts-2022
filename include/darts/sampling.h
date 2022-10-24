/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/common.h>
#include <darts/spherical.h>
#include <pcg32.h>

/** \addtogroup Random
    @{

    The Random module provides a random number generator suitable for ray tracing (via Wenzel Jakob's tiny
    [pcg32](https://github.com/wjakob/pcg32) library), and several functions to generate points and directions useful in
    path tracing and procedural generation.
*/

/** \name Global RNG and rejection sampling
    @{
*/

/// Global random number generator that produces floats between <tt>[0,1)</tt>
inline float randf()
{
    static pcg32 rng = pcg32();
    return rng.nextFloat();
}

/// Sample a random point uniformly within a unit sphere (uses the global randf() RNG and rejection sampling)
inline Vec3f random_in_unit_sphere()
{
    Vec3f p;
    do
    {
        float a = randf();
        float b = randf();
        float c = randf();
        p       = 2.0f * Vec3f(a, b, c) - Vec3f(1);
    } while (length2(p) >= 1.0f);

    return p;
}

/// Sample a random point uniformly within a unit disk (uses the global randf() RNG and rejection sampling)
inline Vec2f random_in_unit_disk()
{
    Vec2f p;
    do
    {
        float a = randf();
        float b = randf();
        p       = 2.0f * Vec2f(a, b) - Vec2f(1);
    } while (length2(p) >= 1.0f);

    return p;
}

/// Hash two integer coordinates (e.g. pixel coordinates) into a pseudo-random unsigned int
inline uint32_t hash2d(int x, int y)
{
    // hash x,y coordinate into a single an unsigned seed
    uint32_t px  = 1103515245u * ((x >> 1u) ^ y);
    uint32_t py  = 1103515245u * ((y >> 1u) ^ x);
    uint32_t h32 = 1103515245u * ((px) ^ (py >> 3u));
    return h32 ^ (h32 >> 16);
}

/** @}*/


/** \name Sampling a disk
    @{
*/

/// Uniformly sample a vector on a 2D disk with radius 1, centered around the origin
inline Vec2f sample_disk(const Vec2f &rv)
{
    float r                 = std::sqrt(rv.y);
    auto [sin_phi, cos_phi] = Spherical::sincos(2.0f * M_PI * rv.x);

    return Vec2f(cos_phi * r, sin_phi * r);
}


/// Probability density of #sample_disk()
inline float sample_disk_pdf(const Vec2f &p)
{
    return length2(p) <= 1 ? INV_PI : 0.0f;
}

/** @}*/



/** \name Sampling a sphere or a ball
    @{
*/

/// Uniformly sample a vector on the unit sphere with respect to solid angles
inline Vec3f sample_sphere(const Vec2f &rv)
{
    return Vec3f{0.f}; // CHANGEME
}

/// Probability density of #sample_sphere()
inline float sample_sphere_pdf()
{
    return 0.f; // CHANGEME
}


/** \name Sampling the hemisphere
    @{
*/

/// Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to solid angles
inline Vec3f sample_hemisphere(const Vec2f &rv)
{
    return Vec3f{0.f}; // CHANGEME
}

/// Probability density of #sample_hemisphere()
inline float sample_hemisphere_pdf(const Vec3f &v)
{
    return 0.f; // CHANGEME
}

/// Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to projected solid
/// angles
inline Vec3f sample_hemisphere_cosine(const Vec2f &rv)
{
    return Vec3f{0.f}; // CHANGEME
}

/// Probability density of #sample_hemisphere_cosine()
inline float sample_hemisphere_cosine_pdf(const Vec3f &v)
{
    return 0.f; // CHANGEME
}

/// Sample a vector on the unit hemisphere with a cosine-power density about the pole (0,0,1)
inline Vec3f sample_hemisphere_cosine_power(float exponent, const Vec2f &rv)
{
    return Vec3f{0.f}; // CHANGEME
}

/// Probability density of #sample_hemisphere_cosine_power()
inline float sample_hemisphere_cosine_power_pdf(float exponent, float cosine)
{
    return 0.f; // CHANGEME
}

/** @}*/



/** \name Sampling a spherical cap
    @{
*/

/**
    Uniformly sample a vector on a spherical cap around (0, 0, 1)

    A spherical cap is the subset of a unit sphere whose directions make an angle of less than 'theta' with the north
    pole. This function expects the cosine of 'theta' as a parameter.
 */
inline Vec3f sample_sphere_cap(const Vec2f &rv, float cos_theta_max)
{
    return Vec3f{0.f}; // CHANGEME
}

/// Probability density of #sample_sphere_cap()
inline float sample_sphere_cap_pdf(float cos_theta, float cos_theta_max)
{
    return 0.f; // CHANGEME
}

/** @}*/

/** \name Sampling a triangle
    @{
*/

/**
    Sample a point uniformly on a triangle with vertices `v0`, `v1`, `v2`.

    \param v0,v1,v2 The vertices of the triangle to sample
    \param rv       Two random variables uniformly distributed in [0,1)
*/
inline Vec3f sample_triangle(const Vec3f &v0, const Vec3f &v1, const Vec3f &v2, const Vec2f &rv)
{
    return Vec3f{0.f}; // CHANGEME
}

/// Sampling density of #sample_triangle()
inline float sample_triangle_pdf(const Vec3f &v0, const Vec3f &v1, const Vec3f &v2)
{
    return 0.f; // CHANGEME
}

/** @}*/



/** @}*/

/**
    \file
    \brief Random sampling on various domains
*/