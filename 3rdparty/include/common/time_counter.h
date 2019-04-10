/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "performance_counter.h"
#include "time_hlp.h"

struct time_counter
{
    typedef boost::posix_time::ptime            ptime;
    typedef boost::posix_time::time_duration    time_duration;
    typedef boost::posix_time::microsec_clock   clock_t;

    time_counter()
    {
        reset();
    }

    static std::string human_readable_time(time_duration const& td)
    {
        return bt::to_short_string(td);
    }

    static double to_double(time_duration const& td)
    {
        return bt::to_double(td);
    }

    time_duration time() const
    {
        using namespace boost::posix_time;
        return microseconds(int64_t(counter_.time_ms() * 1000.));
    }

    void reset()
    {
        counter_.reset();
    }

private:

    PerformanceCounter counter_;
    //perf_counter counter_;

};
