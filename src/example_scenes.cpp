/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/scene.h>

json create_sphere_scene()
{
    std::string test = R"(
    {
        "camera":
        {
            "transform": { "o": [0,0,4] },
            "resolution": [ 512, 512 ],
            "vfov": 45
        },
        "surfaces": [
            {
                "type": "sphere",
                "material": { "type": "lambertian", "albedo": [0.6,0.4,0.4] }
            }
        ],
        "sampler": {"samples": 1},
        "background": [1, 1, 1]
    }
    )";
    return json::parse(test);
}

json create_sphere_plane_scene()
{
    std::string test = R"(
    {
        "camera":
        {
            "transform": { "o": [0,0,4] },
            "resolution": [ 512, 512 ],
            "vfov": 45
        },
        "surfaces": [
            {
                "type": "sphere",
                "radius": 1,
                "material": { "type": "lambertian", "albedo": [0.6,0.4,0.4] }
            },
            {
                "type": "quad",
                "transform": { "o": [0,-1,0], "x": [1,0,0], "y": [0,0,-1], "z": [0,1,0] },
                "size": [ 100, 100 ],
                "material": { "type": "lambertian", "albedo": [1,1,1] }
            }
        ],
        "sampler": {"samples": 1},
        "background": [1, 1, 1]
    }
    )";
    return json::parse(test);
}

json create_steinbach_scene()
{
    json j;

    // Compose the camera
    j["camera"] = {{"transform", {{"from", {-10.0, 10.0, 40.0}}, {"to", {0.0, -1.0, 0.0}}, {"up", {0.0, 1.0, 0.0}}}},
                   {"vfov", 18},
                   {"resolution", {512, 512}}};

    // compose the image properties
    j["sampler"]["samples"] = 1;
    j["background"]         = {1, 1, 1};

    Vec3f object_center(0.0f, 0.0f, 0.0f);
    float radius = 0.5f;
    int   num_s  = 40;
    int   num_t  = 40;
    for (auto is : range(num_s))
    {
        for (auto it : range(num_t))
        {
            float   s = (is + 0.5f) / num_s;
            float   t = (it + 0.5f) / num_t;
            float   u = s * (8) - 4.0f;
            float   v = t * (6.25f);
            Vec3f   center(-u * cos(v), v * cos(u) * 0.75f, u * sin(v));
            Color3f kd = 0.35f * lerp(lerp(Color3f(0.9f, 0.0f, 0.0f), Color3f(0.0f, 0.9f, 0.0f), t),
                                      lerp(Color3f(0.0f, 0.0f, 0.9f), Color3f(0.0f, 0.0f, 0.0f), t), s);

            j["surfaces"] += {{"type", "sphere"},
                              {"radius", radius},
                              {"transform",
                               {{"o", object_center + center},
                                {"x", {1.0, 0.0, 0.0}},
                                {"y", {0.0, 1.0, 0.0}},
                                {"z", {0.0, 0.0, 1.0}}}},
                              {"material", {{"type", "lambertian"}, {"albedo", kd}}}};
        }
    }

    j["surfaces"] +=
        {{"type", "quad"},
         {"size", {100, 100}},
         {"transform",
          {{"o", {0.0, -5.0, 0.0}}, {"x", {1.0, 0.0, 0.0}}, {"y", {0.0, 0.0, -1.0}}, {"z", {0.0, 1.0, 0.0}}}},
         {"material", {{"type", "lambertian"}, {"albedo", 1.0}}}};

    return j;
}

json create_shirley_scene()
{
    pcg32 rng = pcg32();

    json j;

    // Compose the camera
    j["camera"] = {{"transform", {{"from", {13, 2, 3}}, {"to", {0, 0, 0}}, {"up", {0, 1, 0}}}},
                   {"vfov", 20},
                   {"fdist", 10},
                   {"aperture", 0.1},
                   {"resolution", {600, 400}}};

    // compose the image properties
    j["sampler"]["samples"] = 1;
    j["background"]         = {1, 1, 1};

    // ground plane
    j["surfaces"] +=
        {{"type", "quad"},
         {"size", {100, 100}},
         {"transform",
          {{"o", {0.0, 0.0, 0.0}}, {"x", {1.0, 0.0, 0.0}}, {"y", {0.0, 0.0, -1.0}}, {"z", {0.0, 1.0, 0.0}}}},
         {"material", {{"type", "lambertian"}, {"albedo", {0.5, 0.5, 0.5}}}}};

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            float choose_mat = rng.nextFloat();
            float r1         = rng.nextFloat();
            float r2         = rng.nextFloat();
            Vec3f center(a + 0.9f * r1, 0.2f, b + 0.9f * r2);
            if (length(center - Vec3f(4.0f, 0.2f, 0.0f)) > 0.9f)
            {
                json sphere = {{"type", "sphere"}, {"radius", 0.2f}, {"transform", {{"translate", center}}}};

                if (choose_mat < 0.8)
                { // diffuse
                    float   r1 = rng.nextFloat();
                    float   r2 = rng.nextFloat();
                    float   r3 = rng.nextFloat();
                    float   r4 = rng.nextFloat();
                    float   r5 = rng.nextFloat();
                    float   r6 = rng.nextFloat();
                    Color3f albedo(r1 * r2, r3 * r4, r5 * r6);
                    sphere["material"] = {{"type", "lambertian"}, {"albedo", albedo}};
                }
                else if (choose_mat < 0.95)
                { // metal
                    float   r1 = rng.nextFloat();
                    float   r2 = rng.nextFloat();
                    float   r3 = rng.nextFloat();
                    float   r4 = rng.nextFloat();
                    Color3f albedo(0.5f * (1 + r1), 0.5f * (1.0f + r2), 0.5f * (1.0f + r3));
                    float   rough      = 0.5f * r4;
                    sphere["material"] = {{"type", "metal"}, {"albedo", albedo}, {"roughness", rough}};
                }
                else
                { // glass
                    sphere["material"] = {{"type", "dielectric"}, {"ior", 1.5}};
                }

                j["surfaces"] += sphere;
            }
        }
    }

    j["surfaces"] += {{"type", "sphere"},
                      {"radius", 1.f},
                      {"transform", {{"translate", {0, 1, 0}}}},
                      {"material", {{"type", "dielectric"}, {"ior", 1.5}}}};
    j["surfaces"] += {{"type", "sphere"},
                      {"radius", 1.f},
                      {"transform", {{"translate", {-4, 1, 0}}}},
                      {"material", {{"type", "lambertian"}, {"albedo", {0.4, 0.2, 0.1}}}}};
    j["surfaces"] += {{"type", "sphere"},
                      {"radius", 1.f},
                      {"transform", {{"translate", {4, 1, 0}}}},
                      {"material", {{"type", "metal"}, {"albedo", {0.7, 0.6, 0.5}}, {"roughness", 0.0}}}};

    return j;
}

json create_example_scene(int scene_number)
{
    switch (scene_number)
    {
    case 0: return create_sphere_scene();
    case 1: return create_sphere_plane_scene();
    case 2: return create_steinbach_scene();
    case 3: return create_shirley_scene();
    default: throw DartsException("Invalid hardcoded scene number {}. Must be 0..3.", scene_number);
    }
}
