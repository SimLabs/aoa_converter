#include "aurora_mesh_subdivider.h"
#include "aurora_aoa_reader.h"
#include "attribute_operations.h"
#include <optional>
#include "geometry/primitives/triangle.h"

namespace aurora {
    mesh_subdivider::vertex_data_t::vertex_data_t(const vertex_attributes_t&& attributes)
        : attributes(std::forward<const vertex_attributes_t>(attributes))
    {}

    mesh_subdivider::vertex_data_t::vertex_data_t(const vertex_format_t &vertex_format, uint8_t const *bytes)
        : attributes(attribute_operations::vertex_attributes_from_bytes(vertex_format, bytes)) 
    {}

    void mesh_subdivider::vertex_data_t::to_byte_buffer(const vertex_format_t &vertex_format, std::vector<size_t> attribute_offsets, std::vector<uint8_t> &byte_buffer) const
    {
        assert(vertex_format.attributes.size() == attribute_offsets.size());
        using namespace attribute_operations;

        for (size_t i = 0; i < vertex_format.attributes.size(); ++i)
        {
            const auto &attribute_format = vertex_format.attributes.at(i);
            const auto current_offset = attribute_offsets.at(i);

            assert(attribute_format.size == (i + 1 == attribute_offsets.size() ? attributes.size() : attribute_offsets.at(i + 1)) - current_offset);

            if (attribute_format.mode == attr_mode_t::ATTR_MODE_PACKED)
                convert_and_add_to_back(
                    attribute_format.type,
                    geom::point_4f{ 
                        attributes.at(current_offset), 
                        attributes.at(current_offset + 1), 
                        attributes.at(current_offset + 2), 
                        attributes.at(current_offset + 3) 
                    },
                    byte_buffer
                );
            else
                for (size_t j = 0; j < attribute_format.size; ++j)
                    convert_and_add_to_back(attribute_format.type, std::forward<const float>(attributes.at(current_offset + j)), byte_buffer);
        }
    }

    mesh_subdivider::geometry_stream_t::geometry_stream_t(data_buffer_t &&data_buffer, aoa_geometry_stream_t &aoa_geometry_stream, aoa_vao_t &aoa_vao)
        : aoa_vao(aoa_vao)
        , aoa_geometry_stream(aoa_geometry_stream)
        , vertex_stride(attribute_operations::vertex_format_stride(aoa_vao.format))
        , attribute_offsets(attribute_operations::vertex_attribute_offsets(aoa_vao.format))
        , coordinate_attribute_offset(attribute_offsets.at(attribute_operations::vertex_coordinate_attribute_index(aoa_vao.format)))
    {
        {
            const uint8_t *vertex_start = data_buffer.data.data()
                + data_buffer.info.vertex_file_offset_size.offset
                + aoa_geometry_stream.vertex_offset_size.offset;

            const auto vertex_count = aoa_geometry_stream.vertex_offset_size.size / vertex_stride;
            vertices.reserve(vertex_count);
            for (size_t i = 0; i < vertex_count; ++i)
                vertices.emplace_back(aoa_vao.format, vertex_start + i * vertex_stride);
        }

        {
            bool short_index = aoa_vao.vertex_format_offset.format <= std::numeric_limits<unsigned short>::max();
            index_stride = short_index ? sizeof(uint16_t) : sizeof(uint32_t);

            const uint8_t *index_start = data_buffer.data.data()
                + data_buffer.info.index_file_offset_size.offset
                + aoa_geometry_stream.index_offset_size.offset;

            const auto index_count = aoa_geometry_stream.index_offset_size.size / index_stride;
            indexes.reserve(index_count);
            for (size_t i = 0; i < index_count; ++i)
                indexes.emplace_back(short_index
                    ? *reinterpret_cast<const uint16_t *>(index_start + i * index_stride)
                    : *reinterpret_cast<const uint32_t *>(index_start + i * index_stride)
                );
        }
    }

