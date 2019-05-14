#include "object_lights_config.h"
#include "json_io.h"

namespace aurora
{

lights_config const & aurora::get_object_lights_config(optional<string> path)
{
    static optional<lights_config> instance;
    if(!instance)
    {
        Assert(path);
        std::ifstream stream(*path);
        lights_config cfg;
        json_io::stream_to_data(stream, cfg);
        instance = move(cfg);
    }
    return *instance;
}

}
