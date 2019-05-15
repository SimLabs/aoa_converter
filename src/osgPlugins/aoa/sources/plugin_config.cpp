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

const osg::Matrix plugin_config::flip_YZ_matrix = {
    1, 0, 0, 0,
    0, 0, 1, 0,
    0, -1, 0, 0,
    0, 0, 0, 1
};

const osg::Matrix plugin_config::reverse_flip_YZ_matrix = {
    1, 0, 0, 0,
    0, 0,-1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};

}
