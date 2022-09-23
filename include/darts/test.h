/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/array2d.h>
#include <darts/common.h>
#include <darts/fwd.h>

/// Base class for unit tests in Darts
struct Test
{
    /**
        Run the actual test.

        This is where all the work happens. Derived classes should override this function.
    */
    virtual void run() = 0;
};

/// Check if this json object contains tests, and run them
void run_tests(const json &j);


/**
    \file
    \brief Class #Test.
*/