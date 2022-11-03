/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/medium.h>
#include <darts/sampler.h>
#include <darts/scene.h>
#include <filesystem/resolver.h>
#include <limits>
#include <nanovdb/util/GridStats.h>
#include <nanovdb/util/IO.h>
#include <nanovdb/util/Ray.h>
#include <nanovdb/util/SampleFromVoxels.h>

/// A medium with density defined by a NanoVDB grid.
/// \ingroup Media
class NanoVDBMedium : public Medium
{
public:
    NanoVDBMedium(const json &j);

    std::tuple<Color3f, Color3f, Color3f> coeffs(const Vec3f &p) const override;

    bool    sample_free_flight(const Ray3f &ray, int channel, Sampler &sampler, HitInfo &hit, Color3f &f,
                               Color3f &p) const override;
    Color3f total_transmittance(const Ray3f &ray, Sampler &sampler) const override;

protected:
    Color3f                                  m_sigma_s{0.8f};       ///< scattering coefficient
    Color3f                                  m_sigma_a{0.2f};       ///< absorption coefficient
    Color3f                                  m_total;               ///< total coefficient (absorption+scattering+null)
    Transform                                m_xform = Transform(); ///< Transformation to place the grid in the scene
    Box3f                                    m_bbox;                ///< The bounds, local space
    nanovdb::GridHandle<nanovdb::HostBuffer> m_density_handle;
    const nanovdb::FloatGrid                *m_density_grid = nullptr;

    using Lookup = nanovdb::SampleFromVoxels<nanovdb::FloatGrid::TreeType, 1, false>;
};

NanoVDBMedium::NanoVDBMedium(const json &j) : Medium(j)
{
    std::string filename, path;
    try
    {
        filename = j.at("filename").get<string>();
        path     = get_file_resolver().resolve(filename).str();
    }
    catch (...)
    {
        throw DartsException("No \"filename\" specified for vdb medium.", j.dump());
    }

    // load and parse the nanovdb file
    try
    {
        std::string gridname = j.value("gridname", "density");
        spdlog::info("Reading \"{}\" grid from NanoVDB file \"{}\"", gridname, path);
        m_density_handle = nanovdb::io::readGrid(path, gridname);

        if (m_density_handle)
        {
            spdlog::info("Loaded NanoVDB file \"{}\"", filename);
            if (!m_density_handle.gridMetaData()->isFogVolume() && !m_density_handle.gridMetaData()->isUnknown())
                throw DartsException("not a FogVolume", filename);
        }
        else
            throw DartsException("Couldn't find \"{}\" grid in vdb file", gridname);

        m_density_grid = m_density_handle.grid<float>();
    }
    catch (const std::exception &e)
    {
        throw DartsException("nanovdb: {}: \"{}\".", path, e.what());
    }

    m_xform   = j.value("transform", m_xform);
    m_sigma_a = j.value("sigma_a", m_sigma_a);
    m_sigma_s = j.value("sigma_s", m_sigma_s);

    // auto bbox = m_density_grid->worldBBox();
    // m_bbox.enclose(Vec3f{bbox.min()[0], bbox.min()[1], bbox.min()[2]});
    // m_bbox.enclose(Vec3f{bbox.max()[0], bbox.max()[1], bbox.max()[2]});

    Vec3f min_bounds = Vec3f(m_density_grid->worldBBox().min()[0], m_density_grid->worldBBox().min()[1],
                             m_density_grid->worldBBox().min()[2]);
    Vec3f max_bounds = Vec3f(m_density_grid->worldBBox().max()[0], m_density_grid->worldBBox().max()[1],
                             m_density_grid->worldBBox().max()[2]);
    m_bbox.enclose(min_bounds);
    m_bbox.enclose(max_bounds);

    float min, max;
    m_density_grid->tree().extrema(min, max);
    m_total = Vec3f(max * (m_sigma_s + m_sigma_a));

    spdlog::info(
        R"(NanoVDBMedium info:
    filename                    : {}
    sigma_a                     : {}
    sigma_s                     : {}
    total (majorant) coefficient: {}
    # voxels                    : {}
    data type                   : {}
    min density                 : {}
    max density                 : {}
    bbox min                    : {}
    bbox max                    : {}
    xform : {})",
        filename, m_sigma_a, m_sigma_s, m_total, m_density_handle.gridMetaData()->activeVoxelCount(),
        toStr(m_density_handle.gridMetaData()->gridType()), min, max, m_bbox.min, m_bbox.max,
        indent(fmt::format("{}", m_xform.m), string("    xform : ").length()));
}

std::tuple<Color3f, Color3f, Color3f> NanoVDBMedium::coeffs(const Vec3f &p_) const
{
    Vec3f          p       = m_xform.inverse().point(p_);
    nanovdb::Vec3f p_index = m_density_grid->worldToIndexF(nanovdb::Vec3f(p.x, p.y, p.z));
    Lookup         sampler = Lookup(m_density_grid->tree());
    float          d       = sampler(p_index);

    return std::make_tuple(d * m_sigma_a, d * m_sigma_s, m_total - (m_sigma_a + m_sigma_s) * d);
}

bool NanoVDBMedium::sample_free_flight(const Ray3f &ray, int channel, Sampler &sampler, HitInfo &hit, Color3f &f,
                                       Color3f &p) const
{
    // transform ray into volume's space
    Ray3f r = m_xform.inverse().ray(ray);

    // convert to a NanoVDB ray, and check intersection
    auto world_ray =
        nanovdb::Ray<float>(nanovdb::Vec3f(r.o.x, r.o.y, r.o.z), nanovdb::Vec3f(r.d.x, r.d.y, r.d.z), r.mint, r.maxt);

    // clip grid-space ray to bounding box
    if (!world_ray.clip(m_density_grid->worldBBox()))
    {
        hit.t = ray.maxt;
        hit.p = ray(hit.t);
        return false; // ray does not hit any of the medium
    }
    else
    {
        // TODO: implement this function if you want to support nanovdb media. Since we are using the null-scattering
        // framework, this can be quite similar to what you would do in HomogeneousMedium::sample_free_flight()
        return false;
    }
}

Color3f NanoVDBMedium::total_transmittance(const Ray3f &ray, Sampler &sampler) const
{
    // TODO: implement this function if you want to support nanovdb media. This can be identical to
    // HomogeneousMedium::total_transmittance()
    return Color3f{1.f};
}

DARTS_REGISTER_CLASS_IN_FACTORY(Medium, NanoVDBMedium, "nanovdb")

/**
    \file
    \brief NanoVDBMedium Medium
*/