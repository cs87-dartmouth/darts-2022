/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/parallel.h>
#include <darts/progress.h>
#include <darts/sampling.h>
#include <darts/test.h>
#include <filesystem/resolver.h>

void run_tests(const json &j)
{
    // check if this is a scene to render, or a test to execute
    if (j.contains("type") && j["type"] == "tests")
    {
        int count      = j["tests"].size();
        int num_passed = 0;
        for (auto &t : j["tests"])
        {
            try
            {
                auto test = DartsFactory<Test>::create(t);
                test->print_header();
                test->run();
                num_passed++;
            }
            catch (const std::exception &e)
            {
                spdlog::error("Test failed: {}", e.what());
            }
        }
        if (num_passed == count)
        {
            spdlog::info("Passed all {}/{} tests. Also examine the generated images.", num_passed, count);
            exit(EXIT_SUCCESS);
        }
        else
        {
            spdlog::error("Failed {}/{} tests. Also examine the generated images.", count - num_passed, count);
            exit(EXIT_FAILURE);
        }
    }
}


ScatterTest::ScatterTest(const json &j)
{
    name          = j.at("name");
    image_size    = j.value("image size", image_size);
    total_samples = j.value("spp", 1000) * product(image_size);
    up_samples    = j.value("up samples", up_samples);
}

void ScatterTest::print_header() const
{
    fmt::print("---------------------------------------------------------------------------\n");
    fmt::print("Running test for \"{}\"\n", name);
}

Vec2f ScatterTest::sample_to_pixel(const Vec3f &dir) const
{
    return Spherical::direction_to_spherical_coordinates(dir) * image_size * Vec2f{INV_TWOPI, INV_PI};
}

Vec3f ScatterTest::pixel_to_sample(const Vec2f &pixel) const
{
    return Spherical::spherical_coordinates_to_direction(pixel * Vec2f{2 * M_PI, M_PI} / image_size);
}

Image3f ScatterTest::generate_heatmap(const Array2d<float> &density, float scale)
{
    Image3f result(density.width(), density.height());

    for (int y = 0; y < density.height(); ++y)
        for (int x = 0; x < density.width(); ++x)
            result(x, y) = inferno(density(x, y) * scale);

    return result;
}

Image3f ScatterTest::generate_graymap(const Array2d<float> &density, float scale)
{
    Image3f result(density.width(), density.height());

    for (int y = 0; y < density.height(); ++y)
        for (int x = 0; x < density.width(); ++x)
            result(x, y) = Color3f{density(x, y) * scale};

    return result;
}

Array2d<float> ScatterTest::upsample(const Array2d<float> &img, int factor)
{
    Array2d<float> upsampled(img.width() * factor, img.height() * factor);
    for (int y = 0; y < upsampled.height(); ++y)
        for (int x = 0; x < upsampled.width(); ++x)
            upsampled(x, y) = img(x / factor, y / factor);
    return upsampled;
}

