#include "plugin_config.h"

namespace aurora
{

plugin_config& get_config(optional<string> path)
{
    static optional<plugin_config> instance;
    if(!instance)
    {
        Assert(path);
        std::ifstream stream(*path);
        plugin_config cfg;
        json_io::stream_to_data(stream, cfg);
        instance = cfg;
    }
    return *instance;
}

}
