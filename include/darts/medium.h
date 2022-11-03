/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/common.h>
#include <darts/factory.h>
#include <darts/fwd.h>
#include <darts/sampler.h>
#include <darts/surface.h>

/// Generic interface for participating media supporting transmittance evaluation and free-flight sampling
/// \ingroup Media
class Medium
{
public:
    /// Default constructor which accepts a #json object of named parameters; finds the `"phase function"` for the
    /// medium
    Medium(const json &j) : phase_function(DartsFactory<Material>::find(j, "phase function"))
    {
    }
    virtual ~Medium() = default;

    //! Return the medium coefficients (absorption, scattering, and null) at location \p p.
    virtual std::tuple<Color3f, Color3f, Color3f> coeffs(const Vec3f &p) const
    {
        return std::make_tuple(Color3f(0.f), Color3f(0.f), Color3f(0.f));
    }

    /**
        Importance sample the distance to the next medium interaction along the ray.

        \param ray
            A ray data structure. We try to sample a distance within
            [ray.mint, ray.maxt) along the ray.

        \param channel
            The color channel to sample with respect to. Typically you set this
            to one of the 3 channels uniformly at random for each path that is
            traced. The pdf \p p stores the density for all channels (for
            purposes of MIS) assuming this function is called for each channel
            with equal probability.

        \param[in] sampler
            Provides random numbers for sampling the free-flight distance.

        \param[out] hit
            This parameter is used to return the sampled distance and 3D
            hitpoint location.

        \param[in,out] f
            The passed in value is multiplied by the change in path contribution,
            the transmittance to the sampled distance.

        \param[in,out] p
            The passed in value is multiplied by the probability (density) of
            generating the distance sample. When a position inside the medium
            is sampled, it multiplies by the free-flight PDF (product of the
            total coefficient sigma_t and the transmittance for analog
            sampling). When medium sampling fails, it multiplies by the
            probability of failure.

        \return
            \c true if medium sampling succeeded, and \c false otherwise.
     */
    virtual bool sample_free_flight(const Ray3f &ray, int channel, Sampler &sampler, HitInfo &hit, Color3f &f,
                                    Color3f &p) const = 0;

    /**
        Evaluate the transmittance along the ray between [ray.mint, ray.maxt].

        The transmittance is defined as
        \f[
            \exp\left(-\int_\mathrm{mint}^\mathrm{maxt} \sigma_t(t) \mathrm{d}t\right)
        \f]
        where \f$\sigma_t\f$ is the *total* coefficient (the sum of all coefficients, absorption, scattering, *and* null
        scattering).

        \param ray
            A ray data structure

        \param[in] sampler
            Provides random numbers for sampling the free-flight distance.

        The transmittance evaluation may either be deterministic, in which
        case the \p sampler is ignored. Or it can be random, but under the
        assumption that an unbiased transmittance estimate is returned.

        The default implementation performs track-length estimation using
        #sample_free_flight().
     */
    virtual Color3f total_transmittance(const Ray3f &ray, Sampler &sampler) const
    {
        Color3f f(1.f), p(1.f);
        HitInfo hit;
        int     c = std::min(int(sampler.next1f() * 3), 2); // select color channel uniformly at random
        return sample_free_flight(ray, c, sampler, hit, f, p) ? Color3f(0.f) : f / p;
    }

    shared_ptr<Material> phase_function; ///< Pointer to the phase function associated with this medium
};

/**
    \file
    \brief Class #Medium
*/
