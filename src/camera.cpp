/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#include <darts/camera.h>
#include <darts/stats.h>

STAT_COUNTER("Integrator/Camera rays traced", num_camera_rays);

Camera::Camera(const json &j)
{
    m_xform           = j.value("transform", m_xform);
    m_resolution      = j.value("resolution", m_resolution);
    m_focal_distance  = j.value("fdist", m_focal_distance);
    m_aperture_radius = j.value("aperture", m_aperture_radius);

    float vfov = 90.f; // Default vfov value. Override this with the value from json
    // TODO: Assignment 1: read the vertical field-of-view from j ("vfov"),
    // and compute the width and height of the image plane. Remember that
    // the "vfov" parameter is specified in degrees, but C++ math functions
    // expect it in radians. You can use deg2rad() from common.h to convert
    // from one to the other
    put_your_code_here("Assignment 1: Compute the image plane size.");
    m_size = Vec2f(2.f, 1.f);
}

Ray3f Camera::generate_ray(const Vec2f &pixel) const
{
    ++num_camera_rays;
    // TODO: Assignment 1: Implement camera ray generation
    put_your_code_here("Assignment 1: Insert your camera ray generation code here");
    return Ray3f(Vec3f(0.f), Vec3f(1.f));
}