/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/image.h>
#include <darts/photon.h>
#include <darts/progress.h>
#include <darts/sampling.h>
#include <darts/test.h>
#include <filesystem/resolver.h>

#include <algorithm>

struct PhotonMapTest : public SampleTest
{
    PhotonMapTest(const json &j);

    bool  sample(Vec3f &pos, const Vec2f &rv, float rv1) override;
    Vec3f generate_photon(const Vec2f &rv) const;
    float pdf(const Vec3f &pos, float rv1) const override;
    float photon_density(const Vec3f &pos) const;

    Vec2f sample_to_pixel(const Vec3f &pos) const override;
    Vec3f pixel_to_sample(const Vec2f &pixel) const override;

    virtual void run() override;

    float     search_radius = .1f;
    Transform xform;
    int       search_count = 50;
    float     det          = 1.f;

    PhotonMap photon_map;
};

PhotonMapTest::PhotonMapTest(const json &j) : SampleTest(j)
{
    search_radius = j.value("search radius", search_radius);
    search_count  = j.value("search count", search_count);
    xform         = j.value("transform", xform);

    det = determinant(xform.m);

    photon_map.reserve(total_samples);
}

Vec2f PhotonMapTest::sample_to_pixel(const Vec3f &pos) const
{
    return pos.xy() * image_size;
}

Vec3f PhotonMapTest::pixel_to_sample(const Vec2f &pixel) const
{
    return Vec3f{pixel / image_size, 0.f};
}

Vec3f PhotonMapTest::generate_photon(const Vec2f &rv) const
{
    return xform.point(Vec3f{sample_disk(rv), 0.f});
}

bool PhotonMapTest::sample(Vec3f &dir, const Vec2f &rv, float rv1)
{
    dir = generate_photon(rv);
    return true;
}

float PhotonMapTest::pdf(const Vec3f &pos, float rv1) const
{
    return sample_disk_pdf(xform.inverse().point(pos).xy()) / det;
}

float PhotonMapTest::photon_density(const Vec3f &pos) const
{
    // this serves as an example of how to use the KNNSearch and FixedRadiusProcess
    Color3f result;
    float   search_area;
    if (search_count > 0)
    {
        // k-nearest neighbor search
        KNNSearch search(pos, search_radius * search_radius, search_count);
        photon_map.find(search);

        // A KNNSearch stores the k nearest neighbors. Here we just need iterate over all found photons and accumulate
        // their powers.
        for (auto photon : search.results)
            result += photon.photon->data.power();

        // KNNSearch::max_dist2 stores the squared distance to the k-th photon, which determines the disc over which we
        // compute density.
        search_area = float(M_PI * search.max_dist2);
    }
    else
    {
        // For a fixed-radius search, we don't store the found photons, but instead process them in-place as they are
        // found (using the passed in lambda function).
        auto accum_photon = [&result, &pos, this](const SearchResult &p) { result += p.photon->data.power(); };
        FixedRadiusProcess search(pos, search_radius * search_radius, accum_photon);
        photon_map.find(search);

        // in this case we divide by the area of the fixed-radius disc.
        search_area = M_PI * pow2(search_radius);
    }

    return luminance(result) / search_area;
}

void PhotonMapTest::run()
{
    // Step 1: Evaluate pdf over the sphere and compute its integral
    pcg32 rng;
    {
        Progress progress(fmt::format("Generating {} photons", total_samples), total_samples);
        // First insert all the photons into the photon map
        for (auto i : range(total_samples))
        {
            auto p = generate_photon({rng.nextFloat(), rng.nextFloat()});
            photon_map.insert(p, Photon(Vec3f{1.f}, Color3f{1.f / total_samples}));
            ++progress;
        }
        // Then build the photon map
        photon_map.build();
        progress.set_done();
    }

    double         integral = 0.0f;
    Array2d<float> pdf(image_size.x, image_size.y);
    {
        Progress progress("Evaluating analytic PDF", pdf.height());
        for (int y = 0; y < pdf.height(); ++y, ++progress)
            for (int x = 0; x < pdf.width(); x++)
            {
                float accum = 0.f;
                for (int sx = 0; sx < super_samples; ++sx)
                    for (int sy = 0; sy < super_samples; ++sy)
                        accum += this->pdf(
                            pixel_to_sample(Vec2f{x + (sx + 0.5f) / super_samples, y + (sy + 0.5f) / super_samples}),
                            rng.nextFloat());

                accum /= pow2(super_samples);

                integral += pdf(x, y) = accum;
            }
        integral /= product(image_size);
        progress.set_done();
    }

    // Step 2: Compute automatic exposure value as the 99.95th percentile instead of maximum for increased
    // robustness
    if (max_value < 0.f)
    {
        std::vector<float> values(&pdf(0, 0), &pdf(0, 0) + pdf.length());
        std::sort(values.begin(), values.end());
        max_value = values[int((pdf.length() - 1) * 0.9995)];
    }

    // Now upscale our histogram and pdf
    Array2d<float> pdf_upsampled = upsample(pdf, up_samples);

    // Generate heat maps
    // NOTE: we use get_file_resolver()[0] here to refer to the parent directory of the scene file.
    // This assumes that the calling code has prepended this directory to the front of the global resolver list
    generate_heatmap(pdf_upsampled, 1.f / max_value).save((get_file_resolver()[0] / (name + "-pdf.png")).str());
    generate_graymap(pdf_upsampled).save((get_file_resolver()[0] / (name + "-pdf.exr")).str());

    // Step 3: evaluate the photon density estimation result
    Array2d<float> density(image_size.x, image_size.y);
    double         density_integral = 0.0;
    {
        Progress progress("Computing photon density estimate", density.height());
        for (int y = 0; y < density.height(); ++y, ++progress)
            for (int x = 0; x < density.width(); x++)
                density_integral += density(x, y) = photon_density(pixel_to_sample({x + 0.5f, y + 0.5f}));
        density_integral /= product(image_size);
        progress.set_done();
    }

    // Now upscale our histogram and pdf
    Array2d<float> density_upsampled = upsample(density, up_samples);

    generate_heatmap(density_upsampled, 1.f / max_value)
        .save((get_file_resolver()[0] / (name + "-photon-density.png")).str());
    generate_graymap(density_upsampled).save((get_file_resolver()[0] / (name + "-photon-density.exr")).str());

    // Output statistics
    auto msg =
        fmt::format("Integrals of analytic PDF: {}, and photon density: {} should match.", integral, density_integral);
    if (fabs(integral - density_integral) > 0.01)
        throw DartsException(msg.c_str());
    spdlog::info(msg);
}

DARTS_REGISTER_CLASS_IN_FACTORY(Test, PhotonMapTest, "photon map")

/**
    \file
    \brief Class #PhotonMapTest
*/
