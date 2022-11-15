/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/array2d.h>
#include <darts/common.h>
#include <darts/fwd.h>
#include <darts/image.h>

/// Base class for unit tests in Darts
struct Test
{
    /**
        Run the actual test.

        This is where all the work happens. Derived classes should override this function.
    */
    virtual void run()                = 0;
    virtual void print_header() const = 0;
};

/// Check if this json object contains tests, and run them
void run_tests(const json &j);

struct ScatterTest : public Test
{
    ScatterTest(const json &j);

    virtual void run() override;
    virtual bool sample(Vec3f &dir, const Vec2f &rv, float rv1) = 0;
    virtual void print_header() const override;
    virtual void print_more_statistics()
    {
    }

    virtual Vec2f sample_to_pixel(const Vec3f &dir) const;
    virtual Vec3f pixel_to_sample(const Vec2f &pixel) const;

    static Image3f        generate_heatmap(const Array2d<float> &density, float scale = 1.f);
    static Image3f        generate_graymap(const Array2d<float> &density, float scale = 1.f);
    static Array2d<float> upsample(const Array2d<float> &img, int factor);

    string   name;
    Vec2i    image_size{256, 128};
    uint64_t total_samples;
    uint32_t up_samples = 4;
    float    max_value  = -1.f;
};

struct SampleTest : public ScatterTest
{
    SampleTest(const json &j);

    virtual void  run() override;
    virtual float pdf(const Vec3f &dir, float rv1) const = 0;

    uint32_t super_samples;
};

/**
    \file
    \brief Class #Test.
*/