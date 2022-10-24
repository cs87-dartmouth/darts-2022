/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/image.h>
#include <darts/material.h>
#include <darts/scene.h>
#include <darts/surface.h>
#include <darts/surface_group.h>
#include <darts/test.h>

#include <algorithm>

struct SurfaceSampleTest : public SampleTest
{
    SurfaceSampleTest(const json &j);

    bool  sample(Vec3f &dir, const Vec2f &rv, float rv1) override;
    float pdf(Vec3f &dir, float rv1) override;

    shared_ptr<Surface> surface;
    Vec3f               normal;
};

SurfaceSampleTest::SurfaceSampleTest(const json &j) : SampleTest(j)
{
    if (j.contains("surface"))
        surface = DartsFactory<Surface>::create(j.at("surface"));
    else if (j.contains("surfaces"))
    {
        json j2    = j["surfaces"];
        auto group = DartsFactory<Surface>::create(j2);
        group->build();
        surface = group;
    }
    else
        throw DartsException("Invalid sample surface file. No 'surface' or 'surfaces' field found.");
}

bool SurfaceSampleTest::sample(Vec3f &dir, const Vec2f &rv, float rv1)
{
    // Sample geometry
    EmitterRecord rec;
    rec.o = Vec3f(0.f);
    surface->sample(rec, rv);
    dir = normalize(rec.wi);

    return true;
}

float SurfaceSampleTest::pdf(Vec3f &dir, float rv1)
{
    auto [child, prob] = surface->sample_child(rv1);
    return child->pdf(Vec3f(0.f), dir);
}

DARTS_REGISTER_CLASS_IN_FACTORY(Test, SurfaceSampleTest, "sample surface")

/**
    \file
    \brief Class #SurfaceSampleTest
*/
