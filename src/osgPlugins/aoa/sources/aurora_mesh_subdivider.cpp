#include "aurora_mesh_subdivider.h"

namespace aurora {
    mesh_subdivider::mesh_subdivider(refl::aurora_format &aoa, string const& aoa_path, refl::node root_node, float cell_size)
        : aoa(aoa)
        , root_node(root_node)
        , dirname(std::filesystem::path(aoa_path).parent_path())
        , filename(std::filesystem::path(aoa_path).filename())
        , cell_size(cell_size)
    {
        std::ifstream buffer_file(dirname / string(aoa.buffer_data.data_buffer_file), std::ios_base::binary);
        std::ostringstream ss;
        ss << buffer_file.rdbuf();
        auto content = ss.str();

        data_buffer_in = { content.begin(), content.end() };
    }

    void mesh_subdivider::process()
    {
        subdivide();
        build_data_buffer();
    }

    void mesh_subdivider::subdivide()
    {
        for (auto &nd : aoa.nodes)
            if (auto &mesh = nd.mesh) {
                mesh_data_t mesh_data{ mesh.value() };

                mesh_data.face_data.resize(mesh->face_array.size());
                int face_index = 0;
                for (auto &mesh_face : mesh->face_array) {
                    if (auto &face_info = mesh_face.with_shadow_mat)
                        subdivide_mesh_face(mesh->vao_ref, face_info.value(), mesh_data.face_data[face_index]);
                    ++face_index;
                }

                data_cache.mesh_data.emplace(mesh->vao_ref, mesh_data);
            }
    }

    void mesh_subdivider::subdivide_mesh_face(
        refl::node::mesh_t::mesh_vao_ref &vao_ref,
        refl::node::mesh_t::mesh_face::mesh_face_offset_count_base_mat &face_info, 
        face_data_t &output_face_data
    ) {
        auto short_index = aoa.buffer_data.vaos[vao_ref.vao_id].vertex_format_offset.format == 0xFFFF;
        auto index_size = short_index ? 2 : 4;
        auto indexes_begin = data_buffer_in.data()
            + root_node.controllers.object_param_controller->buffer.geometry_streams[vao_ref.geom_stream_id].index_offset_size.offset
            + index_size * face_info.offset;
        auto vertices_start = data_buffer_in.data()
            + root_node.controllers.object_param_controller->buffer.geometry_streams[vao_ref.geom_stream_id].vertex_offset_size.offset
            + index_size * face_info.offset;

        if (short_index)
        {
            auto indexes_in_begin = reinterpret_cast<uint16_t *>(indexes_begin);
            for (int i = 0; i < face_info.count; ++i)
            {
                subdivide_triangle(
                    vao_ref.vao_id,
                    vertices_start,
                    output_face_data,
                    *(indexes_in_begin + 3 * i + 0),
                    *(indexes_in_begin + 3 * i + 1),
                    *(indexes_in_begin + 3 * i + 2)
                );
            }
        }
        else
        {
            auto indexes_in_begin = reinterpret_cast<uint32_t *>(indexes_begin);
            for (int i = 0; i < face_info.count; ++i)
            {
                subdivide_triangle(
                    vao_ref.vao_id,
                    vertices_start,
                    output_face_data,
                    *(indexes_in_begin + 3 * i + 0),
                    *(indexes_in_begin + 3 * i + 1),
                    *(indexes_in_begin + 3 * i + 2)
                );
            }
        }

    }

    void mesh_subdivider::subdivide_triangle(int vao_id, uint8_t vertices_begin, face_data_t &output_face_data, int ia0, int ib0, int ic0)
    {
        
    }

