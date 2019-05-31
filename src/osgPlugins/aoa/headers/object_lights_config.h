#pragma once
#include "aurora_lights_format.h"
#include "reflection/reflection.h"

namespace aurora
{
    struct object_lights
    {
        string                  ref_node;
        vector<aod::omni_light> omni_lights;
        vector<aod::spot_light> spot_lights;

        REFL_INNER(object_lights)
            REFL_ENTRY(ref_node)
            REFL_ENTRY(omni_lights)
            REFL_ENTRY(spot_lights)
        REFL_END()
    };

    using lights_config = map<string, object_lights>;

    lights_config const& get_object_lights_config(optional<string> path = string("object_lights.json"));
}
