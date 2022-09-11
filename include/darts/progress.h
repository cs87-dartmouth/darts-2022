/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <atomic>
#include <string>
#include <thread>

/** \addtogroup Utilities
    @{
*/

/**
    A thread-safe helper object to display and update a progress bar in the terminal.

    Example usage:

    \code{.cpp}
    {
        Progress p1("Solving", 10);
        for (int i = 0; i < 10; ++i, ++p1)
        {
            // do something
        }
    } // end progress p1
    \endcode
*/
class Progress
{
public:
    /**
        Create a progress bar with a \p title and \p total_steps number of steps.

        \param title        A descriptive title for the task being performed.

        \param total_steps  When this is positive, it indicates the number of steps for the calculation. When it is
                            non-positive, an indeterminate progress bar is created instead (for a task with an unknown
                            number of steps), and \p total_steps is used as the period, in milliseconds, of the
                            progress bar's spinning animation. If 0 is specified, a default number of milliseconds is
                            used.
     */
    Progress(const std::string &title, int64_t total_steps = 0);
    ~Progress();

    /// increment the progress by \p steps. \see operator+=()
    void step(int64_t steps = 1)
    {
        m_steps_done += steps;
    }

    /// mark the progress as complete
    void set_done();

    /// the current progress
    int progress() const
    {
        return int(m_steps_done);
    }

    /// increment the progress by 1 step
    Progress &operator++()
    {
        step();
        return *this;
    }

    /// increment the progress by \p steps. \see step()
    Progress &operator+=(int64_t steps)
    {
        step(steps);
        return *this;
    }

private:
    std::string          m_title;
    int64_t              m_num_steps;
    std::atomic<int64_t> m_steps_done;
    std::atomic<bool>    m_exit;
    std::thread          m_update_thread;
};

/** @}*/

/**
    \file
    \brief Class #Progress
*/
