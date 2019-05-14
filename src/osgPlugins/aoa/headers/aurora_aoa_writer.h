#pragma once
#include "vao.h"
#include "geometry/primitives/quaternion.h"
#include "aurora_format.h"
#include "aurora_lights_format.h"

namespace aurora
{

struct write_aoa_visitor;

using aurora::refl::vertex_attrs::type_t;
using aurora::refl::vertex_attrs::mode_t;
using vertex_attribute = refl::data_buffer::vao_buffer::vertex_format::vertex_attribute;

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

    void set_omni_lights_buffer_data(vector<aod::omni_light> const& data);
    void set_spot_lights_buffer_data(vector<aod::spot_light> const& data);

    node_ptr get_root_node();
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


        template<typename T>
        node_ptr add_mesh(geom::rectangle_3f bbox, vector<T>const & attributes, vector<vertex_attribute> format, vector<face> const& faces, float lod, string material, string shadow_material = "Shadow_Common")
        {
            return add_mesh_impl(bbox, reinterpret_cast<const char*>(attributes.data()), attributes.size() * sizeof(T), attributes.size(), format, faces, lod, material, shadow_material);
        }

        template<typename T>
        node_ptr add_collision_mesh(vector<T>const & attributes, vector<face> const& faces, vector<vertex_attribute> format)
        {
            return add_collision_mesh_impl(reinterpret_cast<const char*>(attributes.data()), attributes.size() * sizeof(T), attributes.size(), faces, format);
        }

        node_ptr set_omni_lights(unsigned offset, unsigned size);
        node_ptr set_spot_lights(unsigned offset, unsigned size);

        node_ptr  set_cvbox_spec(geom::rectangle_3f const& box);
        node_ptr  set_cvsphere_spec(geom::sphere_3f const& sphere);
        node_ptr  add_control_pos_key_spec(float key, geom::point_3f pos);
        node_ptr  add_control_rot_key_spec(float key, geom::quaternionf rot);
        node_ptr  set_control_ref_node_spec(string name);
    private:
        node_ptr  set_collision_stream_spec(pair<unsigned, unsigned> vertex_offset_size, pair<unsigned, unsigned> index_offset_size);
        node_ptr  add_cvmesh_spec(unsigned vao, std::pair<unsigned, unsigned> vertex_offset_size, std::pair<unsigned, unsigned> index_offset_size);
        node_ptr  add_mesh_spec(geom::rectangle_3f bbox, unsigned offset, unsigned count, unsigned base_vertex, unsigned num_vertices, string material, string shadow_material = "Shadow_Common", std::pair<unsigned, unsigned> vao_ref = {0, 0});
        node_ptr  add_collision_mesh_impl(const char* data, size_t size, size_t num_vertices, vector<face> const& faces, vector<vertex_attribute> format);
        node_ptr  add_mesh_impl(geom::rectangle_3f bbox, const char* data, size_t size, size_t num_vertices, vector<vertex_attribute> format, vector<face> const& faces, float lod, string material, string shadow_material);
        node_ptr  add_geometry_stream(float lod, pair<unsigned, unsigned> vertex_offset_size, pair<unsigned, unsigned> index_offset_size);
        node_ptr  add_omnilights_stream(unsigned offset, unsigned size);
        node_ptr  add_spotlights_stream(unsigned offset, unsigned size);
        node_ptr set_name(string name);
        string   get_name() const;
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