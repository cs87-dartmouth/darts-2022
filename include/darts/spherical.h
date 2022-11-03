/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/math.h>

/** \addtogroup Math
    @{
*/

/**
    Working with spherical coordinate and angles
*/
namespace Spherical
{

/// Convert radians to degrees
inline float rad2deg(float value)
{
    return value * (180.0f / M_PI);
}

/// Convert degrees to radians
inline float deg2rad(float value)
{
    return value * (M_PI / 180.0f);
}

/**
    Return the sine and cosine in a single function call.

    \note In C++17 you can unpack the result using `auto [s,c] = sincos(theta)`.
*/
template <typename T>
std::pair<T, T> sincos(T arg)
{
    return std::make_pair(std::sin(arg), std::cos(arg));
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       The theta parameter in spherical coordinates
*/
template <typename T>
T theta(const Vec3<T> &v)
{
    return std::acos(std::clamp(v.z, T(-1), T(1)));
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       Cosine of the angle between the local z axis and \p v
*/
template <typename T>
T cos_theta(const Vec3<T> &v)
{
    return v.z;
}

/**

    \param [in] v   Unit direction vector in local coordinates
    \return T       Sine of the angle between the local z axis and \p v
*/
template <typename T>
T sin_theta(const Vec3<T> &v)
{
    float temp = sin_theta2(v);
    if (temp <= T(0))
        return T(0);
    return std::sqrt(temp);
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       Tangent of the angle between the local z axis and \p v
*/
template <typename T>
T tan_theta(const Vec3<T> &v)
{
    float temp = 1 - v.z * v.z;
    if (temp <= T(0))
        return T(0);
    return std::sqrt(temp) / v.z;
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       Squared sine of the angle between the local z axis and \p v
*/
template <typename T>
T sin_theta2(const Vec3<T> &v)
{
    return T(1) - v.z * v.z;
}

/**
    \param [in] v   Direction vector in local coordinates (need not be unit length)
    \return T       The phi parameter in spherical coordinates
*/
template <typename T>
T phi(const Vec3<T> &v)
{
    return std::atan2(-v.y, -v.x) + T(M_PI);
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       Sine of the phi parameter in spherical coordinates
*/
template <typename T>
T sin_phi(const Vec3<T> &v)
{
    float sin_theta = sin_theta(v);
    if (sin_theta == T(0))
        return T(1);
    return std::clamp(v.y / sin_theta, T(-1), T(1));
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       Cosine of the phi parameter in spherical coordinates
*/
template <typename T>
T cos_phi(const Vec3<T> &v)
{
    float sin_theta = sin_theta(v);
    if (sin_theta == T(0))
        return T(1);
    return std::clamp(v.x / sin_theta, T(-1), T(1));
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       Squared sine of the phi parameter in spherical coordinates
*/
template <typename T>
T sin_phi2(const Vec3<T> &v)
{
    return std::clamp(v.y * v.y / sin_theta2(v), T(0), T(1));
}

/**
    \param [in] v   Unit direction vector in local coordinates
    \return T       Squared cosine of the phi parameter in spherical coordinates
*/
template <typename T>
T cos_phi2(const Vec3<T> &v)
{
    return std::clamp(v.x * v.x / sin_theta2(v), T(0), T(1));
}

/**
    Convert spherical (phi,theta) coordinates to a unit direction in Cartesian coordinates

    \param [in] phi_theta
        The spherical angles with \f$\phi \in [0,2\pi)\f$ and \f$\theta \in [0, \pi]\f$.
        \f$\theta = 0 \mapsto (0,0,1)\f$ and \f$\theta = 0 \mapsto (0,0,-1)\f$.
    \return
        The corresponding unit-length direction vector in Cartesian coordinates.
*/
template <typename T>
Vec3<T> spherical_coordinates_to_direction(const Vec2<T> &phi_theta)
{
    auto [sin_theta, cos_theta] = sincos(phi_theta.y);
    auto [sin_phi, cos_phi]     = sincos(phi_theta.x);

    return Vec3<T>{sin_theta * cos_phi, sin_theta * sin_phi, cos_theta};
}

/**
    Convert a unit direction from Cartesian coordinates to spherical (phi,theta) coordinates

    \param [in] v
        The direction vector in Cartesian coordinates (assumed to be unit length)
    \return
        The spherical angles with \f$\phi \in [0,2\pi)\f$ and \f$\theta \in [0, \pi]\f$.
        \f$\theta = 0 \mapsto (0,0,1)\f$ and \f$\theta = 0 \mapsto (0,0,-1)\f$.
*/
template <typename T>
Vec2<T> direction_to_spherical_coordinates(const Vec3<T> &v)
{
    return {phi(v), theta(v)};
}


template <typename T>
Vec2<T> direction_to_equirectangular_range(const Vec3<T> &dir, const Vec4<T> &range)
{
    if (la::all(la::equal(dir, T(0))))
        return Vec2<T>(0, 0);

    return {(atan2(dir.y, dir.x) - range.y) / range.x, (theta(dir) - range.w) / range.z};
}

template <typename T>
Vec3<T> equirectangular_range_to_direction(const Vec2<T> &uv, const Vec4<T> &range)
{
    auto [sin_theta, cos_theta] = sincos(range.z * uv.y + range.w);
    auto [sin_phi, cos_phi]     = sincos(range.x * uv.x + range.y);
    return Vec3<T>{sin_theta * cos_phi, sin_theta * sin_phi, cos_theta};
}

template <typename T>
Vec2<T> direction_to_equirectangular(const Vec3<T> &dir)
{
    return direction_to_equirectangular_range(dir, Vec4<T>{-2 * M_PI, M_PI, -M_PI, M_PI});
}

template <typename T>
Vec3<T> equirectangular_to_direction(const Vec2<T> &uv)
{
    return equirectangular_range_to_direction(uv, Vec4<T>(-2 * M_PI, M_PI, -M_PI, M_PI));
}


} // namespace Spherical

/** @}*/

/**
    \file
    \brief Namespace #Spherical
*/
