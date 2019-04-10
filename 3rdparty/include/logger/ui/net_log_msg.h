/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "reflection/proc/io_streams_refl.h"

namespace logging
{

struct net_log_message
{
    net_log_message()

    {
    }
    
    net_log_message(string const& app, pt::time_duration td, level lev, string source, string file, size_t line, string msg)
        : app           (app)
        , time_dur_ms   (td.total_milliseconds())
        , level         (lev)
        , source        (std::move(source))
        , file          (str(format("(%-26.26s:%4d)") % file % line))
        , msg           (std::move(msg))
    {
        //
    }
    
    std::string         app;
    uint64_t            time_dur_ms;
    binary::size_type   level;
    std::string         source;
    std::string         file;
    std::string         msg;
};

REFL_STRUCT(net_log_message)
    REFL_ENTRY(app)
    REFL_ENTRY(time_dur_ms)
    REFL_ENTRY(level)
    REFL_ENTRY(source)
    REFL_ENTRY(file)
    REFL_ENTRY(msg)
REFL_END()
} // namespace logging

