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

SampleTest::SampleTest(const json &j)
{
    name          = j.at("name");
    image_width   = j.value("image width", 256);
    image_height  = j.value("image height", 128);
    total_samples = j.value("spp", 1000) * image_width * image_height;
    super_samples = j.value("super samples", 32);
    up_samples    = j.value("up samples", 4);
}

Vec2i SampleTest::direction_to_pixel(const Vec3f &dir) const
{
    return Vec2i(Spherical::direction_to_spherical_coordinates(dir) *
                 Vec2f(image_width * INV_TWOPI, image_height * INV_PI));
}

Vec3f SampleTest::pixel_to_direction(const Vec2f &pixel) const
{
    return Spherical::spherical_coordinates_to_direction(pixel * Vec2f(2 * M_PI / image_width, M_PI / image_height));
}

Image3f SampleTest::generate_heatmap(const Array2d<float> &density, float max_value)
{
    Image3f result(density.width(), density.height());

    for (int y = 0; y < density.height(); ++y)
        for (int x = 0; x < density.width(); ++x)
            result(x, y) = inferno(density(x, y) / max_value);

    return result;
}

void SampleTest::run()
{
    fmt::print("---------------------------------------------------------------------------\n");
    fmt::print("Running sample test for \"{}\"\n", name);

    // Step 1: Evaluate pdf over the sphere and compute its integral
    pcg32          rng;
    double         integral = 0.0f;
    Array2d<float> pdf(image_width, image_height);
    Progress       progress1("Evaluating analytic PDF", pdf.height());
    for (int y = 0; y < pdf.height(); ++y, ++progress1)
        for (int x = 0; x < pdf.width(); x++)
        {
            float accum = 0.f;
            for (int sx = 0; sx < super_samples; ++sx)
                for (int sy = 0; sy < super_samples; ++sy)
                {
                    Vec3f dir =
                        pixel_to_direction(Vec2f(x + (sx + 0.5f) / super_samples, y + (sy + 0.5f) / super_samples));
                    float sin_theta = std::sqrt(max(1.0f - dir.z * dir.z, 0.0f));
                    float pixel_area =
                        (M_PI / image_width / super_samples) * (M_PI * 2.0f / image_height / super_samples) * sin_theta;
                    float value = this->pdf(dir, rng.nextFloat());
                    accum += value;
                    integral += pixel_area * value;
                }
            pdf(x, y) = accum / (super_samples * super_samples);
        }
    progress1.set_done();

    // Step 2: Generate histogram of samples
    Array2d<float> histogram(image_width, image_height);

    uint64_t valid_samples = 0;
    bool     nan_or_inf    = false;
    Progress progress(fmt::format("Generating {} samples", total_samples), total_samples);
    for (uint64_t i = 0; i < total_samples; ++i, ++progress)
    {
        Vec3f dir;
        if (!sample(dir, Vec2f(rng.nextFloat(), rng.nextFloat()), rng.nextFloat()))
            continue;

        if (std::isnan(dir.x + dir.y + dir.z) || std::isinf(dir.x + dir.y + dir.z))
        {
            nan_or_inf = true;
            continue;
        }

        // Map scattered direction to pixel in our sample histogram
        Vec2i pixel = direction_to_pixel(dir);
        if (pixel.x < 0 || pixel.y < 0 || pixel.x >= histogram.width() || pixel.y >= histogram.height())
            continue;

        // Incorporate Jacobian of spherical mapping and bin area into the sample weight
        float sin_theta = std::sqrt(max(1.0f - dir.z * dir.z, 0.0f));
        float weight    = (histogram.width() * histogram.height()) / (M_PI * (2.0f * M_PI) * total_samples * sin_theta);
        // Accumulate into histogram
        histogram(pixel.x, pixel.y) += weight;
        valid_samples++;
    }

    progress.set_done();

    // Now upscale our histogram and pdf
    Array2d<float> histo_upsampled(image_width * up_samples, image_height * up_samples);
    for (int y = 0; y < histo_upsampled.height(); ++y)
        for (int x = 0; x < histo_upsampled.width(); ++x)
            histo_upsampled(x, y) = histogram(x / up_samples, y / up_samples);

    Array2d<float> pdf_upsampled(image_width * up_samples, image_height * up_samples);
    for (int y = 0; y < pdf_upsampled.height(); ++y)
        for (int x = 0; x < pdf_upsampled.width(); ++x)
            pdf_upsampled(x, y) = pdf(x / up_samples, y / up_samples);

    // Step 3: For auto-exposure, compute 99.95th percentile instead of maximum for increased robustness
    std::vector<float> values(&pdf(0, 0), &pdf(0, 0) + pdf.size());
    std::sort(values.begin(), values.end());
    float max_value = values[int((pdf.size() - 1) * 0.9995)];
    for (int i = 0; i < pdf.size(); ++i)
        if (std::isnan(pdf(i)) || std::isinf(pdf(i)))
            nan_or_inf = true;

    // Generate heat maps
    // NOTE: we use get_file_resolver()[0] here to refer to the parent directory of the scene file.
    // This assumes that the calling code has prepended this directory to the front of the global resolver list
    generate_heatmap(pdf_upsampled, max_value).save((get_file_resolver()[0] / (name + "-pdf.png")).str());
    generate_heatmap(histo_upsampled, max_value).save((get_file_resolver()[0] / (name + "-sampled.png")).str());

    // Output statistics
    auto integral_msg = fmt::format("Integral of PDF (should be close to 1): {}", integral);
    if (integral > 1.01f || integral < 0.90f)
        throw DartsException(integral_msg.c_str());
    spdlog::info(integral_msg);

    uint64_t percent_valid = (valid_samples * 100) / total_samples;
    auto     percent_msg   = fmt::format("{}% of samples were valid (this should be close to 100%)", percent_valid);
    if (percent_valid > 100 || percent_valid < 90)
        throw DartsException(percent_msg.c_str());
    spdlog::info(percent_msg);

    if (nan_or_inf)
        throw DartsException("Some directions/PDFs contained invalid values (NaN or infinity). This should not happen. "
                             "Make sure you catch all corner cases in your code.");
    print_more_statistics();
}

/**
    \dir
    \brief Darts Test plugins source directory
*/