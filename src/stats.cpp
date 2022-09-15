/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz

    This file are adapted from PBRTv4, it's copyright follows:

        pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
        The pbrt source code is licensed under the Apache License, Version 2.0.
        SPDX: Apache-2.0
*/

#include <darts/common.h>
#include <darts/stats.h>

// Statistics Local Variables
static std::vector<StatRegisterer::AccumFunc> *stat_funcs;
static StatsAccumulator                        stats_accumulator;

void accumulate_thread_stats()
{
    static std::mutex           mutex;
    std::lock_guard<std::mutex> lock(mutex);
    StatRegisterer::call_callbacks(stats_accumulator);
}

void StatRegisterer::call_callbacks(StatsAccumulator &accum)
{
    for (AccumFunc func : *stat_funcs)
        func(accum);
}

StatRegisterer::StatRegisterer(AccumFunc func)
{
    static std::mutex           mutex;
    std::lock_guard<std::mutex> lock(mutex);

    if (!stat_funcs)
        stat_funcs = new std::vector<AccumFunc>;
    if (func)
        stat_funcs->push_back(func);
}

struct StatsAccumulator::Stats
{
    std::map<std::string, int64_t> counters;
    std::map<std::string, int64_t> memory_counters;
    template <typename T>
    struct Distribution
    {
        int64_t count = 0;
        T       min   = std::numeric_limits<T>::max();
        T       max   = std::numeric_limits<T>::lowest();
        T       sum   = 0;
    };
    std::map<std::string, Distribution<int64_t>>       int_distributions;
    std::map<std::string, Distribution<double>>        float_distributions;
    std::map<std::string, std::pair<int64_t, int64_t>> percentages;
    std::map<std::string, std::pair<int64_t, int64_t>> ratios;
};

void StatsAccumulator::report_counter(const char *name, int64_t val)
{
    stats->counters[name] += val;
}

StatsAccumulator::StatsAccumulator()
{
    stats = new Stats;
}

void StatsAccumulator::report_memory_counter(const char *name, int64_t val)
{
    stats->memory_counters[name] += val;
}

void StatsAccumulator::report_percentage(const char *name, int64_t num, int64_t denom)
{
    stats->percentages[name].first += num;
    stats->percentages[name].second += denom;
}

void StatsAccumulator::report_ratio(const char *name, int64_t num, int64_t denom)
{
    stats->ratios[name].first += num;
    stats->ratios[name].second += denom;
}

void StatsAccumulator::report_int_distribution(const char *name, int64_t sum, int64_t count, int64_t min, int64_t max)
{
    Stats::Distribution<int64_t> &distrib = stats->int_distributions[name];
    distrib.sum += sum;
    distrib.count += count;
    distrib.min = std::min(distrib.min, min);
    distrib.max = std::max(distrib.max, max);
}

void StatsAccumulator::report_float_distribution(const char *name, double sum, int64_t count, double min, double max)
{
    Stats::Distribution<double> &distrib = stats->float_distributions[name];
    distrib.sum += sum;
    distrib.count += count;
    distrib.min = std::min(distrib.min, min);
    distrib.max = std::max(distrib.max, max);
}

std::string stats_report()
{
    return stats_accumulator.report();
}

void clear_stats()
{
    stats_accumulator.clear();
}

static std::vector<std::string> split_string(std::string_view str, char ch)
{
    std::vector<std::string> strings;

    if (str.empty())
        return strings;

    std::string_view::iterator begin = str.begin();
    while (true)
    {
        std::string_view::iterator end = begin;
        while (end != str.end() && *end != ch)
            ++end;

        strings.push_back(std::string(begin, end));

        if (end == str.end())
            break;

        begin = end + 1;
    }

    return strings;
}

static void get_category_and_title(const std::string &str, std::string *category, std::string *title)
{
    std::vector<std::string> comps = split_string(str, '/');
    if (comps.size() == 1)
        *title = comps[0];
    else
    {
        *category = comps[0];
        *title    = comps[1];
    }
}

