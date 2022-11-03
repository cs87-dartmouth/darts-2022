/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996) // strncpy and sscanf not secure
#endif

#include <cstdio> // for size_t
#include <darts/fwd.h>
#include <darts/math.h>
#include <filesystem/fwd.h>
#include <map>       // for map
#include <memory>    // for shared_ptr and make_shared
#include <set>       // for set
#include <stdexcept> // for runtime_error
#include <string>    // for operator+, basic_string
#include <utility>   // for make_pair, pair
#include <vector>    // for vector

#include <nlohmann/json.hpp>

// #define FMT_HEADER_ONLY
#include <fmt/color.h>
#include <fmt/format.h>

#include <spdlog/spdlog.h>

// bringing standard library objects in scope
using std::clamp;
using std::make_pair;
using std::make_shared;
using std::make_unique;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::set;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

/** \addtogroup Utilities
    @{
*/

/** \name Formatted printing and logging

    Darts provides a convenient way to report information to the user.

    This is all based on the `spdlog` and `fmt` libraries. You can find extensive documentation for both of these linked
    from the \ref code-organization section, but in a nutshell:

    # Formatted printing to the console or to strings:
    Darts includes the fmt library to provide a modern replacement for cout or printf.

    The two most useful functions are:
    ```
        fmt::print()
        fmt::format()
    ```

    See the [fmt library website](https://fmt.dev/latest/index.html) for complete documentation.

    # Logging
    The [spdlog library](https://github.com/gabime/spdlog) provides a number of functions:
        - `spdlog::debug`
        - `spdlog::info`
        - `spdlog::warn`
        - `spdlog::err`
        - `spdlog::critical`

    These operate just like `fmt::print`, but colorize the output based on the type of message. Darts can also
    globally disable messages below a certain threshold to control the verbosity of the output (this is done in
    #darts_init, and controlled by a command-line argument in `src/darts.cpp`).

    Darts also allows you to print and update a nice progress bar (see #Progress) for long operations.
    @{
*/

/**
    Initialize darts before parsing or rendering a scene

    \param [in] verbosity specify the verbosity of messages printed to the console
*/
void darts_init(int verbosity = spdlog::level::info);

/// Convert a time value in milliseconds into a human-readable string
string time_string(double time, int precision = 2);

/**
    Indent a string by the specified number of spaces

    Useful for creating nested indentation within error strings.

    \param [in] string  The string to indent
    \param [in] amount  The amount of spaces to indent

    \return The indented string
*/
string indent(const std::string &string, int amount = 2);

/**
    An exception storing a human-readable error description passed in using `fmt:format`-style arguments

    This allows easily constructing human-readable exceptions like so:

    \code{.cpp}
    throw DartsException("Cannot create a '{}' here:\n{}.\n\t{}", type, j.dump(4), error);
    \endcode

    where `type` and `error` are strings and `j` is a #json object.
*/
class DartsException : public std::runtime_error
{
public:
    /// Variadic template constructor to support fmt::format-style arguments
    template <typename... Args>
    DartsException(const char *fmt, const Args &...args) : std::runtime_error(fmt::format(fmt, args...))
    {
    }
};

/**
    Warning signaling unimplemented features.

    Prints out a warning with the line number, but otherwise lets execution continue.
*/
#define put_your_code_here(txt)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        static bool been_here = false;                                                                                 \
        if (!been_here)                                                                                                \
        {                                                                                                              \
            been_here = true;                                                                                          \
            spdlog::warn("{}() not (fully) implemented at {}:{}.\n    msg: {}", __FUNCTION__, __FILE__, __LINE__,      \
                         txt);                                                                                         \
        }                                                                                                              \
    } while (0);

/** @}*/

/**
    Python-style range: iterates from min to max in range-based for loops

    To use:

    \code{.cpp}
    for(int i = 0; i < 100; i++) { ... }             // old way
    for(auto i : range(100))     { ... }             // new way

    for(int i = 10; i < 100; i+=2)  { ... }          // old way
    for(auto i : range(10, 100, 2)) { ... }          // new way

    for(float i = 3.5f; i > 1.5f; i-=0.01f) { ... } // old way
    for(auto i : range(3.5f, 1.5f, -0.01f)) { ... } // new way
    \endcode
*/
template <typename T>
class Range
{
public:
    // Standard iterator support for #Range
    class Iterator
    {
    public:
        Iterator(T pos, T step) : m_pos(pos), m_step(step)
        {
        }

        bool operator!=(const Iterator &o) const
        {
            return (o.m_pos - m_pos) * m_step > T(0);
        }
        Iterator &operator++()
        {
            m_pos += m_step;
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator copy(*this);
                     operator++();
            return copy;
        }
        T operator*() const
        {
            return m_pos;
        }

    private:
        T m_pos, m_step;
    };

    /// Construct an iterable range from \p start to \p end in increments of \p step
    Range(T start, T end, T step = T(1)) : m_start(start), m_end(end), m_step(step)
    {
    }

    Iterator begin() const
    {
        return Iterator(m_start, m_step);
    }
    Iterator end() const
    {
        return Iterator(m_end, m_step);
    }

private:
    T m_start, m_end, m_step;
};

/// Construct a Python-style range from a single parameter \p end
template <typename T>
Range<T> range(T end)
{
    return Range<T>(T(0), end, T(1));
}

/// Construct a Python-style range from \p start, up to but excluding \p end, in increments of \p step
template <typename T>
Range<T> range(T start, T end, T step = T(1))
{
    return Range<T>(start, end, step);
}


/**
    Return the global file resolver instance.

    This class is used to locate resource files (e.g. mesh or texture files) referenced by a scene being loaded.
*/
filesystem::resolver &get_file_resolver();

/** @}*/

/**
    \file
    \brief Common include files and various utility functions.
*/