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







/** @}*/

/**
    \file
    \brief Random sampling on various domains
*/