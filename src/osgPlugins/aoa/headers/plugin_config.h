#pragma once

#include "json_io.h"

namespace aurora
{
    enum light_types
    {
        PAPI, 
        APPROACH, 
        RUNWAY_EDGE, 
        RUNWAY_CENTER,
        RUNWAY_TOUCHZONE,

        TAXIWAY_CENTER,
        TAXYWAY_EDGE
    };

    struct plugin_config
    {
        struct lights_config
        {
            map<string, vector<string>> light_kind_node_names_map;

            REFL_INNER(lights_config)
                REFL_ENTRY(light_kind_node_names_map)
            REFL_END()
        };

        lights_config lights;

        REFL_INNER(plugin_config)
            REFL_ENTRY(lights)
        REFL_END()
    };

    plugin_config& get_config(optional<string> path = boost::none);
}