    static refl::node &get_node(refl::aurora_format &aoa, std::string &&name)
    {
        const auto compare_case_insensitive = [](auto &&a, auto &&b) noexcept
        {
            return a.size() == b.size()
                && std::equal(a.begin(), a.end(), b.begin(), [](char &c1, char &c2) {
                return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
            });
        };

        for (auto &node : aoa.nodes)
            if (compare_case_insensitive(std::forward<std::string>(name), std::string(node.name)))
                return node;

        throw std::logic_error("Node \"" + name + "\" not found");
    }

    mesh_subdivider::mesh_subdivider(const std::filesystem::path &&aoa_path, float cell_size)
        : aoa(read_aoa(aoa_path.string()))
        , root_node(get_node(aoa, aoa_path.stem().string()))
        , dirname(aoa_path.parent_path())
        , filename(aoa_path.filename())
        , cell_size(cell_size)
    {
        std::ifstream buffer_file(dirname / std::string(aoa.buffer_data.data_buffer_file), std::ios_base::binary);
        std::ostringstream ss;
        ss << buffer_file.rdbuf();
        auto content = ss.str();

        std::vector<uint8_t> data_buffer_in{ content.begin(), content.end() };
        map_data_buffer(data_buffer_in);

        subdivide();
        assemble_data_buffer(data_buffer_in);
    }

    void mesh_subdivider::map_data_buffer(std::vector<uint8_t> &data_buffer_in)
    {
        std::map<unsigned, unsigned> stream_vao_mapping;

        for (auto &nd : aoa.nodes)
            if (auto &mesh = nd.mesh) {
                const auto &[stream_id, vao_id] = mesh->vao_ref;
                if (stream_vao_mapping.find(stream_id) == stream_vao_mapping.end()) { // ensure that each geometry stream belong to at most one vao
                    stream_vao_mapping.emplace(stream_id, vao_id);

                    auto[it, success] = geometry_streams.emplace(
                        stream_id,
                        geometry_stream_t(
                            data_buffer_t{ aoa.buffer_data, data_buffer_in },
                            root_node.controllers.object_param_controller->buffer.geometry_streams.at(stream_id),
                            aoa.buffer_data.vaos.at(vao_id)
                        )
                    );
                    assert(success);
                }
                else
                    assert(stream_vao_mapping.at(stream_id) == vao_id);
            }
    }

    void mesh_subdivider::subdivide()
    {
        std::map<unsigned, std::vector<vertex_index_t>> old_indexes;
        for (auto &[stream_id, stream] : geometry_streams)
            std::swap(old_indexes[stream_id], stream.indexes);

        for (auto &nd : aoa.nodes)
            if (auto &mesh = nd.mesh) {
                const auto stream_id = mesh->vao_ref.geom_stream_id;
                auto &geometry_stream = geometry_streams.at(stream_id);

                for (auto &mesh_face : mesh->face_array)
                    if (auto &face_info = mesh_face.with_shadow_mat)
                        subdivide_mesh_face(
                            face_info.value(),
                            geometry_stream,
                            old_indexes.at(stream_id),
                            face_info->offset - geometry_stream.aoa_geometry_stream.index_offset_size.offset / geometry_stream.index_stride,
                            face_info->base_vertex - geometry_stream.aoa_geometry_stream.vertex_offset_size.offset / geometry_stream.vertex_stride
                        );
            }
    }

    void mesh_subdivider::subdivide_mesh_face(
        mesh_face_data_t &face_info,
        geometry_stream_t &geometry_stream,
        const std::vector<vertex_index_t> &indexes_in,
        const size_t offset,
        const size_t base_vertex
    ) {
        const auto new_offset = geometry_stream.indexes.size();

        for (unsigned i = 0; i < face_info.count; ++i)
            subdivide_triangle(
                geometry_stream, 
                base_vertex + indexes_in.at(offset + 3 * i + 0), 
                base_vertex + indexes_in.at(offset + 3 * i + 1),
                base_vertex + indexes_in.at(offset + 3 * i + 2)
            );

        const auto new_vertices = geometry_stream.indexes.size() - new_offset;
        assert(new_vertices % 3 == 0);

        OSG_DEBUG << face_info.count << " -> " << new_vertices / 3 << " triangles\n";

        face_info.offset = new_offset;
        face_info.base_vertex = 0;
        face_info.num_vertices = new_vertices;
        face_info.count = face_info.num_vertices / 3;
    }
    
