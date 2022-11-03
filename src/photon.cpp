/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#include <darts/photon.h>

bool  Photon::m_precomp_table_ready = Photon::initialize();
float Photon::m_cos_theta[256];
float Photon::m_sin_theta[256];
float Photon::m_cos_phi[256];
float Photon::m_sin_phi[256];
float Photon::m_exp_table[256];

bool Photon::initialize()
{
    for (int i = 0; i < 256; i++)
    {
        float angle    = (float)i * ((float)M_PI / 256.0f);
        m_cos_phi[i]   = std::cos(2.0f * angle);
        m_sin_phi[i]   = std::sin(2.0f * angle);
        m_cos_theta[i] = std::cos(angle);
        m_sin_theta[i] = std::sin(angle);
        m_exp_table[i] = std::ldexp((float)1, i - (128 + 8));
    }
    m_exp_table[0] = 0;
    return true;
}

Photon::Photon(const Vec3f &dir, const Color3f &power)
{
    if (!la::all(gequal(power, 0.f) & la::isfinite(power)))
        spdlog::warn("Creating an invalid photon with power: {}", power);

    // Convert the direction into an approximate spherical coordinate format to reduce storage requirements
    theta = (uint8_t)std::min(255, (int)(std::acos(dir.z) * (256.0f / M_PI)));

    int tmp = std::min(255, (int)(std::atan2(dir.y, dir.x) * (256.0f / (2.0f * M_PI))));
    if (tmp < 0)
        phi = (uint8_t)(tmp + 256);
    else
        phi = (uint8_t)tmp;

    // Convert to Ward's RGBE format
    float max = la::maxelem(power);
    if (max < 1e-32)
    {
        rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
    }
    else
    {
        int e;
        // Extract exponent and convert the fractional part into the [0..255] range. Afterwards, divide by max so that
        // any color component multiplied by the result will be in [0,255]
        max     = std::frexp(max, &e) * 256.0f / max;
        rgbe[0] = (uint8_t)(power.x * max);
        rgbe[1] = (uint8_t)(power.y * max);
        rgbe[2] = (uint8_t)(power.z * max);
        rgbe[3] = e + 128; // Exponent value in bias format
    }
}
