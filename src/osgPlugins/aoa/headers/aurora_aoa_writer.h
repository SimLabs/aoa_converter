#pragma once
#include "vao.h"
#include "geometry/primitives/color.h"
#include "geometry/half.h"

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

    float         position[COORD_3D];
    float         power;

    // word 1
    geom::colorb color;
    uint8_t      r_min;
    // word 2
    uint32_t     power_mul = 0;
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

    float        position[COORD_3D];
    float        power;
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
};

static_assert(sizeof(spot_light::raw_data) == sizeof(spot_light), "sizes differ");

#pragma pack(pop)

} // aod

struct geometry_visitor;

struct aoa_writer
{
    enum node_flags
    {
        TREAT_CHILDREN   = 0x1,
        DRAW_LIGHTPOINTS = 0x2,
        DRAW_MESH        = 0x4,
        GLOBAL_NODE      = 0x8
    };

    aoa_writer(string path);
    ~aoa_writer();

    struct node;
    using node_ptr = std::shared_ptr<node>;

    static string material_name_for_node(string node_name)
    {
        return node_name + "_mtl";
    }

    void set_index_buffer_data(vector<face> const& data);
    void set_vertex_buffer_data(vector<vertex_info> const& data);
    void set_omni_lights_buffer_data(vector<aod::omni_light> const& data);
    void set_spot_lights_buffer_data(vector<aod::spot_light> const& data);

    node_ptr create_root_node(string name);
    aoa_writer& add_material(string name, material_info const& mat);
    void save_data();

    struct node: std::enable_shared_from_this<node>
    {
        friend struct aoa_writer;
        node(aoa_writer& writer);
        node(node&&);
        ~node();

        node_ptr create_child(string name);
        node_ptr add_flags(uint32_t flags);
        node_ptr set_draw_order(unsigned order);

        node_ptr add_mesh(geom::rectangle_3f bbox, unsigned offset, unsigned count, unsigned base_vertex, unsigned num_vertices, std::pair<unsigned, unsigned> vao_ref = {0, 0});
        node_ptr set_omni_lights(unsigned offset, unsigned size);
        node_ptr set_spot_lights(unsigned offset, unsigned size);

    private:
        node_ptr  add_geometry_stream(float lod, pair<unsigned, unsigned> vertex_offset_size, pair<unsigned, unsigned> index_offset_size);
        node_ptr  add_omnilights_stream(unsigned offset, unsigned size);
        node_ptr  add_spotlights_stream(unsigned offset, unsigned size);
        node_ptr set_name(string name);
        void* get_underlying();

    private:
        struct impl;
        unique_ptr<impl> pimpl_;
    };

private:
    node_ptr create_node();

private:
    struct impl;
    unique_ptr<impl> pimpl_;
};


}