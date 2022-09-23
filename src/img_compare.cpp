/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <CLI/CLI.hpp>
#include <darts/common.h>
#include <darts/image.h>

/**
 * Compares a test image to a reference image, outputs the difference, and exits with failure or success depending on
 * whether the difference is above a threshold
 */
int main(int argc, char **argv)
{
    string outfile, test_filename, reference_filename;
    float  multiplier = 1.f;
    float  threshold  = 2 / 255.f;
    int    verbosity  = spdlog::get_level();

    CLI::App app{"\nCompares a test image to a reference image, outputs the difference, and exits with failure or "
                 "success depending on whether the difference is above a threshold.\n",
                 "img_compare"};

    app.get_formatter()->column_width(35);

    string save_formats = fmt::format("{}", fmt::join(Image3f::savable_formats(), ", "));

    app.add_option("-o,--outfile", outfile,
                   fmt::format("Specify the output image filename (extension must be one of: {})", save_formats));
    app.add_option("-m,--multiplier", multiplier, "The amount to multiply the difference image by.")
        ->check(CLI::PositiveNumber);
    app.add_option("-t,--threshold", threshold, "The rmse threshold above which the images are considered different.")
        ->check(CLI::PositiveNumber);
    app.add_option("test_img", test_filename, "The filename of a test image")->required()->check(CLI::ExistingFile);
    app.add_option("ref_img", reference_filename, "The filename of a reference image")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-v,--verbosity", verbosity,
                   R"(Set verbosity threshold T with lower values meaning more verbose
and higher values removing low-priority messages. All messages with
severity >= T are displayed, where the severities are:
    trace    = 0
    debug    = 1
    info     = 2
    warn     = 3
    err      = 4
    critical = 5
    off      = 6
The default is 2 (info).)")
        ->check(CLI::Range(0, 6));

    try
    {
        CLI11_PARSE(app, argc, argv);

        darts_init(verbosity);

        spdlog::info("Comparing\n   test image:      {}\n   reference image: {}", test_filename, reference_filename);

        Image3f test;
        if (!test.load(test_filename))
            throw DartsException("Cannot load test image!");
        Image3f reference;
        if (!reference.load(reference_filename))
            throw DartsException("Cannot load reference image!");

        if (test.width() != reference.width() || test.height() != reference.height())
            throw DartsException("Test image ({}x{}) and reference image ({}x{}) resolutions don't match!",
                                 test.width(), test.height(), reference.width(), reference.height());

        Image3f diff(test.width(), test.height());
        Color3f mad(0.f);
        for (auto y : range(test.height()))
            for (auto x : range(test.width()))
            {
                auto d = abs(test(x, y) - reference(x, y));
                mad += d;
                diff(x, y) = d * multiplier;
            }

        mad /= diff.size();

        float scalar_mad = sum(mad) / 3.f;

        spdlog::info("Mean Absolute Difference: {}", mad);
        spdlog::info("Average of MAD across color channels: {}", scalar_mad);

        if (!outfile.empty())
        {
            spdlog::info("Writing difference image to '{}'.", outfile);
            diff.save(outfile);
        }

        if (scalar_mad > threshold)
            throw DartsException("Images don't match!");
    }
    catch (const std::exception &e)
    {
        spdlog::error("{}", e.what());
        exit(EXIT_FAILURE);
    }

    spdlog::info("Images match!");
    exit(EXIT_SUCCESS);
}