    struct weighted_point_t
    {
        float alpha, beta;
        geom::point_3f p;
        std::optional<mesh_subdivider::vertex_index_t> i;
    };

    enum coord_t
    {
        X, Y
    };

    template<coord_t Coord>
    void intersect_half_plane(std::vector<weighted_point_t> &polygon, const float coord, const float sign)
    {
        const static auto get_coord = [](weighted_point_t &p) constexpr
        {
            if constexpr (Coord == X)
                return p.p.x;
            else if (Coord == Y)
                return p.p.y;
        };

        static std::vector<weighted_point_t> tmp;
        tmp.clear();

        for (size_t i = 0; i < polygon.size(); ++i)
        {
            size_t j = (i + 1) % polygon.size();

            auto pi = polygon.at(i);
            auto pj = polygon.at(j);

            if (sign * get_coord(pi) >= sign * coord)
                tmp.push_back(pi);
            if (
                (sign * get_coord(pi) >= sign * coord && sign * coord >= sign * get_coord(pj)) ||
                (sign * get_coord(pj) >= sign * coord && sign * coord >= sign * get_coord(pi))
            ) {
                const float alpha = abs((get_coord(pi) - coord) / (get_coord(pi) - get_coord(pj)));
                tmp.push_back({
                    geom::blend(pi.alpha, pj.alpha, alpha),
                    geom::blend(pi.beta,  pj.beta,  alpha),
                    geom::blend(pi.p,     pj.p,     alpha)
                });
            }
        }

        std::swap(tmp, polygon);
    };

    std::vector<weighted_point_t> intersect_rectangle(std::vector<weighted_point_t> polygon, const geom::rectangle_2f &rectangle)
    {
        intersect_half_plane<X>(polygon, rectangle.lo().x,  1);
        intersect_half_plane<X>(polygon, rectangle.hi().x, -1);
        intersect_half_plane<Y>(polygon, rectangle.lo().y,  1);
        intersect_half_plane<Y>(polygon, rectangle.hi().y, -1);

        return polygon;
    }

