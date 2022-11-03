/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/medium.h>
#include <darts/sampler.h>
#include <darts/scene.h>
#include <limits>

/// A homogeneous medium with the same scattering properties everywhere
/// \ingroup Media
class HomogeneousMedium : public Medium
{
protected:
    Color3f m_albedo{0.8f};
    Color3f m_total{1.f};
    Color3f m_real_fraction{1.f};

public:
    HomogeneousMedium(const json &j) : Medium(j)
    {
        try
        {
            m_albedo = j.value("albedo", m_albedo);
            m_total  = j.value("total", m_total);

            // "real fraction" specifies what fraction of the total density is due to real (absorbing and scattering)
            // particles and how much is to due fictitious "null" or "delta-scattering" particles. This would typically
            // be set to 1.0, since there is no need to have null particles in a homogeneous medium. But we include it
            // as an explicit parameter because it can be helpful to get null-scattering to work for a homogeneous
            // medium before moving on to support heterogeneous media.
            m_real_fraction = j.value("real fraction", m_real_fraction);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Cannot parse Homogeneous medium specification: \"{}\"", e.what());
        }
    }

    std::tuple<Color3f, Color3f, Color3f> coeffs(const Vec3f &p) const override
    {
        Color3f real_fraction(m_real_fraction);
        return std::make_tuple(m_total * (1.f - m_albedo) * real_fraction, m_total * m_albedo * real_fraction,
                               m_total * (1.f - real_fraction));
    }

    bool sample_free_flight(const Ray3f &ray, int channel, Sampler &sampler, HitInfo &hit, Color3f &f,
                            Color3f &p) const override
    {
        // TODO: implement this function if you want to support homogeneous media
        return false;
    }

    Color3f total_transmittance(const Ray3f &ray, Sampler &sampler) const override
    {
        // TODO: implement this function if you want to support homogeneous media
        return Color3f{1.f};
    }
};

DARTS_REGISTER_CLASS_IN_FACTORY(Medium, HomogeneousMedium, "homogeneous")

/**
    \file
    \brief HomogeneousMedium Medium
*/
