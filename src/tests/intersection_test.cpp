/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/surface.h>
#include <darts/surface_group.h>
#include <darts/test.h>

namespace
{

template <typename T>
float max_abs_error(const T &a, const T &b)
{
    return maxelem(abs(a - b));
}

template <>
float max_abs_error(const float &a, const float &b)
{
    return std::abs(a - b);
}

} // namespace

/// A class to test the results of a ray-surface intersection
struct IntersectionTest : public Test
{
    IntersectionTest(const json &j);

    virtual void run() override;
    virtual void print_header() const override;

    shared_ptr<Surface> surface;
    Ray3f               ray;
    float               correct_t;
    Vec3f               correct_p;
    Vec3f               correct_gn;
    Vec3f               correct_sn;
    bool                should_hit = true;
    float               threshold  = 1e-5f;
};

IntersectionTest::IntersectionTest(const json &j)
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
        throw DartsException("Invalid intersection test file. No 'surface' or 'surfaces' field found.");

    ray        = Ray3f(j.at("ray").at("origin").get<Vec3f>(), j.at("ray").at("direction").get<Vec3f>());
    correct_t  = j.at("t").get<float>();
    correct_p  = j.at("p").get<Vec3f>();
    correct_gn = j.at("gn").get<Vec3f>();
    correct_sn = j.at("sn").get<Vec3f>();
    should_hit = j.value("should_hit", should_hit);
    threshold  = j.value("threshold", threshold);
}

void IntersectionTest::print_header() const
{
    fmt::print("---------------------------------------------------------------------------\n");
    fmt::print("Testing surface intersection...\n");
}

void IntersectionTest::run()
{
    HitInfo hit;
    if (surface->intersect(ray, hit))
    {
        if (!should_hit)
            throw DartsException("Intersection incorrect! Ray should not hit the surface.");

        if (max_abs_error(correct_t, hit.t) > threshold)
            throw DartsException("Intersection distance incorrect!\nShould be {}\n but got {}", correct_t, hit.t);

        if (max_abs_error(correct_p, hit.p) > threshold)
            throw DartsException("Intersection point incorrect!\nShould be {}\n but got {}", correct_p, hit.p);

        if (max_abs_error(correct_gn, hit.gn) > threshold)
            throw DartsException("Geometric normal incorrect!\nShould be {}\n but got {}", correct_gn, hit.gn);

        if (max_abs_error(correct_sn, hit.sn) > threshold)
            throw DartsException("Shading normal incorrect!\nShould be {}\n but got {}", correct_sn, hit.sn);
    }
    else
    {
        if (should_hit)
            throw DartsException("Intersection incorrect! Ray should hit surface.");
    }
}

DARTS_REGISTER_CLASS_IN_FACTORY(Test, IntersectionTest, "intersection")

/**
    \file
    \brief Class #IntersectionTest
*/
