/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/camera.h>
#include <darts/common.h>
#include <darts/factory.h>
#include <darts/image.h>
#include <darts/material.h>
#include <darts/surface_group.h>

/**
    Main scene data structure.

    This class aggregates all the surfaces and materials along with the camera.
*/
class Scene : public Surface
{
public:
    static uint32_t random_seed;

    Scene() = default;
    /// Construct a new scene from a json object
    Scene(const json &j);

    /// Parse a scene from a json object
    void parse(const json &j);

    /// Release all memory
    virtual ~Scene();

    virtual void add_child(shared_ptr<Surface> surface) override
    {
        m_surfaces->add_child(surface);
    }

    bool intersect(const Ray3f &ray, HitInfo &hit) const override;

    Box3f bounds() const override
    {
        return m_surfaces->bounds();
    }

    /// Return the background color
    Color3f background(const Ray3f &ray) const;

    /// Return the camera
    shared_ptr<const Camera> camera() const
    {
        return m_camera;
    }

    /**
        Sample the color along a ray

        \param ray      The ray in question
        \param depth    The current recursion depth
        \return         An estimate of the color from this direction
    */
    Color3f recursive_color(const Ray3f &ray, int depth) const;

    /// Generate the entire image by ray tracing.
    Image3f raytrace() const;

private:
    shared_ptr<Camera>       m_camera;
    shared_ptr<SurfaceGroup> m_surfaces;
    Color3f m_background  = Color3f(0.2f);
    int     m_num_samples = 1;
};

/// create hard-coded test scenes that do not need to be loaded from a file
json create_example_scene(int scene_number);

/**
    \file
    \brief Class #Scene
*/