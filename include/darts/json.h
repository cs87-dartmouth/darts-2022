/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/spherical.h>
#include <darts/transform.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4706)
#endif
#include <nlohmann/json.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

/** \addtogroup Parser
    @{
*/

using json = nlohmann::json; ///< Bring nlohmann::json into scope

// The below functions allow us to parse matrices, vectors, and other common darts types from a #json object
namespace linalg
{

/** \addtogroup Parser
    @{
*/

/// parse a Mat44<T> from json
template <class T>
void from_json(const json &j, mat<T, 4, 4> &m)
{
    if (j.count("from") || j.count("at") || j.count("to") || j.count("up"))
    {
        Vec3<T> from(0, 0, 1), at(0, 0, 0), up(0, 1, 0);
        from = j.value("from", from);
        at   = j.value("at", at) + j.value("to", at);
        up   = j.value("up", up);

        spdlog::info("up vector is {}", up);

        Vec3<T> dir   = normalize(from - at);
        Vec3<T> left  = normalize(cross(up, dir));
        Vec3<T> newUp = normalize(cross(dir, left));

        m = Mat44<T>(Vec4<T>(left, 0.f), Vec4<T>(newUp, 0.f), Vec4<T>(dir, 0.f), Vec4<T>(from, 1.f));
        // m = inverse(lookat_matrix(j.value("from", Vec3<T>{0, 0, 1}),
        //                           j.value("at", Vec3<T>{0.f}) + j.value("to", Vec3<T>{0.f}),
        //                           j.value("up", Vec3<T>{0, 1, 0})));
    }
    else if (j.count("o") || j.count("x") || j.count("y") || j.count("z"))
    {
        Vec3<T> o(0, 0, 0), x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
        o = j.value("o", o);
        x = j.value("x", x);
        y = j.value("y", y);
        z = j.value("z", z);
        m = Mat44<T>(Vec4<T>(x, 0.f), Vec4<T>(y, 0.f), Vec4<T>(z, 0.f), Vec4<T>(o, 1.f));
    }
    else if (j.count("translate"))
    {
        m = translation_matrix(j.value("translate", Vec3<T>(0.f)));
    }
    else if (j.count("scale"))
    {
        m = scaling_matrix(j.value("scale", Vec3<T>(1)));
    }
    else if (j.count("rotate"))
    {
        auto v = j.value("rotate", Vec4<T>{0, 1, 0, 0});
        m      = rotation_matrix(rotation_quat(Vec3<T>{v.y, v.z, v.w}, Spherical::deg2rad(v.x)));
    }
    else if (j.count("matrix"))
    {
        json jm = j.at("matrix");
        if (jm.size() == 1)
        {
            m = ::Mat44<T>(jm.get<T>());
            spdlog::warn("Incorrect array size when trying to parse a Matrix. "
                         "Expecting 4 x 4 = 16 values but only found a single scalar. "
                         "Creating a 4 x 4 scaling matrix with '{}'s along the diagonal.",
                         jm.get<float>());
            return;
        }
        else if (16 != jm.size())
        {
            throw DartsException("Incorrect array size when trying to parse a Matrix. "
                                 "Expecting 4 x 4 = 16 values but found {}, here:\n{}.",
                                 jm.size(), jm.dump(4));
        }

        // jm.size() == 16
        for (auto row : range(4))
            for (auto col : range(4))
                jm.at(row * 4 + col).get_to(m[col][row]);
    }
    else
        throw DartsException("Unrecognized 'transform' command:\n{}.", j.dump(4));
}

/// parse a Vec<N,T> from json
template <class T, int N>
inline void from_json(const json &j, vec<T, N> &v)
{
    if (j.is_object())
        throw DartsException("Can't parse a Vec{}. Expecting a json array, but got a json object.", N);

    if (j.size() == 1)
    {
        if (j.is_array())
            spdlog::info("Incorrect array size when trying to parse a Vec3. "
                         "Expecting {} values but only found 1. "
                         "Creating a Vec of all '{}'s.",
                         N, j.get<T>());
        v = vec<T, N>(j.get<T>());
        return;
    }
    else if (N != j.size())
    {
        throw DartsException("Incorrect array size when trying to parse a Vec. "
                             "Expecting {} values but found {} here:\n{}",
                             N, (int)j.size(), j.dump(4));
    }

    // j.size() == N
    for (auto i : range((int)j.size()))
        j.at(i).get_to(v[i]);
}

/// Serialize a Mat44<T> to json
template <class T>
inline void to_json(json &j, const mat<T, 4, 4> &v)
{
    j.at("matrix") = vector<T>(reinterpret_cast<const T *>(&v.x), reinterpret_cast<const T *>(&v.x) + 16);
}

/// Serialize a Vec3<N,T> to json
template <class T, int N>
inline void to_json(json &j, const vec<T, N> &v)
{
    j = vector<T>(&(v[0]), &(v[0]) + N);
}

/** @}*/

} // namespace linalg

/// Parse a Transform from json
inline void from_json(const json &j, Transform &v)
{
    Mat44f m = v.m;
    if (j.is_array())
    {
        // multiple transformation commands listed in order
        for (auto &element : j)
            m = mul(element.get<Mat44f>(), m);
    }
    else if (j.is_object())
    {
        // a single transformation
        j.get_to(m);
    }
    else
        throw DartsException("'transform' must be either an array or an object here:\n{}", j.dump(4));

    v = Transform(m);
}

/// Serialize a Transform to json
inline void to_json(json &j, const Transform &t)
{
    to_json(j, t.m);
}

/** @}*/

/**
    \file
    \brief Serialization and deserialization of various darts types to/from JSON
*/