    void mesh_subdivider::build_data_buffer() // filling output data buffer
    {
        std::map<int, int> stream_vao_mapping;
        std::map<int, std::vector<int>> vao_stream_mapping;
        std::map<int, face_data_t> stream_data;
        
        for (auto &[stream_vao_id, mesh_data] : data_cache.mesh_data)
        {
            auto &[stream_id, vao_id] = stream_vao_id;
            if (stream_vao_mapping.find(stream_id) == stream_vao_mapping.end()) { // ensure that each geometry stream belong to at most one vao
                stream_vao_mapping.emplace(stream_id, vao_id);
                vao_stream_mapping[vao_id].push_back(stream_id);
            } 
            else
                assert(stream_vao_mapping.at(stream_id) == vao_id);

            face_data_t &face_data = stream_data[stream_id];

            for (size_t face_index = 0; face_index < mesh_data.associated_mesh.face_array.size(); ++face_index) // add faces to corresponding geometry streams
            {
                const uint64_t index_offset = face_data.indexes.size();
                const uint64_t vertex_offset = face_data.vertex_data.size();

                auto &tmp_vertex = mesh_data.face_data[face_index].vertex_data;
                face_data.vertex_data.insert(face_data.vertex_data.end(), tmp_vertex.begin(), tmp_vertex.end());

                auto &tmp_index = mesh_data.face_data[face_index].indexes;
                face_data.indexes.insert(face_data.indexes.end(), tmp_index.begin(), tmp_index.end());

                if (auto &face_info = mesh_data.associated_mesh.face_array[face_index].with_shadow_mat)
                {
                    face_info->offset = index_offset;
                    face_info->count = face_data.triangle_count;
                    face_info->base_vertex = vertex_offset;
                    face_info->num_vertices = face_data.vertex_count;
                }
            }
        }

        auto add_to_end = [](uint8_t *from, std::vector<uint8_t> &to, int offset, int size) // copy from input data buffer to output without changes
        {
            const auto new_offset = to.size();
            std::copy(from + offset, from + offset + size, std::back_inserter(to));

            return new_offset;
        };

        auto add_to_end_in_out = [&data_buffer_in, &data_buffer_out](int offset, int size) // copy from input data buffer to output without changes
        {
            return add_to_end(data_buffer_in.data(), data_buffer_out, offset, size);
        };

        add_to_end_in_out(0, 24); // fill header TODO: magic + sizes instead of just copying
        for (auto &light_stream : root_node.controllers.object_param_controller->buffer.light_streams) // copy light streams
        {
            auto &[offset, size] = light_stream.light_offset_size;
            offset = add_to_end_in_out(offset, size);
        }

        // fill vertex file
        {
            auto &[vertex_offset, vertex_size] = aoa.buffer_data.vertex_file_offset_size;
            vertex_offset = data_buffer_out.size();
            if (auto &collision_stream = root_node.controllers.object_param_controller->buffer.collision_stream) { // collision stream
                auto &[offset, size] = collision_stream->vertex_offset_size;
                offset = add_to_end_in_out(offset, size);
            }

            for (auto &[vao_id, stream_ids] : vao_stream_mapping) // filling vao vertex info
            {
                aoa.buffer_data.vaos[vao_id].vertex_format_offset.offset = data_buffer_out.size(); // vao offset
                for (auto stream_id : stream_ids)
                {
                    auto &geometry_stream = root_node.controllers.object_param_controller->buffer.geometry_streams[stream_id];
                    auto &[offset, size] = geometry_stream.vertex_offset_size;
                    size = stream_data[stream_id].vertex_data.size();
                    offset = add_to_end(stream_data[stream_id].vertex_data.data(), data_buffer_out, 0, size);
                }
            }

            vertex_size = data_buffer_out.size() - vertex_offset;
        }

        // fill index file
        {
            auto &[index_offset, index_size] = aoa.buffer_data.index_file_offset_size;
            index_offset = data_buffer_out.size();
            if (auto &collision_stream = root_node.controllers.object_param_controller->buffer.collision_stream) { // collision stream
                auto &[offset, size] = collision_stream->index_offset_size;
                offset = add_to_end_in_out(offset, size);
            }

            for (auto &[vao_id, stream_ids] : vao_stream_mapping) // filling vao index info
            {
                int max_index = -1;
                for (auto stream_id : stream_ids) {
                    auto &stream_indexes = stream_data[stream_id].indexes;
                    if (auto max_iter = std::max_element(stream_indexes.begin(), stream_indexes.end()); max_iter != stream_indexes.end())
                        max_index = std::max(max_index, *max_iter);
                }
                const bool use_short = max_index <= 0xFFFF;
                aoa.buffer_data.vaos[vao_id].vertex_format_offset.format = use_short ? 0xFFFF : 0xFFFFFFFF; // vao index size

                for (auto stream_id : stream_ids)
                {
                    auto &stream_indexes = stream_data[stream_id].indexes;
                    auto &geometry_stream = root_node.controllers.object_param_controller->buffer.geometry_streams[stream_id];
                    auto &[offset, size] = geometry_stream.index_offset_size;
                    size = stream_indexes.size() * (use_short ? 2 : 4);
                    if (use_short)
                    {
                        std::vector<uint16_t> indexes_to_add;
                        std::copy(stream_indexes.begin(), stream_indexes.end(), std::back_inserter(indexes_to_add));
                        offset = add_to_end(reinterpret_cast<uint8_t *>(indexes_to_add.data()), data_buffer_out, 0, size);
                    }
                    else
                    {
                        std::vector<uint32_t> indexes_to_add;
                        std::copy(stream_indexes.begin(), stream_indexes.end(), std::back_inserter(indexes_to_add));
                        offset = add_to_end(reinterpret_cast<uint8_t *>(indexes_to_add.data()), data_buffer_out, 0, size);
                    }
                }
            }

            index_size = data_buffer_out.size() - index_offset;
        }
    }
}