std::string StatsAccumulator::report()
{
    std::string dest;
    dest +=
        fmt::format(fmt::emphasis::bold | fg(fmt::color::light_sea_green), "{:—<95}\n Statistics:\n{:—<95}\n", "", "");

    std::map<std::string, std::vector<std::string>> to_print;

    for (auto &counter : stats->counters)
    {
        if (counter.second == 0)
            continue;
        std::string category, title;
        get_category_and_title(counter.first, &category, &title);
        to_print[category].push_back(fmt::format(fmt::emphasis::italic, "{:<42}               ", title) +
                                     fmt::format(fg(fmt::color::cornflower_blue), "{:>12d}", counter.second));
    }

    size_t totalMemoryReported = 0;
    auto   printBytes          = [](size_t bytes) -> std::string
    {
        float kb = (float)bytes / 1024.f;
        if (std::abs(kb) < 1024.f)
            return fmt::format(fg(fmt::color::cornflower_blue), "{:9.2f}", kb) +
                   fmt::format(fg(fmt::color::dim_gray), " kB");

        float mib = kb / 1024.f;
        if (std::abs(mib) < 1024.f)
            return fmt::format(fg(fmt::color::cornflower_blue), "{:9.2f}", mib) +
                   fmt::format(fg(fmt::color::dim_gray), " MiB");

        float gib = mib / 1024.f;
        return fmt::format(fg(fmt::color::cornflower_blue), "{:9.2f}", gib) +
               fmt::format(fg(fmt::color::dim_gray), " GiB");
    };

    for (auto &counter : stats->memory_counters)
    {
        if (counter.second == 0)
            continue;
        totalMemoryReported += counter.second;

        std::string category, title;
        get_category_and_title(counter.first, &category, &title);
        to_print[category].push_back(fmt::format(fmt::emphasis::italic, "{:<42}                  ", title) +
                                     printBytes(counter.second));
    }
    // int64_t unreportedBytes = GetCurrentRSS() - totalMemoryReported;
    // if (unreportedBytes > 0)
    //     to_print["Memory"].push_back(
    //         fmt::format("{:<42}                  {}", "Unreported / unused", printBytes(unreportedBytes)));

    for (auto &distrib : stats->int_distributions)
    {
        const std::string &name = distrib.first;
        if (distrib.second.count == 0)
            continue;
        std::string category, title;
        get_category_and_title(name, &category, &title);
        double avg = (double)distrib.second.sum / (double)distrib.second.count;
        to_print[category].push_back(
            fmt::format(fmt::emphasis::italic, "{:<42}                      ", title) +
            fmt::format(fg(fmt::color::cornflower_blue), "{:.3f}", avg) +
            fmt::format(fg(fmt::color::dim_gray), " avg [range {} - {}]", distrib.second.min, distrib.second.max));
    }
    for (auto &distrib : stats->float_distributions)
    {
        const std::string &name = distrib.first;
        if (distrib.second.count == 0)
            continue;
        std::string category, title;
        get_category_and_title(name, &category, &title);
        double avg = (double)distrib.second.sum / (double)distrib.second.count;
        to_print[category].push_back(
            fmt::format(fmt::emphasis::italic, "{:<42}                      ", title) +
            fmt::format(fg(fmt::color::cornflower_blue), "{:.3f}", avg) +
            fmt::format(fg(fmt::color::dim_gray), " avg [range {} - {}]", distrib.second.min, distrib.second.max));
    }
    for (auto &percentage : stats->percentages)
    {
        if (percentage.second.second == 0)
            continue;
        int64_t     num   = percentage.second.first;
        int64_t     denom = percentage.second.second;
        std::string category, title;
        get_category_and_title(percentage.first, &category, &title);
        to_print[category].push_back(fmt::format(fmt::emphasis::italic, "{:<42}", title) +
                                     fmt::format(fg(fmt::color::medium_sea_green), "{:>12}", num) + fmt::format(" / ") +
                                     fmt::format(fg(fmt::color::cornflower_blue), "{:>12}", denom) +
                                     fmt::format(fg(fmt::color::dim_gray), " ({:.2f}%)", (100.f * num) / denom));
    }
    for (auto &ratio : stats->ratios)
    {
        if (ratio.second.second == 0)
            continue;
        int64_t     num   = ratio.second.first;
        int64_t     denom = ratio.second.second;
        std::string category, title;
        get_category_and_title(ratio.first, &category, &title);
        to_print[category].push_back(fmt::format(fmt::emphasis::italic, "{:<42}", title) +
                                     fmt::format(fg(fmt::color::medium_sea_green), "{:>12}", num) + fmt::format(" / ") +
                                     fmt::format(fg(fmt::color::cornflower_blue), "{:>12}", denom) +
                                     fmt::format(fg(fmt::color::dim_gray), " ({:.2f}x)", (double)num / (double)denom));
    }

    for (auto &categories : to_print)
    {
        dest += fmt::format(fmt::emphasis::italic | fmt::emphasis::bold, "  {}\n", categories.first);
        for (auto &item : categories.second)
            dest += fmt::format("    {}\n", item);
    }

    dest += fmt::format(fmt::emphasis::bold | fg(fmt::color::light_sea_green), "{:—<95}\n", "");

    return dest;
}

void StatsAccumulator::clear()
{
    stats->counters.clear();
    stats->memory_counters.clear();
    stats->int_distributions.clear();
    stats->float_distributions.clear();
    stats->percentages.clear();
    stats->ratios.clear();
}
