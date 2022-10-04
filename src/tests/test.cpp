/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/factory.h>
#include <darts/image.h>
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
            spdlog::info("Passed all {}/{} tests!", num_passed, count);
            exit(EXIT_SUCCESS);
        }
        else
        {
            spdlog::error("Failed {}/{} tests.", count - num_passed, count);
            exit(EXIT_FAILURE);
        }
    }
}


/**
    \dir
    \brief Darts Test plugins source directory
*/