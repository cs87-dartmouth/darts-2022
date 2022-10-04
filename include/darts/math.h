/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include "linalg.h"
#include <algorithm>
#include <fmt/format.h>
#if defined(_WIN32)
#pragma warning(disable : 4305) // double constant assigned to float
#pragma warning(disable : 4244) // int -> float conversion
#pragma warning(disable : 4843) // double -> float conversion
#pragma warning(disable : 4267) // size_t -> int
#pragma warning(disable : 4838) // another double -> int
#endif

/** \addtogroup Math
    @{
*/

/** \name Basic darts vector/matrix/color types
    @{
*/

// Shortname for the linalg namespace
namespace la = linalg;

template <int N, class T>
using Vec = la::vec<T, N>; ///< Generic \p N dimensional vector
template <class T, int M, int N>
using Mat = la::mat<T, M, N>; ///< Generic \p M x \p N matrix

template <class T>
using Vec2 = Vec<2, T>;
template <class T>
using Vec3 = Vec<3, T>;
template <class T>
using Vec4 = Vec<4, T>;

template <class T>
using Color3 = Vec<3, T>; ///< RGB color of type T
template <class T>
using Color4 = Vec<4, T>; ///< RGBA color of type T

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec2i = Vec2<std::int32_t>;
using Vec2u = Vec2<std::uint32_t>;
using Vec2c = Vec2<std::uint8_t>;

using Vec3f   = Vec3<float>;
using Vec3d   = Vec3<double>;
using Vec3i   = Vec3<std::int32_t>;
using Vec3u   = Vec3<std::uint32_t>;
using Vec3c   = Vec3<std::uint8_t>;
using Color3f = Vec3<float>;
using Color3d = Vec3<double>;
using Color3u = Vec3<std::uint32_t>;
using Color3c = Vec3<std::uint8_t>;

using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
using Vec4i = Vec4<std::int32_t>;
using Vec4u = Vec4<std::uint32_t>;
using Vec4c = Vec4<std::uint8_t>;

using Color4f = Vec4<float>;
using Color4d = Vec4<double>;
using Color4u = Vec4<std::uint32_t>;
using Color4c = Vec4<std::uint8_t>;

template <class T>
using Mat22 = la::mat<T, 2, 2>;
template <class T>
using Mat33 = la::mat<T, 3, 3>;
template <class T>
using Mat44 = la::mat<T, 4, 4>;

using Mat22f = Mat22<float>;
using Mat22d = Mat22<double>;
using Mat33f = Mat33<float>;
using Mat33d = Mat33<double>;
using Mat44f = Mat44<float>;
using Mat44d = Mat44<double>;

/** @}*/

/** \name Math constants
    \brief A few useful constants
    @{
*/
#undef M_PI
#define M_PI         3.14159265358979323846f ///< \f$\pi\f$
#define INV_PI       0.31830988618379067154f ///< \f$\frac{1}{\pi}\f$
#define INV_TWOPI    0.15915494309189533577f ///< \f$\frac{1}{2\pi}\f$
#define INV_FOURPI   0.07957747154594766788f ///< \f$\frac{1}{4\pi}\f$
#define SQRT_TWO     1.41421356237309504880f ///< \f$\sqrt{2}\f$
#define INV_SQRT_TWO 0.70710678118654752440f ///< \f$\frac{1}{\sqrt{2}}\f$
/** @}*/

/** \name Integer powers
    \brief Convenience functions for the first few integer powers
    @{
*/
template <typename T>
inline T pow2(T x)
{
    return x * x;
}
template <typename T>
inline T pow3(T x)
{
    return x * x * x;
}
template <typename T>
inline T pow4(T x)
{
    T x2 = x * x;
    return x2 * x2;
}
template <typename T>
inline T pow5(T x)
{
    T x2 = x * x;
    return x2 * x2 * x;
}
template <typename T>
inline T sqr(T x)
{
    return pow2(x);
}
template <typename T>
inline T cube(T x)
{
    return pow3(x);
}

/** @}*/

//
// allow applying isfinite and isnan to vectors
//
namespace linalg
{

namespace detail
{
struct std_isfinite
{
    template <class A>
    auto operator()(A a) const -> decltype(std::isfinite(a))
    {
        return std::isfinite(a);
    }
};

struct std_isnan
{
    template <class A>
    auto operator()(A a) const -> decltype(std::isnan(a))
    {
        return std::isnan(a);
    }
};
} // namespace detail

template <class A>
apply_t<detail::std_isfinite, A> isfinite(const A &a)
{
    return apply(detail::std_isfinite{}, a);
}
template <class A>
apply_t<detail::std_isnan, A> isnan(const A &a)
{
    return apply(detail::std_isnan{}, a);
}

} // namespace linalg

/** \name Interpolation utilities
    @{
*/

