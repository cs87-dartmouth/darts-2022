/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/math.h>
#include <limits>

/** \addtogroup Math
    @{
*/

/**
    Simple ray segment data structure.

    Along with the ray origin and direction, this data structure additionally stores the segment interval [\ref mint,
    \ref maxt], which may include positive/negative infinity.
*/
template <size_t N, typename T>
struct Ray
{
    /// "Ray epsilon": relative error threshold for ray intersection computations
    static constexpr float epsilon = T(0.0001);
    /// infinity for type \tparam T
    static constexpr float infinity = std::numeric_limits<T>::infinity();

    Vec<N, T> o;    ///< The origin of the ray
    Vec<N, T> d;    ///< The direction of the ray
    T         mint; ///< Minimum distance along the ray segment
    T         maxt; ///< Maximum distance along the ray segment

    /// Construct a new ray
    Ray() : mint(epsilon), maxt(infinity)
    {
    }

    /// Construct a new ray
    Ray(const Vec<N, T> &o, const Vec<N, T> &d, T mint = Ray::epsilon, T maxt = Ray::infinity) :
        o(o), d(d), mint(mint), maxt(maxt)
    {
    }

    /// Copy a ray, but change the covered segment of the copy
    Ray(const Ray &ray, T mint, T maxt) : o(ray.o), d(ray.d), mint(mint), maxt(maxt)
    {
    }

    /// Return the position of a point along the ray
    Vec<N, T> operator()(T t) const
    {
        return o + t * d;
    }
};

template <typename T>
using Ray2 = Ray<2, T>;
template <typename T>
using Ray3 = Ray<3, T>;

using Ray2f = Ray2<float>;
using Ray2d = Ray2<double>;

using Ray3f = Ray3<float>;
using Ray3d = Ray3<double>;


/** @}*/

/**
    \file
    \brief Contains the definition of a generic, N-dimension #Ray class.
*/
