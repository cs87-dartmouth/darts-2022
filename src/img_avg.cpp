/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

/**
    \file
    \brief Utility program to average two or more images
*/

#include <CLI/CLI.hpp>
#include <darts/common.h>
#include <darts/image.h>

/**
    Average a sequence of images
 */
int main(int argc, char **argv)
{
    string         outfile;
    vector<string> infiles;
    int            verbosity = spdlog::get_level();

    CLI::App app{"\nAverage a sequence of images and save the result to a new file."};

    app.get_formatter()->column_width(35);

    string save_formats = fmt::format("{}", fmt::join(Image3f::savable_formats(), ", "));

    app.add_option("-o,--outfile", outfile,
                   fmt::format("Specify the output image filename (extension must be one of: {})", save_formats));
    app.add_option("infiles", infiles, "The files to read in and average.")->required()->check(CLI::ExistingFile);
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

        spdlog::info("Averaging {} images.", infiles.size());

        Image3f average;
        if (!average.load(infiles[0]))
            throw DartsException("Cannot load image \"{}\".", infiles[0]);

        average.reset(Color3f(0.f));

        for (int i = 0; i < infiles.size(); ++i)
        {
            Image3f image;
            if (!image.load(infiles[i]))
                throw DartsException("Cannot load image {}: \"{}\".", i, infiles[i]);

            if (image.width() != average.width() || image.height() != average.height())
                throw DartsException("Image dimensions don't match. \"{}\" : ({}x{}) vs. \"{}\" ({}x{}).", infiles[0],
                                     average.width(), average.height(), infiles[i], image.width(), image.height());

            for (auto y : range(average.height()))
                for (auto x : range(average.width()))
                    average(x, y) += image(x, y);
        }

        for (auto y : range(average.height()))
            for (auto x : range(average.width()))
                average(x, y) /= float(infiles.size());

        if (!outfile.empty())
        {
            spdlog::info("Writing average image to '{}'.", outfile);
            average.save(outfile);
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("{}", e.what());
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
