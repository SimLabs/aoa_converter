#pragma once

#include "json_io.h"

namespace aurora
{
    enum light_type
    {
        STOPBARS,
        PAPI, 
        APPROACH, 

        RUNWAY_EDGE, 
        RUNWAY_CENTER,
        RUNWAY_TOUCHZONE,
        RUNWAY_END,
        RUNWAY_STROBE,

        TAXIWAY_CENTER,
        TAXYWAY_EDGE
    };

    ENUM_DECL(light_type)
        ENUM_DECL_ENTRY(STOPBARS)
        ENUM_DECL_ENTRY(PAPI)
        ENUM_DECL_ENTRY(APPROACH)

        ENUM_DECL_ENTRY(RUNWAY_EDGE)
        ENUM_DECL_ENTRY(RUNWAY_CENTER)
        ENUM_DECL_ENTRY(RUNWAY_TOUCHZONE)
        ENUM_DECL_ENTRY(RUNWAY_END)
        ENUM_DECL_ENTRY(RUNWAY_STROBE)

        ENUM_DECL_ENTRY(TAXIWAY_CENTER)
        ENUM_DECL_ENTRY(TAXYWAY_EDGE)
    ENUM_DECL_END()

    struct plugin_config
    {
        struct lights_config
        {
            // TODO: change key to light_type
            map<string, vector<string>> light_kind_node_names_map;
            map<string, string> light_kind_node_ref;

            REFL_INNER(lights_config)
                REFL_ENTRY(light_kind_node_names_map)
                REFL_ENTRY(light_kind_node_ref)
            REFL_END()
        };

        lights_config lights;

        REFL_INNER(plugin_config)
            REFL_ENTRY(lights)
        REFL_END()
    };

    plugin_config& get_config(optional<string> path = boost::none);
}