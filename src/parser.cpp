/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#include <darts/factory.h>
#include <darts/scene.h>
#include <darts/sphere.h>
#include <darts/stats.h>

// anonymous namespace for variables/functions local to this file
namespace
{

// helper function to check for a required string key within a json object
string check_key(const string &key, const string &parent, const json &j)
{
    try
    {
        return j.at(key).get<string>();
    }
    catch (...)
    {
        throw DartsException("Missing '{}' on '{}' specification:\n{}", key, parent, j.dump(4));
    }
}

} // namespace

STAT_COUNTER("Scene/Materials", num_materials_created);
STAT_COUNTER("Scene/Surfaces", num_surfaces_created);

void Scene::parse(const json &j)
{
    spdlog::info("Parsing scene ...");


    //
    // check for and parse the camera specification
    //
    if (j.contains("camera"))
        m_camera = make_shared<Camera>(j["camera"]);
    else
        throw DartsException("No camera specified in scene!");

    //
    // read number of samples to take per pixel
    //
    if (j.contains("sampler") && j["sampler"].contains("samples"))
        m_num_samples = j["sampler"]["samples"];

    //
    // create the scene-wide acceleration structure so we can put other surfaces into it
    //
    if (j.contains("accelerator"))
        m_surfaces = DartsFactory<SurfaceGroup>::create(j["accelerator"]);
    else
        // default to a naive linear accelerator
        m_surfaces = make_shared<SurfaceGroup>(json::object());

    //
    // parse scene background
    //
    if (j.contains("background"))
    {
        m_background  = j["background"].get<Color3f>();
    }

    //
    // parse materials
    //
    if (j.contains("materials"))
    {
        for (auto &m : j["materials"])
        {
            auto material = DartsFactory<Material>::create(m);
            DartsFactory<Material>::register_instance(check_key("name", "material", m), material);
            spdlog::info("registering material with name {}", check_key("name", "material", m));
            ++num_materials_created;
        }
    }

    //
    // parse surfaces
    //
    if (j.contains("surfaces"))
    {
        for (auto &s : j["surfaces"])
        {
            auto surface = DartsFactory<Surface>::create(s);
            surface->add_to_parent(this, surface, j);
            surface->build(); // in case this top-level surface is a group, build it now
            ++num_surfaces_created;
        }
    }

    // set of all fields we'd expect to see at the top level of a darts scene
    // some of these are not yet supported, but we include them to be future-proof
    set<string> toplevel_fields{"integrator",  "media",  "materials", "surfaces",
                                "accelerator", "camera", "sampler",   "background"};

    // now loop through all keys in the json file to see if there are any that we don't recognize
    for (auto it = j.begin(); it != j.end(); ++it)
        if (toplevel_fields.count(it.key()) == 0)
            throw DartsException("Unsupported field '{}' here:\n{}", it.key(), it.value().dump(4));

    m_surfaces->build();
    spdlog::info("done parsing scene.");
}
