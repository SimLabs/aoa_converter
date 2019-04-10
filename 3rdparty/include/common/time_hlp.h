/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

// look please at http://www.boost.org/doc/libs/1_50_0/doc/html/date_time/posix_time.html
// for full posix time functions reference

// ptime.date() -> date
// ptime.time_of_day() -> time_duration
// utc_time() - start_of_epoch() ->time_duration

#include "boost/date_time/posix_time/posix_time.hpp"

#include "cpp_utils/inplace_format.h"

namespace bt = boost::posix_time;
typedef   bt::time_duration td_t;

namespace boost
{
namespace posix_time
{

//////////////////////////////////////////////////////////////////////////
// Epoch, usually 00:00:00 1/1/1970 
inline ptime start_of_epoch()
{
    return from_time_t(0);
}

//////////////////////////////////////////////////////////////////////////
// conversions to/from double
inline double to_double(time_duration dur)
{
    return dur.total_microseconds() * 1e-6;
}

inline time_duration from_double(double dur_in_seconds)
{
    long    sec = long   (dur_in_seconds);
    int64_t mms = int64_t(1e6*(dur_in_seconds - double(sec)));

    // note: there was a bug: https://svn.boost.org/trac/boost/ticket/3471 
    return bt::seconds(sec) + bt::microsec(mms);
}

inline long to_unsigned(time_duration dur)
{
    return dur.total_seconds();
}

inline time_duration from_unsigned(long dur_in_seconds)
{
    return seconds(long(dur_in_seconds));
}

//////////////////////////////////////////////////////////////////////////
// conversion to string (there many already "to_simple_string", "to_iso_string", ... - look at documentation)
inline std::string to_short_string(time_duration dur)
{
    return str(cpp_utils::inplace_format("%3d:%02d:%02d.%03d")
        % dur.hours  ()
        % dur.minutes()
        % dur.seconds()
        % (dur.total_milliseconds() % int64_t(1000)));
}

// time is absolute time
inline std::string to_file_string(ptime time)
{
    std::string str = to_simple_string(time);
    boost::algorithm::replace_all(str, ":", ".");
    boost::algorithm::replace_all(str, " ", "_");

    return str;
}

// convert global time-range to string
inline std::string time_range_to_string(unsigned from, unsigned to)
{
    time_duration const dt_lo = (start_of_epoch() + from_unsigned(from)).time_of_day();
    time_duration const dt_hi = (start_of_epoch() + from_unsigned(to)).time_of_day();

    return str(cpp_utils::inplace_format("%02d:%02d - %02d:%02d") % dt_lo.hours() % dt_lo.minutes() % dt_hi.hours() % dt_hi.minutes());
}

//////////////////////////////////////////////////////////////////////////
// clock helpers
// be careful, these function don't achieve microseconds resolution on most Win32

inline ptime loc_time() 
{
    return microsec_clock::local_time();
}

inline ptime utc_time()
{
    return microsec_clock::universal_time();
}

}
}

