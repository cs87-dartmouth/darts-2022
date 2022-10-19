/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/image.h>
#include <darts/material.h>
#include <darts/surface.h>
#include <darts/test.h>

#include <algorithm>

struct MaterialScatterTest : public ScatterTest
{
    MaterialScatterTest(const json &j);

    bool sample(Vec3f &dir, const Vec2f &rv, float rv1) override;
    void print_more_statistics() override;

    shared_ptr<Material> material;
    Vec3f                normal;
    Ray3f                ray;
    HitInfo              hit;

    bool any_below_hemisphere = false;
};

MaterialScatterTest::MaterialScatterTest(const json &j) : ScatterTest(j)
{
    material = DartsFactory<Material>::create(j.at("material"));
    normal   = normalize(j.at("normal").get<Vec3f>());
    ray.d    = normalize(j.value("incoming", Vec3f(0.0f, 0.25f, -1.0f)));

    hit.t  = 1.0f;
    hit.p  = Vec3f(0.0f);
    hit.gn = hit.sn = normal;
    hit.uv          = Vec2f(0.5f);
}

bool MaterialScatterTest::sample(Vec3f &dir, const Vec2f &, float)
{
    // Sample material
    Color3f attenuation;
    Ray3f   out;
    if (!material->scatter(ray, hit, attenuation, out))
        return false;

    dir = normalize(out.d);

    // Sanity check to make sure directions are valid
    if (dot(dir, hit.sn) < -Ray3f::epsilon)
    {
        any_below_hemisphere = true;
        return false;
    }

    return true;
}

void MaterialScatterTest::print_more_statistics()
{
    if (any_below_hemisphere)
        throw DartsException("Some generated directions from `scatter` were below the hemisphere. For opaque materials "
                             "like Lambertian and Metal, this should never happen.");
}

DARTS_REGISTER_CLASS_IN_FACTORY(Test, MaterialScatterTest, "scatter material")

/**
    \file
    \brief Class #MaterialScatterTest
*/
