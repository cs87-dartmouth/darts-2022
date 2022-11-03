/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#include <darts/medium.h>
#include <darts/sampler.h>
#include <darts/scene.h>
#include <limits>

/// A vacuum (empty medium). \ingroup Media
class VacuumMedium : public Medium
{
public:
    VacuumMedium(const json &j) : Medium(j)
    {
    }

    bool sample_free_flight(const Ray3f &ray, int channel, Sampler &sampler, HitInfo &hit, Color3f &f,
                            Color3f &p) const override
    {
        return false;
    }

    Color3f total_transmittance(const Ray3f &ray, Sampler &sampler) const override
    {
        return Color3f(1.f);
    }
};

DARTS_REGISTER_CLASS_IN_FACTORY(Medium, VacuumMedium, "vacuum")

/**
    \file
    \brief VacuumMedium Medium
*/

/**
    \dir
    \brief Darts Medium plugins source directory
*/