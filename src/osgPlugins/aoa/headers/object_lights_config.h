#pragma once
#include "aurora_lights_format.h"
#include "reflection/reflection.h"

namespace aurora
{
    struct object_lights
    {
        vector<aod::omni_light> omni_lights;
        vector<aod::spot_light> spot_lights;
        unsigned                clazz;

        REFL_INNER(object_lights)
            REFL_ENTRY(omni_lights)
            REFL_ENTRY(spot_lights)
            REFL_ENTRY_NAMED(clazz, "class")
        REFL_END()
    };

    using lights_config = map<string, object_lights>;

    lights_config const& get_object_lights_config(optional<string> path = string("object_lights.json"));
}
