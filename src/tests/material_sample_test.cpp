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

struct MaterialSampleTest : public SampleTest
{
    MaterialSampleTest(const json &j);

    bool  sample(Vec3f &dir, const Vec2f &rv, float rv1) override;
    float pdf(Vec3f &dir, float rv1) override;
    void  print_more_statistics() override;

    shared_ptr<Material> material;
    Vec3f                normal;
    Vec3f                incoming;
    HitInfo              hit;

    bool any_specular         = false;
    bool any_below_hemisphere = false;
};

MaterialSampleTest::MaterialSampleTest(const json &j) : SampleTest(j)
{
    material = DartsFactory<Material>::create(j.at("material"));
    normal   = normalize(j.at("normal").get<Vec3f>());
    incoming = normalize(j.value("incoming", Vec3f(0.25f, 0.0f, -1.0f)));

    hit.t  = 1.0f;
    hit.p  = Vec3f(0.0f);
    hit.gn = hit.sn = normal;
    hit.uv          = Vec2f(0.5f);
}

bool MaterialSampleTest::sample(Vec3f &dir, const Vec2f &rv, float rv1)
{
    // Sample material
    ScatterRecord record;
    if (!material->sample(incoming, hit, record, rv, rv1))
        return false;

    dir = record.wo;

    if (record.is_specular)
        any_specular = true;

    // Sanity check to make sure directions are valid
    Vec3f wo = normalize(record.wo);
    if (dot(wo, hit.sn) < -Ray3f::epsilon)
    {
        any_below_hemisphere = true;
        return false;
    }

    return true;
}

float MaterialSampleTest::pdf(Vec3f &dir, float rv1)
{
    return material->pdf(incoming, dir, hit);
}
void MaterialSampleTest::print_more_statistics()
{
    if (any_specular)
        throw DartsException("is_specular is set. It should not be.");
    if (any_below_hemisphere)
        throw DartsException("Some generated directions were below the hemisphere. You should check for this case and "
                             "return false from sample instead.");
}

DARTS_REGISTER_CLASS_IN_FACTORY(Test, MaterialSampleTest, "sample material")

/**
    \file
    \brief Class #MaterialSampleTest
*/
