/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/factory.h>
#include <darts/math.h>
#include <memory>

/**
    Abstract sample generator.

    A sample generator is responsible for generating the random number stream that will be passed to an #Integrator
    implementation as it computes the radiance incident along a specified ray.

    \ingroup Samplers
*/
class Sampler
{
public:
    Sampler() : m_sample_count(1u), m_current_sample(0u), m_current_dimension(0u)
    {
    }

    virtual ~Sampler()
    {
    }

    /// Create an exact copy of this Sampler instance
    virtual std::unique_ptr<Sampler> clone() const = 0;

    /**
        Set the base seed for the sampler (passed in as a command-line argument).

        Setting the seed of the underlying RNG deterministically is important to produce identical results between runs.

        This function should only need to be called once before rendering starts.
    */
    virtual void set_base_seed(uint32_t s)
    {
        m_base_seed = s;
    }

    /**
        Deterministically seed the sampler for a new scanline or region of the image.

        The base implementation just resets the current dimension and sample index, but derived classes may want to e.g. compute a new
        seed based on the scanline/image region coordinates. If you do so, make sure to still make the seed depend on
        \c m_base_seed.
        \param [in] x   The x coordinate associated with the image region
        \param [in] y   The y coordinate associated with the image region
    */
    virtual void seed(int x, int y)
    {
        m_current_dimension = 0u;
        m_current_sample    = 0u;
    }

    /**
        Prepare to generate samples for pixel (x,y).

        This function is called every time the integrator starts rendering a new pixel.

        The base class simply resets the current dimension to zero, but derived classes may want to e.g. compute a new
        seed based on the pixel coordinates. If you do so, make sure to still make the per-pixel seed depend on
        \c m_base_seed.
    */
    virtual void start_pixel(int x, int y)
    {
        m_current_dimension = 0u;
    }

    /// Advance to the next sample
    virtual void advance()
    {
        m_current_dimension = 0u;
        m_current_sample++;
    }

    /// Retrieve the next float value (dimension) from the current sample
    virtual float next1f() = 0;

    /// Retrieve the next two float values (dimensions) from the current sample
    virtual Vec2f next2f() = 0;

    /// Return the number of configured pixel samples
    virtual uint32_t sample_count() const
    {
        return m_sample_count;
    }

    uint32_t current_sample() const
    {
        return m_current_sample;
    }

    uint32_t current_dimension() const
    {
        return m_current_dimension;
    }

protected:
    uint32_t m_base_seed;
    uint32_t m_sample_count;
    uint32_t m_current_sample;
    uint32_t m_current_dimension;
};

/**
    \file
    \brief Class #Sampler
*/
