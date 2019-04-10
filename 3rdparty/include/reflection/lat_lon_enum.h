#pragma once

// TODO: remove this bullshit

namespace ui_cfg
{

enum lat_lon_kind
{
    LL_LAT = 0,
    LL_LON,

    LL_SIZE
};

inline const char* to_string(lat_lon_kind kind)
{
    static const std::vector<const char*> names = {
        "lat"
        , "lon"
    };

    Assert(kind < names.size());
    return names[kind];
}

}