/**
    Linear interpolation.

    Linearly interpolates between \p a and \p b, using parameter \p t.

    \tparam T    type for start and end points, and return value
    \tparam S    type for interpolation parameter
    \param a     Start point
    \param b     End point
    \param t     A blending factor of \p a and \p b.
    \return      Linear interpolation of \p a and \p b -
                 a value between \p a and \p b if \p t is between 0 and 1.
*/
using la::lerp;

/// Always-positive modulo operation
template <typename T>
inline T mod(T a, T b)
{
    int n = (int)(a / b);
    a -= n * b;
    if (a < 0)
        a += b;
    return a;
}


/** @}*/

/**
    Construct an orthonormal coordinate system given one vector \p a.

    \param [in]  z  The coordinate system's local z axis direction.
    \return         The local x and y-axes orthogonal to z.
*/
template <typename T>
std::pair<Vec3<T>, Vec3<T>> coordinate_system(const Vec3<T> &z)
{
    if (std::abs(z.x) > std::abs(z.y))
    {
        T       inv_len = T(1) / std::sqrt(z.x * z.x + z.z * z.z);
        Vec3<T> y{z.z * inv_len, 0, -z.x * inv_len};
        return {cross(y, z), y};
    }
    else
    {
        T       inv_len = T(1) / std::sqrt(z.y * z.y + z.z * z.z);
        Vec3<T> y{0, z.z * inv_len, -z.y * inv_len};
        return {cross(y, z), y};
    }
}

/** \name Color utilities
    \brief Working with colors
    @{
*/

/// Convert from linear RGB to sRGB
inline Color3f to_sRGB(const Color3f &c)
{
    return la::select(la::lequal(c, 0.0031308f), c * 12.92f, (1.0f + 0.055f) * la::pow(c, 1.0f / 2.4f) - 0.055f);
}

/// Convert from sRGB to linear RGB
inline Color3f to_linear_RGB(const Color3f &c)
{
    return la::select(la::lequal(c, 0.04045f), c * (1.0f / 12.92f), la::pow((c + 0.055f) * (1.0f / 1.055f), 2.4f));
}

/// Return the associated luminance
inline float luminance(const Color3f &c)
{
    return dot(c, {0.212671f, 0.715160f, 0.072169f});
}

// Matplotlib-style false-color maps
Color3f viridis(float t);
Color3f inferno(float t);
Color3f magma(float t);
Color3f plasma(float t);


/** @}*/


//
// The contents below are largely details that you do not need to understand.
// It allows us to use the fmt format/print, spdlog logging functions, and
// standard C++ iostreams like cout with darts vectors, colors and matrices.
//

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//
// Base class for both vec and mat fmtlib formatters.
//
// Based on the great blog tutorial: https://wgml.pl/blog/formatting-user-defined-types-fmt.html
//
template <typename V, typename T, bool Newline>
struct vecmat_formatter
{
    using underlying_formatter_type = fmt::formatter<T>;

    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return underlying_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const V &v, FormatContext &ctx)
    {
        fmt::format_to(ctx.out(), "{{");
        auto it = begin(v);
        while (true)
        {
            ctx.advance_to(underlying_formatter.format(*it, ctx));
            if (++it == end(v))
            {
                fmt::format_to(ctx.out(), "}}");
                break;
            }
            else
                fmt::format_to(ctx.out(), ",{} ", Newline ? "\n" : "");
        }
        return ctx.out();
    }

protected:
    underlying_formatter_type underlying_formatter;
};

template <typename T, int N>
struct fmt::formatter<la::vec<T, N>> : public vecmat_formatter<la::vec<T, N>, T, false>
{
};

template <typename T, int M, int N>
struct fmt::formatter<la::mat<T, M, N>> : public vecmat_formatter<la::mat<T, M, N>, la::vec<T, N>, true>
{
};

#ifdef DARTS_IOSTREAMS
#include <iomanip>
#include <iostream>
template <class C, int N, class T>
std::basic_ostream<C> &operator<<(std::basic_ostream<C> &out, const Vec<N, T> &v)
{
    std::ios_base::fmtflags oldFlags = out.flags();
    auto                    width    = out.precision() + 2;

    out.setf(std::ios_base::right);
    if (!(out.flags() & std::ios_base::scientific))
        out.setf(std::ios_base::fixed);
    width += 5;

    out << '{';
    for (size_t i = 0; i < N - 1; ++i)
        out << std::setw(width) << v[i] << ',';
    out << std::setw(width) << v[N - 1] << '}';

    out.flags(oldFlags);
    return out;
}

template <class C, class T>
std::basic_ostream<C> &operator<<(std::basic_ostream<C> &s, const Mat44<T> &m)
{
    return s << "{" << m[0] << ",\n " << m[1] << ",\n " << m[2] << ",\n " << m[3] << "}";
}
#endif

#endif

/** @}*/

/**
    \file
    \brief Contains various classes for linear algebra: vectors, matrices, rays, axis-aligned bounding boxes.
*/
