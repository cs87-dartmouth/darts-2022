/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <cmath>
#include <darts/common.h>
#include <filesystem/resolver.h>


#include <spdlog/sinks/stdout_color_sinks.h>

void darts_init(int verbosity)
{
    // changing the default logger requires calling this twice since the default empty name is already taken
    // here we force the default logger to always output in color
    spdlog::set_default_logger(spdlog::stdout_color_mt("some_arbitrary_name"));
    spdlog::set_default_logger(spdlog::stdout_color_mt("", spdlog::color_mode::always));

    spdlog::set_pattern("%^%v%$");
    spdlog::set_level(spdlog::level::level_enum(verbosity));

}

std::string time_string(double time, int precision)
{
    if (std::isnan(time) || std::isinf(time))
        return "inf";

    int seconds = int(time) / 1000;
    int minutes = seconds / 60;
    int hours   = minutes / 60;
    int days    = hours / 24;

    if (days > 0)
        return fmt::format("{}d:{:0>2}h:{:0>2}m", days, hours % 24, minutes % 60);
    else if (hours > 0)
        return fmt::format("{}h:{:0>2}m:{:0>2}s", hours % 24, minutes % 60, seconds % 60);
    else if (minutes > 0)
        return fmt::format("{}m:{:0>2}s", minutes % 60, seconds % 60);
    else if (seconds > 0)
        return fmt::format("{:.3f}s", time / 1000);
    else
        return fmt::format("{}ms", int(time));
}

std::string indent(const std::string &string, int amount)
{
    // This could probably be done faster (it's not really speed-critical though)
    std::istringstream iss(string);
    std::string        output;
    std::string        spacer(amount, ' ');
    bool               first_line = true;
    for (std::string line; std::getline(iss, line);)
    {
        if (!first_line)
            output += spacer;
        output += line;
        if (!iss.eof())
            output += '\n';
        first_line = false;
    }
    return output;
}

filesystem::resolver &get_file_resolver()
{
    static filesystem::resolver resolver;
    return resolver;
}

/**
    \dir
    \brief main darts source directory
*/