void ScatterTest::run()
{
    // Step 1: Generate histogram of samples
    Array2d<float> histogram(image_size.x, image_size.y);

    // populate the histogram
    bool     nan_or_inf    = false;
    uint64_t valid_samples = 0;
    pcg32    rng;
    Progress progress(fmt::format("Generating {} samples", total_samples), total_samples);
    for (uint64_t i = 0; i < total_samples; ++i, ++progress)
    {
        Vec3f dir;
        if (!sample(dir, Vec2f{rng.nextFloat(), rng.nextFloat()}, rng.nextFloat()))
            continue;

        dir = normalize(dir);

        if (!la::any(la::isfinite(dir)))
        {
            nan_or_inf = true;
            continue;
        }

        // Map scattered direction to pixel in our sample histogram
        Vec2i pixel{sample_to_pixel(dir)};
        if (pixel.x < 0 || pixel.y < 0 || pixel.x >= histogram.width() || pixel.y >= histogram.height())
            continue;

        // Incorporate Jacobian of spherical mapping and bin area into the sample weight
        float sin_theta = std::max(1e-8f, std::sqrt(max(1.0f - dir.z * dir.z, 0.0f)));
        float weight    = (histogram.length()) / (M_PI * (2.0f * M_PI) * total_samples * sin_theta);
        // Accumulate into histogram
        float val = histogram(pixel.x, pixel.y) + weight;
        if (!std::isfinite(val))
        {
            spdlog::error("Caught a NaN or Inf: {}; {}; {}; {}; {}", val, weight, pixel, dir, sin_theta);
            nan_or_inf = true;
            continue;
        }

        histogram(pixel.x, pixel.y) = val;
        valid_samples++;
    }
    progress.set_done();

    // Step 2: Compute automatic exposure value as the 99.95th percentile instead of maximum for increased robustness
    if (max_value < 0.f)
    {
        std::vector<float> values(&histogram(0, 0), &histogram(0, 0) + histogram.length());
        std::sort(values.begin(), values.end());
        max_value = values[int((histogram.length() - 1) * 0.9995)];
    }

    // Now upscale our histogram and pdf
    Array2d<float> histo_upsampled = upsample(histogram, up_samples);

    // Generate heat maps
    // NOTE: we use get_file_resolver()[0] here to refer to the parent directory of the scene file.
    // This assumes that the calling code has prepended this directory to the front of the global resolver list
    generate_heatmap(histo_upsampled, 1.f / max_value).save((get_file_resolver()[0] / (name + "-sampled.png")).str());
    generate_graymap(histo_upsampled).save((get_file_resolver()[0] / (name + "-sampled.exr")).str());

    uint64_t percent_valid = (valid_samples * 100) / total_samples;
    auto     percent_msg =
        fmt::format("{}% of the scattered directions were valid (this should be close to 100%)", percent_valid);
    if (percent_valid > 100 || percent_valid < 90)
        throw DartsException(percent_msg.c_str());
    spdlog::info(percent_msg);

    if (nan_or_inf)
        throw DartsException("Some directions/PDFs contained invalid values (NaN or infinity). This should not happen. "
                             "Make sure you catch all corner cases in your code.");
    print_more_statistics();
}

SampleTest::SampleTest(const json &j) : ScatterTest(j)
{
    super_samples = j.value("super samples", 32);
}

void SampleTest::run()
{
    // Step 1: Evaluate pdf over the sphere and compute its integral
    pcg32          rng;
    double         integral = 0.0f;
    Array2d<float> pdf(image_size.x, image_size.y);
    Progress       progress1("Evaluating analytic PDF", pdf.height());
    for (int y = 0; y < pdf.height(); ++y, ++progress1)
        for (int x = 0; x < pdf.width(); x++)
        {
            float accum = 0.f;
            for (int sx = 0; sx < super_samples; ++sx)
                for (int sy = 0; sy < super_samples; ++sy)
                {
                    Vec3f dir =
                        pixel_to_sample(Vec2f{x + (sx + 0.5f) / super_samples, y + (sy + 0.5f) / super_samples});
                    float sin_theta = std::sqrt(max(1.0f - dir.z * dir.z, 0.0f));
                    float pixel_area =
                        (M_PI / super_samples) * (M_PI * 2.0f / super_samples) * sin_theta / product(image_size);
                    float value = this->pdf(dir, rng.nextFloat());
                    accum += value;
                    integral += pixel_area * value;
                }
            pdf(x, y) = accum / (super_samples * super_samples);
        }
    progress1.set_done();

    // Step 2: Compute automatic exposure value as the 99.95th percentile instead of maximum for increased robustness
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

    // Output statistics
    auto integral_msg = fmt::format("Integral of PDF (should be close to 1): {}", integral);
    if (integral > 1.01f || integral < 0.90f)
        throw DartsException(integral_msg.c_str());
    spdlog::info(integral_msg);

    // Step 3: build the histogram
    ScatterTest::run();
}

/**
    \dir
    \brief Darts Test plugins source directory
*/