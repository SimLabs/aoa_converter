#pragma once
#include "aurora_format.h"

#include <filesystem>

namespace aurora
{

struct mesh_subdivider
{
    mesh_subdivider::mesh_subdivider(refl::aurora_format &aoa, string const& aoa_path, refl::node root_node, float cell_size);

private:
    struct face_data_t
    {
        std::vector<uint8_t> vertex_data;
        int triangle_count;
        std::vector<int> indexes;
        int vertex_count;
    };

    struct mesh_data_t
    {
        refl::node::mesh_t &associated_mesh;
        std::vector<face_data_t> face_data;
    };

    struct data_cache_t
    {
        std::multimap<refl::node::mesh_t::mesh_vao_ref, mesh_data_t> mesh_data;
    };

    refl::aurora_format aoa;
    refl::node &root_node;
    std::filesystem::path dirname, filename;

    float cell_size;
    data_cache_t data_cache;
    std::vector<uint8_t> data_buffer_in, data_buffer_out;

    void process();
    void subdivide();
    void subdivide_mesh_face(refl::node::mesh_t::mesh_vao_ref &, refl::node::mesh_t::mesh_face::mesh_face_offset_count_base_mat &, face_data_t &);
    void subdivide_triangle(int vao_id, uint8_t vertices_begin, face_data_t &output_face_data, int ia0, int ib0, int ic0);
    void build_data_buffer();
};

}
