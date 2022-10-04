/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/material.h>
#include <darts/scene.h>
#include <darts/surface.h>


Material::Material(const json &j)
{
}

float fresnel_dielectric(float cos_theta_i, float eta_i, float eta_t)
{
    if (eta_t == eta_i)
        return 0.f;

    // Swap the indices of refraction if the interaction starts at the inside of the object
    bool entering = cos_theta_i > 0.0f;
    if (!entering)
    {
        std::swap(eta_t, eta_i);
        cos_theta_i = -cos_theta_i;
    }

    // Using Sahl-Snell's law, calculate the squared sine of the angle between the normal and the transmitted ray
    float eta          = eta_i / eta_t;
    float sin_theta_t2 = eta * eta * (1 - cos_theta_i * cos_theta_i);

    // Total internal reflection!
    if (sin_theta_t2 > 1.0f)
        return 1.0f;

    float cos_theta_t = std::sqrt(1.0f - sin_theta_t2);

    float Rs = (eta_i * cos_theta_i - eta_t * cos_theta_t) / (eta_i * cos_theta_i + eta_t * cos_theta_t);
    float Rp = (eta_t * cos_theta_i - eta_i * cos_theta_t) / (eta_t * cos_theta_i + eta_i * cos_theta_t);

    return 0.5f * (Rs * Rs + Rp * Rp);
}

bool refract(const Vec3f &v_, const Vec3f &n, float eta, Vec3f &refracted)
{
    Vec3f v       = normalize(v_);
    float dt      = dot(v, n);
    float discrim = 1.0f - eta * eta * (1.0f - dt * dt);
    if (discrim > 0)
    {
        refracted = eta * (v - n * dt) - n * std::sqrt(discrim);
        return true;
    }
    else
        return false;
}

/**
    \dir
    \brief Darts Material plugins source directory
*/
/**
    \file
    \brief Material and fresnel utility functions
*/
