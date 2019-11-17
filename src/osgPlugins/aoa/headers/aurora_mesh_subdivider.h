#pragma once
#include "aurora_format.h"

#include <filesystem>
#include <optional>

namespace aurora
{

struct mesh_subdivider
{
    mesh_subdivider(const std::filesystem::path &&aoa_path, float cell_size);

    const refl::aurora_format &processed_aoa() const;
    const std::vector<uint8_t> &processed_aod() const;

    using vertex_format_t = refl::data_buffer::vao_buffer::vertex_format;
    using vertex_index_t = int;
    using vertex_attributes_t = std::vector<std::vector<float>>;
private:

    struct vertex_data_t
    {
        const vertex_attributes_t attributes;

        vertex_data_t(const vertex_attributes_t &&attributes);
        vertex_data_t(const vertex_format_t &vertex_format, uint8_t const *bytes);
        void to_byte_buffer(const vertex_format_t &vertex_format, std::vector<uint8_t> &byte_buffer) const;
    };

    struct vertex_index_data_t
    {
        std::vector<vertex_data_t> vertices;
        std::vector<int> indexes;
    };

    using vao_ref_t = std::pair<unsigned, unsigned>;
    using geometry_stream_t = refl::node::controllers_t::control_object_param_data::data_buffer::geometry_buffer_stream;
    using mesh_face_data_t = refl::node::mesh_t::mesh_face::mesh_face_offset_count_base_mat;
    using mesh_data_t = std::tuple<refl::node::mesh_t &, vertex_index_data_t, std::vector<mesh_face_data_t>>;

    refl::aurora_format aoa;
    refl::node &root_node;
    std::filesystem::path dirname, filename;

    float cell_size;

    std::multimap<vao_ref_t, mesh_data_t> data_cache;
    std::vector<uint8_t> data_buffer_in, data_buffer_out;
    std::map<std::tuple<vertex_index_t, vertex_index_t, float>, vertex_index_t> split_vertex_cache;
    
    void subdivide();
    void fill_processed_data();

    mesh_face_data_t subdivide_mesh_face(
        const refl::data_buffer::vao_buffer &vao,
        const geometry_stream_t &stream,
        unsigned vertex_stride,
        const mesh_face_data_t& face_info,
        vertex_index_data_t& mesh_vertex_index_data
    );
    
    void subdivide_triangle(
        const vertex_format_t &vertex_format,
        vertex_index_data_t &face_data,
        vertex_index_t base_vertex,
        vertex_index_t ia0, vertex_index_t ib0, vertex_index_t ic0
    );

    vertex_index_t split_vertex(const vertex_format_t &vertex_format, vertex_index_data_t &face_data, vertex_index_t ia, vertex_index_t ib, float alpha);

    std::optional<float> get_split_point(float a, float b) const;

    geometry_stream_t &get_geometry_stream(unsigned stream_id) const;
};

}
