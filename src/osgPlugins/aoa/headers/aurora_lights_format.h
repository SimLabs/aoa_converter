#pragma once
#include "geometry/primitives/color.h"
#include "geometry/half.h"
#include "reflection/reflection.h"

template<class S, class processor>
REFL_STRUCT_BODY(geom::color_t<S>)
    REFL_ENTRY(r)
    REFL_ENTRY(g)
    REFL_ENTRY(b)
REFL_END()

namespace aurora
{
namespace aod
{

// -- lights packed data struct from AOD files --------------------------------
#pragma pack(push, 1)

constexpr unsigned COORD_3D = 3;

struct omni_light // NOTE: this struct not used because sAodOmni and  sLightOmniVertex identical
{
    // from aurora sources
    struct raw_data
    {
        // forbid usage
        raw_data() = delete;
        float        fPos[COORD_3D];
        float        fPower;
        unsigned     uiData;        // color, Rmin
        unsigned     m_uiRefPowerMul;
    };

    std::array<float, COORD_3D> position;
    float                      power;

    // word 1
    geom::colorb color;
    uint8_t      r_min;
    // word 2
    uint32_t     power_mul = 0;

    REFL_INNER(omni_light)
        REFL_ENTRY(position)
        REFL_ENTRY(power)
        REFL_ENTRY(color)
    REFL_END()
};

static_assert(sizeof(omni_light::raw_data) == sizeof(omni_light), "sizes differ");

struct spot_light
{
    // from aurora sources
    struct raw_data
    {
        // forbid usage
        raw_data() = delete;
        float        fPos[COORD_3D];
        float        fPower;
        unsigned     uiData;     // color, Rmin
        unsigned     uiData1;    // dir.xy
        unsigned     uiData2;    // dir.z, mask
        unsigned     uiData3;    // half fov, angular power
        unsigned     m_uiRefPowerMul;
    };

    std::array<float, COORD_3D> position;
    float                       power;
    // word 1
    geom::colorb color;
    uint8_t      r_min;
    // word 2
    geom::half dir_x, dir_y;
    // word 3
    geom::half dir_z;
    uint16_t   mask = 0x7; // 0x7 enables everything
                           // word 4
    geom::half half_fov;
    geom::half angular_power;
    // word 5
    uint32_t   power_mul = 0;

    REFL_INNER(spot_light)
        REFL_ENTRY(position)
        REFL_ENTRY(power)
        REFL_ENTRY(color)
        REFL_AS_TYPE(dir_x, float)
        REFL_AS_TYPE(dir_y, float)
        REFL_AS_TYPE(dir_z, float)
        REFL_AS_TYPE(half_fov, float)
        REFL_AS_TYPE(angular_power, float)
    REFL_END()
};

static_assert(sizeof(spot_light::raw_data) == sizeof(spot_light), "sizes differ");

#pragma pack(pop)

} // aod
}