    void mesh_subdivider::subdivide_triangle(
        geometry_stream_t &geometry_stream,
        const vertex_index_t ia, const vertex_index_t ib, const vertex_index_t ic
    ) {
        using namespace attribute_operations;

        const std::vector<weighted_point_t> triangle {
            { 1, 0, to_point_3f(geometry_stream.vertices.at(ia).attributes, geometry_stream.coordinate_attribute_offset), ia },
            { 0, 1, to_point_3f(geometry_stream.vertices.at(ib).attributes, geometry_stream.coordinate_attribute_offset), ib },
            { 0, 0, to_point_3f(geometry_stream.vertices.at(ic).attributes, geometry_stream.coordinate_attribute_offset), ic },
        };

        const auto bbox_cells = [&triangle, this]
        {
            float x_min, x_max;
            geom::min_max_3(triangle.at(0).p.x, triangle.at(1).p.x, triangle.at(2).p.x, x_min, x_max);

            float x_min_cell, x_max_cell;
            std::modf(x_min / cell_size - 2, &x_min_cell);
            std::modf(x_max / cell_size + 2, &x_max_cell);
            
            float y_min, y_max;
            geom::min_max_3(triangle.at(0).p.y, triangle.at(1).p.y, triangle.at(2).p.y, y_min, y_max);

            float y_min_cell, y_max_cell;
            std::modf(y_min / cell_size - 2, &y_min_cell);
            std::modf(y_max / cell_size + 2, &y_max_cell);

            return geom::rectangle_2f(
                geom::range_2f(x_min_cell, x_max_cell),
                geom::range_2f(y_min_cell, y_max_cell)
            );
        }();

        for (float x_cell = bbox_cells.lo().x; x_cell <= bbox_cells.hi().x; ++x_cell)
            for (float y_cell = bbox_cells.lo().y; y_cell <= bbox_cells.hi().y; ++y_cell) {
                const auto intersection_points = intersect_rectangle(
                    triangle,
                    geom::rectangle_by_size(
                        geom::point_2f(x_cell, y_cell) * cell_size,
                        geom::point_2f::from_scalar(cell_size)
                    )
                );

                if (intersection_points.size() < 3)
                    continue;

                size_t base_index = -1;
                size_t prev_index = -1;

                for (size_t k = 0; k < intersection_points.size(); ++k)
                {
                    const auto &p = intersection_points.at(k);
                    const vertex_index_t new_index = p.i
                        ? p.i.value()
                        : geometry_stream.vertices.size();

                    if (!p.i)
                        geometry_stream.vertices.emplace_back(
                            interpolate_vertex_attributes(
                                geometry_stream.aoa_vao.format,
                                geometry_stream.attribute_offsets,
                                geometry_stream.vertices.at(ic).attributes,
                                interpolate_vertex_attributes(
                                    geometry_stream.aoa_vao.format,
                                    geometry_stream.attribute_offsets,
                                    geometry_stream.vertices.at(ia).attributes,
                                    geometry_stream.vertices.at(ib).attributes,
                                    p.beta / (p.alpha + p.beta)
                                ),
                                p.alpha + p.beta
                            )
                        );

                    if (k == 0)
                        base_index = new_index;
                    else if (k == 1)
                        prev_index = new_index;
                    else
                    {
                        assert(base_index != -1 && prev_index != -1);
                        geometry_stream.indexes.insert(geometry_stream.indexes.end(), { base_index, prev_index, new_index });
                        prev_index = new_index;
                    }
                }
            }
    }

