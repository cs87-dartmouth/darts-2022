/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
    ------------------------------------------------------------------------
    This file is based on the Transform class from Nori:

    Copyright (c) 2015 by Wenzel Jakob
*/
#pragma once

#include <darts/common.h>
#include <darts/ray.h>
#include <darts/box.h>

/**
    Homogeneous coordinate transformation

    This class stores a general homogeneous coordinate transformation, such as rotation, translation, uniform or
    non-uniform scaling, and perspective transformations. The inverse of this transformation is also recorded here,
    since it is required when transforming normal vectors.

    \ingroup Math
*/
struct Transform
{
    Mat44f m;
    Mat44f m_inv;

    /// Create the identity transform
    Transform() : m(la::identity), m_inv(la::identity)
    {
    }

    /// Create a new transform instance for the given matrix
    Transform(const Mat44f &m) : m(m), m_inv(la::inverse(m))
    {
    }

    /// Create a new transform instance for the given matrix and its inverse
    Transform(const Mat44f &trafo, const Mat44f &inv) : m(trafo), m_inv(inv)
    {
    }

    /// Return the inverse transformation
    Transform inverse() const
    {
        return Transform(m_inv, m);
    }

    /// Concatenate with another transform
    Transform operator*(const Transform &t) const
    {
        return Transform(mul(m, t.m), mul(t.m_inv, m_inv));
    }

    /// Apply the homogeneous transformation to a 3D direction vector
    Vec3f vector(const Vec3f &v) const
    {
        // TODO: Implement the transformation of a 3D direction vector by the transform matrix m. A direction vector
        // should be transformed without any translation applied -- meaning you should interpret the vector v to be a
        // 4-vector with a 0 in the 4th component.You should return a three-vector through. Given a Vec4f v, you can
        // extract just the xyz components by calling the v.xyz() method.
        return Vec3f(0.0f, 0.0f, 0.0f);
    }

    /// Apply the homogeneous transformation to a 3D normal
    Vec3f normal(const Vec3f &n) const
    {
        // TODO: Implement the transformation of a normal by the transform matrix m. Note that normals need to be
        // transformed differently than direction vectors; you need to multiply by the transpose of the inverse matrix.
        // The inverse transformation matrix has already been computed for you: m_inv. Similar to vectors, you should
        // not apply translation - use homogeneous coordinate to ensure these are excluded. Make sure to return a
        // normalized (unit-length) transformed normal vector.
        return Vec3f(0.0f, 0.0f, 0.0f);
    }

    /// Transform a point by an arbitrary matrix in homogeneous coordinates
    Vec3f point(const Vec3f &p) const
    {
        // TODO: Implement the transformation of a point by the transform matrix m. Here we want to apply translation -
        // meaning you should interpret the point p to be a 4-vector with a 1 in the 4th component.
        //
        // The result of the transform is another 4-vector, (x, y, z, w). You should return the first 3 elements of this
        // vector (use the .xyz() method), divided by the 4th coordinate
        put_your_code_here("Assignment 1: insert your Transform*Vec3f code here");
        return Vec3f(0.0f, 0.0f, 0.0f);
    }

    /// Apply the homogeneous transformation to a ray
    Ray3f ray(const Ray3f &r) const
    {
        // TODO: Transform a ray by this transform. A ray consists of an origin, the point r.o, and a direction, r.d.
        // Transform these, and return a new ray with the transformed coordinates.

        // IMPORTANT: The ray you return should have the same mint and maxt as the original ray
        put_your_code_here("Assignment 1: insert your Transform*Ray3f code here");
        return Ray3f();
    }

    /// Transform the axis-aligned Box and return the bounding box of the result
    Box3f box(const Box3f &box) const
    {
        // a transformed empty box is still empty
        if (box.is_empty())
            return box;

        // Just in case this is a projection matrix, do things the naive way.
        Vec3f pts[8];

        // Set up the eight points at the corners of the extent
        pts[0].x = pts[1].x = pts[2].x = pts[3].x = box.min.x;
        pts[4].x = pts[5].x = pts[6].x = pts[7].x = box.max.x;

        pts[0].y = pts[1].y = pts[4].y = pts[5].y = box.min.y;
        pts[2].y = pts[3].y = pts[6].y = pts[7].y = box.max.y;

        pts[0].z = pts[2].z = pts[4].z = pts[6].z = box.min.z;
        pts[1].z = pts[3].z = pts[5].z = pts[7].z = box.max.z;

        Box3f new_box(point(pts[0]));
        for (size_t i = 1; i < 8; ++i)
            new_box.enclose(point(pts[i]));

        return new_box;
    }

    static Transform translate(const Vec3f &t)
    {
        return Transform(
            Mat44f({1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {t.x, t.y, t.z, 1.f}));
    }

    static Transform axisOffset(const Vec3f &x, const Vec3f &y, const Vec3f &z, const Vec3f &o)
    {
        return Transform(Mat44f({x, 0}, {y, 0}, {z, 0}, {o, 1}));
    }
};

/**
    \file
    \brief Class #Transform
*/
