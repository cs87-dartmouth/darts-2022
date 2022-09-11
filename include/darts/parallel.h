/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <nanothread/nanothread.h>

/** \addtogroup Parallel
    @{
*/

/// Bring nanothread functions into scope
using namespace drjit;

/**
    Implements a simple barrier thread-coordination mechanism using a condition variable and mutex

    Generally you would construct a Barrier of a certain size, and then pass it to the parallel tasks. The tasks that
    call #Barrier::block wait until the specified number of tasks have reached the barrier. Here is a simple example:
    \code{.cpp}
    Barrier *barrier = new Barrier(5);
    parallel_for(blocked_range<uint32_t>(0, 15),
                 [barrier](blocked_range<uint32_t> range)
                 {
                    // do some work

                    // now wait until 5 threads have finished the above work
                    if (barrier->block())
                        delete barrier;

                    // we now know that 5 threads have reached and passed the barrier
                 });
    \endcode
*/
class Barrier
{
public:
    explicit Barrier(int n) : m_num_to_block(n), m_num_to_exit(n)
    {
    }

    Barrier(const Barrier &)            = delete;
    Barrier &operator=(const Barrier &) = delete;

    /// Returns true to only one thread (which should delete the barrier).
    bool block()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        --m_num_to_block;

        if (m_num_to_block > 0)
        {
            m_cv.wait(lock, [this]() { return m_num_to_block == 0; });
        }
        else
            m_cv.notify_all();

        return --m_num_to_exit == 0;
    }

private:
    std::mutex              m_mutex;
    std::condition_variable m_cv;
    int                     m_num_to_block, m_num_to_exit;
};

/// Run a function once on each thread in thread Pool \p pool
inline void for_each_thread(std::function<void(void)> func, Pool *pool = nullptr)
{
    Barrier *barrier = new Barrier(pool_size(pool) + 1);
    parallel_for(blocked_range<uint32_t>(0, pool_size(pool) + 1),
                 [barrier, &func](blocked_range<uint32_t> range)
                 {
                     func();
                     if (barrier->block())
                         delete barrier;
                 });
}

/** @}*/

/**
    \file
    \brief parallel processing functionality
*/