    void mesh_subdivider::assemble_data_buffer(const std::vector<uint8_t> &data_buffer_in) // filling output data buffer and updating aoa
    {
        std::map<unsigned, std::set<unsigned>> vaos_to_streams;
        for (auto &nd : aoa.nodes)
            if (auto &mesh = nd.mesh) 
            {
                const auto [stream_id, vao_id] = mesh->vao_ref;
                vaos_to_streams[vao_id].insert(stream_id);
            }

        const auto add_to_end = [](const uint8_t *from, std::vector<uint8_t> &to, int offset, int size) // copy from input data buffer to output without changes
        {
            const auto new_offset = to.size();
            std::copy(from + offset, from + offset + size, std::back_inserter(to));

            return new_offset;
        };

        const auto add_to_end_in_out = [this, &add_to_end, &data_buffer_in](int offset, int size) // copy from input data buffer to output without changes
        {
            return add_to_end(data_buffer_in.data(), data_buffer, offset, size);
        };

        // fill header TODO: magic + sizes instead of just copying
        add_to_end_in_out(0, 24);

        // copy light streams TODO: needed?
        for (auto& light_stream : root_node.controllers.object_param_controller->buffer.light_streams)
        {
            auto&[offset, size] = light_stream.light_offset_size;
            offset = add_to_end_in_out(offset, size);
        }

        // fill index file
        auto &[index_offset, index_size] = aoa.buffer_data.index_file_offset_size;
        {
            index_offset = data_buffer.size();

            // collision stream TODO: needed?
            if (auto &collision_stream = root_node.controllers.object_param_controller->buffer.collision_stream) {
                auto &[offset, size] = collision_stream->index_offset_size;
                offset = add_to_end_in_out(offset, size);
            }

            for (auto &[vao_id, stream_ids] : vaos_to_streams)
            {
                // vao index size
                size_t max_index = 0;
                for (auto stream_id : stream_ids) {
                    auto &stream = geometry_streams.at(stream_id);
                    if (auto max_iter = std::max_element(stream.indexes.begin(), stream.indexes.end()); max_iter != stream.indexes.end())
                        max_index = std::max(max_index, *max_iter);
                }
                const bool use_short = max_index <= std::numeric_limits<unsigned short>::max();
                aoa.buffer_data.vaos[vao_id].vertex_format_offset.format = use_short ? std::numeric_limits<unsigned short>::max() : std::numeric_limits<unsigned int>::max();

                // stream index info
                for (auto stream_id : stream_ids)
                {
                    auto &stream = geometry_streams.at(stream_id);
                    auto &[offset, size] = stream.aoa_geometry_stream.index_offset_size;
                    size = stream.indexes.size() * (use_short ? sizeof(uint16_t) : sizeof(uint32_t));
                    if (use_short)
                    {
                        std::vector<uint16_t> indexes_to_add;
                        std::copy(stream.indexes.begin(), stream.indexes.end(), std::back_inserter(indexes_to_add));
                        offset = add_to_end(reinterpret_cast<uint8_t *>(indexes_to_add.data()), data_buffer, 0, size) - index_offset;
                    }
                    else
                    {
                        std::vector<uint32_t> indexes_to_add;
                        std::copy(stream.indexes.begin(), stream.indexes.end(), std::back_inserter(indexes_to_add));
                        offset = add_to_end(reinterpret_cast<uint8_t *>(indexes_to_add.data()), data_buffer, 0, size) - index_offset;
                    }
                }
            }

            index_size = data_buffer.size() - index_offset;
        }

        // fill vertex file
        auto &[vertex_offset, vertex_size] = aoa.buffer_data.vertex_file_offset_size;
        {
            vertex_offset = data_buffer.size();
            
            // collision stream TODO: needed?
            if (auto &collision_stream = root_node.controllers.object_param_controller->buffer.collision_stream) {
                auto &[offset, size] = collision_stream->vertex_offset_size;
                offset = add_to_end_in_out(offset, size);
            }

            for (auto &[vao_id, stream_ids] : vaos_to_streams)
            {
                // vao offset
                auto &vao_info = aoa.buffer_data.vaos[vao_id];
                vao_info.vertex_format_offset.offset = data_buffer.size() - vertex_offset + index_size;

                // stream vertex info
                for (auto stream_id : stream_ids)
                {
                    auto &stream = geometry_streams.at(stream_id);
                    auto &[offset, size] = stream.aoa_geometry_stream.vertex_offset_size;
                    offset = data_buffer.size() - vertex_offset;
                    for (auto &vertex : stream.vertices)
                        vertex.to_byte_buffer(vao_info.format, stream.attribute_offsets, data_buffer);
                    size = data_buffer.size() - vertex_offset - offset;
                }
            }

            vertex_size = data_buffer.size() - vertex_offset;
        }

        // update face info knowing new stream and vao offsets
        for (auto &nd : aoa.nodes)
            if (auto &mesh = nd.mesh)
                for (auto &mesh_face : mesh->face_array)
                    if (auto &face_info = mesh_face.with_shadow_mat)
                    {
                        auto &stream = geometry_streams.at(mesh->vao_ref.geom_stream_id);
                        const bool use_short = stream.aoa_vao.vertex_format_offset.format <= std::numeric_limits<unsigned short>::max();

                        face_info->offset += stream.aoa_geometry_stream.index_offset_size.offset / (use_short ? sizeof(uint16_t) : sizeof(uint32_t));
                        face_info->base_vertex += stream.aoa_geometry_stream.vertex_offset_size.offset / stream.vertex_stride;
                    }
    }

    const refl::aurora_format &mesh_subdivider::processed_aoa() const
    {
        return aoa;
    }

    const std::vector<uint8_t> &mesh_subdivider::processed_aod() const
    {
        return data_buffer;
    }
}
