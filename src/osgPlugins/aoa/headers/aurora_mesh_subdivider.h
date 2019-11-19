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
    using vertex_index_t = size_t;
    using vertex_attributes_t = std::vector<float>;

    struct data_buffer_t
    {
        refl::data_buffer &info;
        std::vector<uint8_t> data;
    };
private:

    struct vertex_data_t
    {
        const vertex_attributes_t attributes;

        vertex_data_t(const vertex_attributes_t &&attributes);
        vertex_data_t(const vertex_format_t &vertex_format, uint8_t const *bytes);
        void to_byte_buffer(const vertex_format_t &vertex_format, std::vector<size_t> attribute_offsets, std::vector<uint8_t> &byte_buffer) const;
    };

    struct geometry_stream_t
    {
        using aoa_geometry_stream_t = refl::node::controllers_t::control_object_param_data::data_buffer::geometry_buffer_stream;
        using aoa_vao_t = refl::data_buffer::vao_buffer;

        aoa_vao_t &aoa_vao;
        aoa_geometry_stream_t &aoa_geometry_stream;

        const size_t vertex_stride;
        size_t index_stride;

        const std::vector<size_t> attribute_offsets;
        const size_t coordinate_attribute_offset;

        std::vector<vertex_data_t> vertices;
        std::vector<vertex_index_t> indexes;

        geometry_stream_t(data_buffer_t &&, aoa_geometry_stream_t &, aoa_vao_t &);
    };
    
    using mesh_face_data_t = refl::node::mesh_t::mesh_face::mesh_face_offset_count_base_mat;

    refl::aurora_format aoa;
    refl::node &root_node;
    std::map<unsigned, geometry_stream_t> geometry_streams;

    std::filesystem::path dirname, filename;

    float cell_size;

    std::vector<uint8_t> data_buffer;

    void map_data_buffer(std::vector<uint8_t> &);
    
    void subdivide();
    void subdivide_mesh_face(mesh_face_data_t &, geometry_stream_t &, const std::vector<vertex_index_t> &, size_t, size_t);
    void subdivide_triangle(geometry_stream_t &, vertex_index_t, vertex_index_t, vertex_index_t);
    std::optional<float> get_split_point(float a, float b);
    vertex_index_t split_vertex(geometry_stream_t &, vertex_index_t, vertex_index_t, float);

    void assemble_data_buffer(const std::vector<uint8_t> &);
};

}
