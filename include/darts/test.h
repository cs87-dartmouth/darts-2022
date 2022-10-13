/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/array2d.h>
#include <darts/common.h>
#include <darts/fwd.h>

/// Base class for unit tests in Darts
struct Test
{
    /**
        Run the actual test.

        This is where all the work happens. Derived classes should override this function.
    */
    virtual void run() = 0;
};

/// Check if this json object contains tests, and run them
void run_tests(const json &j);

struct SampleTest : public Test
{
    SampleTest(const json &j);

    Vec2i          direction_to_pixel(const Vec3f &dir) const;
    Vec3f          pixel_to_direction(const Vec2f &pixel) const;
    static Image3f generate_heatmap(const Array2d<float> &density, float max_value);

    virtual void  run() override;
    virtual bool  sample(Vec3f &dir, const Vec2f &rv, float rv1) = 0;
    virtual float pdf(Vec3f &dir, float rv1)                     = 0;
    virtual void  print_more_statistics()
    {
    }

    string   name;
    uint32_t image_width;
    uint32_t image_height;
    uint64_t total_samples;
    uint32_t super_samples;
    uint32_t up_samples;
};

/**
    \file
    \brief Class #Test.
*/