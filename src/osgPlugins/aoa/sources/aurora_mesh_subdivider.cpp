#include "aurora_mesh_subdivider.h"
#include "aurora_aoa_reader.h"
#include "packed.h"
#include <optional>

namespace aurora {
    namespace attribute_operations
    {   
        using attr_type_t = mesh_subdivider::vertex_format_t::vertex_attribute::type_t;
        using attr_mode_t = mesh_subdivider::vertex_format_t::vertex_attribute::mode_t;

        template<typename T>
        uint32_t get_index(T *data); // TODO: delete?

        template<>
        uint32_t get_index<uint16_t>(uint16_t *data)
        {
            auto index = *data;

            return (index >> 8) | ((index & 0xFF) << 8);
        }

        template<>
        uint32_t get_index<uint32_t>(uint32_t *data)
        {
            auto index34 = get_index<uint16_t>(reinterpret_cast<uint16_t *>(data));
            auto index12 = get_index<uint16_t>(reinterpret_cast<uint16_t *>(data) + 1);

            return (index12 << 16) | index34;
        }

        const static std::map<attr_type_t, int> type_to_size{
            { attr_type_t::UNSIGNED_INT_2_10_10_10_REV, 4 },
            { attr_type_t::INT_2_10_10_10_REV, 4 },
            { attr_type_t::UNSIGNED_INT, 4 },
            { attr_type_t::INT, 4 },
            { attr_type_t::UNSIGNED_BYTE, 1 },
            { attr_type_t::BYTE, 1 },
            { attr_type_t::FLOAT, 4 },
            { attr_type_t::HALF_FLOAT, 2 }
        };

        template<typename Outer>
        struct base_attribute_converter
        {
            virtual ~base_attribute_converter() = default;
            virtual Outer from_binary(uint8_t const* data) const = 0;

            virtual std::vector<uint8_t> to_binary(const Outer&& attr) const = 0;
        };

        template<attr_type_t Type, typename Inner> 
        struct float_attribute_converter : base_attribute_converter<float> {
            float from_binary(uint8_t const* data) const override
            {
                return *reinterpret_cast<Inner const*>(data);
            }

            std::vector<uint8_t> to_binary(const float&& attr) const override
            {
                std::vector<uint8_t> out(type_to_size.at(Type));
                *reinterpret_cast<Inner *>(out.data()) = Inner(std::forward<const float>(attr));

                return out;
            }
        };

        template<attr_type_t Type, typename Inner>
        struct int_attribute_converter : base_attribute_converter<float> {
            float from_binary(uint8_t const* data) const override
            {
                return 1.0f * (*reinterpret_cast<Inner const*>(data)) / Inner(~0);
            }

            std::vector<uint8_t> to_binary(const float&& attr) const override
            {
                std::vector<uint8_t> out(type_to_size.at(Type));
                *reinterpret_cast<Inner *>(out.data()) = Inner(attr * Inner(~0));

                return out;
            }
        };

#pragma pack(push)
        struct u_packed_data_t
        {
            typedef uint32_t Packing;

            Packing x : 10;
            Packing y : 10;
            Packing z : 10;
            Packing w : 2;

            const static inline float mod_10 = 0b1111111111;
            const static inline float mod_2 = 0b11;
        };
        struct packed_data_t
        {
            typedef int32_t Packing;

            Packing x : 10;
            Packing y : 10;
            Packing z : 10;
            Packing w : 2;

            const static inline float mod_10 = 0b111111111;
            const static inline float mod_2 = 0b1;
        };
#pragma pack(pop)

        template<attr_type_t Type, typename Inner> 
        struct packed_attribute_converter : base_attribute_converter<geom::point_4f> {
            geom::point_4f from_binary(uint8_t const* data) const override
            {
                auto *packed_data = reinterpret_cast<Inner const*>(data);
                return geom::point_4f(packed_data->x / Inner::mod_10, packed_data->y / Inner::mod_10, packed_data->z / Inner::mod_10, packed_data->w / Inner::mod_2);
            }

            std::vector<uint8_t> to_binary(const geom::point_4f&& attr) const override
            {
                std::vector<uint8_t> out(type_to_size.at(Type));
                Inner out_value = {
                    Inner::Packing(attr.x * Inner::mod_10),
                    Inner::Packing(attr.y * Inner::mod_10),
                    Inner::Packing(attr.z * Inner::mod_10),
                    Inner::Packing(attr.w * Inner::mod_2)
                };
                *reinterpret_cast<Inner *>(out.data()) = out_value;

                return out;
            }
        };

        template<
            attr_type_t Type, 
            typename Inner, 
            std::enable_if_t<
                Type == attr_type_t::FLOAT || 
                Type == attr_type_t::HALF_FLOAT,    
                int
            > = 0
        > static std::pair<attr_type_t, base_attribute_converter<float> *> converter_entry()
        {
            return std::make_pair(Type, new float_attribute_converter<Type, Inner>);
        }

        template<
            attr_type_t Type, 
            typename Inner,
            std::enable_if_t<
                Type == attr_type_t::UNSIGNED_INT || 
                Type == attr_type_t::INT || 
                Type == attr_type_t::UNSIGNED_BYTE || 
                Type == attr_type_t::BYTE,
                int
            > = 0
        > static std::pair<attr_type_t, base_attribute_converter<float> *> converter_entry()
        {
            return std::make_pair(Type, new int_attribute_converter<Type, Inner>);
        }

        template<
            attr_type_t Type, 
            typename Inner, 
            std::enable_if_t<
                Type == attr_type_t::UNSIGNED_INT_2_10_10_10_REV || 
                Type == attr_type_t::INT_2_10_10_10_REV,
                int
            > = 0
        > static std::pair<attr_type_t, base_attribute_converter<geom::point_4f> *> converter_entry()
        {
            return std::make_pair(Type, new packed_attribute_converter<Type, Inner>);
        }

        static_assert(sizeof(half_float) == 2);

        const static std::map<attr_type_t, const base_attribute_converter<float> *> float_converters {
            converter_entry<attr_type_t::FLOAT, float>(),
            converter_entry<attr_type_t::HALF_FLOAT, half_float>(),
            converter_entry<attr_type_t::UNSIGNED_INT, uint32_t>(),
            converter_entry<attr_type_t::INT, int32_t>(),
            converter_entry<attr_type_t::UNSIGNED_BYTE, uint8_t>(),
            converter_entry<attr_type_t::BYTE, int8_t>()
        };

        const static std::map<attr_type_t, const base_attribute_converter<geom::point_4f> *> point_4f_converters {
            converter_entry<attr_type_t::UNSIGNED_INT_2_10_10_10_REV, u_packed_data_t>(),
            converter_entry<attr_type_t::INT_2_10_10_10_REV,          packed_data_t  >()
        };

        int vertex_format_stride(const mesh_subdivider::vertex_format_t &vertex_format)
        {
            int total = 0;
            for (auto &attribute : vertex_format.attributes)
                if (attribute.mode == attr_mode_t::ATTR_MODE_PACKED)
                {
                    assert(attribute.size == 4);
                    total += type_to_size.at(attribute.type);
                }
                else
                    total += attribute.size * type_to_size.at(attribute.type);

            return total;
        }

        int vertex_coordinate_attribute_index(const mesh_subdivider::vertex_format_t &vertex_format)
        {
            return 0; // TODO: anything better?
        }

        mesh_subdivider::vertex_attributes_t vertex_attributes_from_bytes(const mesh_subdivider::vertex_format_t &vertex_format, uint8_t const *bytes)
        {
            std::vector<std::vector<float>> result;

            for (auto &attribute : vertex_format.attributes)
            {
                if (attribute.mode == attr_mode_t::ATTR_MODE_PACKED)
                {
                    assert(attribute.size == 4); // packed 10_10_10_2 should always be vec4

                    geom::point_4f point4 = point_4f_converters.at(attribute.type)->from_binary(bytes);
                    result.push_back({ point4.x, point4.y, point4.z, point4.w });
                    bytes += type_to_size.at(attribute.type);
                }
                else
                {
                    result.emplace_back(attribute.size);
                    for (size_t i = 0; i < attribute.size; ++i) {
                        result.back().at(i) = float_converters.at(attribute.type)->from_binary(bytes);
                        bytes += type_to_size.at(attribute.type);
                    }
                }
            }

            return result;
        }


        geom::point_3f to_point_3f(const std::vector<float> &coordinate_attribute) {
            assert(coordinate_attribute.size() == 3);

            return geom::point_3f(
                coordinate_attribute.at(0),
                coordinate_attribute.at(1),
                coordinate_attribute.at(2)
            );
        };

        mesh_subdivider::vertex_attributes_t interpolate_vertex_attributes(
            const mesh_subdivider::vertex_format_t &vertex_format,
            const mesh_subdivider::vertex_attributes_t &a_attributes, 
            const mesh_subdivider::vertex_attributes_t &b_attributes,
            const float alpha
        ) {
            assert(a_attributes.size() == b_attributes.size() && b_attributes.size() == vertex_format.attributes.size());

            mesh_subdivider::vertex_attributes_t ab_attributes;
            
            for (size_t i = 0; i < vertex_format.attributes.size(); ++i)
            {
                const auto &a_attribute = a_attributes.at(i);
                const auto &b_attribute = b_attributes.at(i);
                const auto &attribute = vertex_format.attributes.at(i);

                assert(a_attribute.size() == attribute.size && b_attribute.size() == attribute.size);

                ab_attributes.emplace_back(attribute.size);
                if (attribute.mode == attr_mode_t::ATTR_MODE_PACKED)
                {
                    // normal should be slerped
                    assert(a_attribute.at(3) == 0 && b_attribute.at(3) == 0);
                    
                    geom::point_3f a(a_attribute.at(0), a_attribute.at(1), a_attribute.at(2));
                    normalize(a);

                    geom::point_3f b(b_attribute.at(0), b_attribute.at(1), b_attribute.at(2));
                    normalize(b);

                    geom::point_3f ab;

                    float dot = a.x * b.x + a.y * b.y + a.z * b.z;

                    if (abs(dot) < 0.99)
                    {
                        const auto angle = acos(dot);
                        const auto sin_angle = sin(angle);
                        const auto sin_angle_alpha = sin(angle * alpha);
                        const auto sin_angle_inv_alpha = sin(angle * (1 - alpha));

                        ab = (a * sin_angle_inv_alpha + b * sin_angle_alpha) / sin_angle;
                    }
                    else
                        ab = geom::blend(a, b, alpha);
                    normalize(ab);

                    ab_attributes.back() = { ab.x, ab.y, ab.z, 0 };

                }
                else
                    for (size_t j = 0; j < attribute.size; ++j)
                        ab_attributes.back().at(j) = geom::blend(a_attribute.at(j), b_attribute.at(j), alpha);
            }
                
            return ab_attributes;
        }
    }

    template<typename T>
    void convert_and_add_to_back(
        const attribute_operations::base_attribute_converter<T> *converter, 
        const T&& value, 
        std::vector<uint8_t> &byte_buffer
    ) {
        auto bytes = converter->to_binary(std::forward<const T>(value));
        std::copy(bytes.begin(), bytes.end(), std::back_inserter(byte_buffer));
    }

    mesh_subdivider::vertex_data_t::vertex_data_t(const vertex_attributes_t&& attributes)
        : attributes(std::forward<const vertex_attributes_t>(attributes))
    {}


    mesh_subdivider::vertex_data_t::vertex_data_t(const vertex_format_t &vertex_format, uint8_t const *bytes)
        : attributes(attribute_operations::vertex_attributes_from_bytes(vertex_format, bytes)) 
    {}

    void mesh_subdivider::vertex_data_t::to_byte_buffer(const vertex_format_t &vertex_format, std::vector<uint8_t> &byte_buffer) const
    {
        assert(vertex_format.attributes.size() == attributes.size());
        using namespace attribute_operations;

        for (size_t i = 0; i < vertex_format.attributes.size(); ++i)
        {
            const auto &attribute_format = vertex_format.attributes.at(i);
            const auto &attribute_value = attributes.at(i);

            assert(attribute_format.size == attribute_value.size());

            if (attribute_format.mode == attr_mode_t::ATTR_MODE_PACKED)
                convert_and_add_to_back(
                    point_4f_converters.at(attribute_format.type),
                    geom::point_4f{ attribute_value.at(0), attribute_value.at(1), attribute_value.at(2), attribute_value.at(3) },
                    byte_buffer
                );
            else
                for (size_t j = 0; j < attribute_format.size; ++j)
                    convert_and_add_to_back(float_converters.at(attribute_format.type), std::forward<const float>(attribute_value.at(j)), byte_buffer);
        }
    }

    mesh_subdivider::geometry_stream_t::geometry_stream_t(data_buffer_t &&data_buffer, aoa_geometry_stream_t &aoa_geometry_stream, aoa_vao_t &aoa_vao)
        : aoa_vao(aoa_vao)
        , aoa_geometry_stream(aoa_geometry_stream)
        , vertex_stride(attribute_operations::vertex_format_stride(aoa_vao.format))
        , coordinate_attribute(attribute_operations::vertex_coordinate_attribute_index(aoa_vao.format))
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

                split_vertex_cache.clear();
                
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
    
    void mesh_subdivider::subdivide_triangle(
        geometry_stream_t &geometry_stream,
        const vertex_index_t ia0, const vertex_index_t ib0, const vertex_index_t ic0
    ) {
        using namespace attribute_operations;

        std::deque<std::tuple<vertex_index_t, vertex_index_t, vertex_index_t>> q = { { ia0, ib0, ic0 } };

        struct indexed_point
        {
            vertex_index_t i;
            geom::point_3f p;

            indexed_point(vertex_index_t i, geometry_stream_t &geometry_stream)
                : i(i)
                , p(to_point_3f(geometry_stream.vertices.at(i).attributes.at(geometry_stream.coordinate_attribute)))
            {}
        };

        while (!q.empty())
        {
            indexed_point a(std::get<0>(q.front()), geometry_stream);
            indexed_point b(std::get<1>(q.front()), geometry_stream);
            indexed_point c(std::get<2>(q.front()), geometry_stream);
            q.pop_front();

            const auto rotate_ccw = [&a, &b, &c]()
            {
                std::swap(a, b);
                std::swap(c, b);
            };

            const auto try_split = [&, this](auto coord, const bool is_min)
            {            
                while (is_min ? coord(a) > std::min(coord(b), coord(c)) : coord(a) < std::max(coord(b), coord(c)))
                    rotate_ccw();
                                    
                std::optional<float> ab = get_split_point(coord(a), coord(b));
                std::optional<float> ac = get_split_point(coord(a), coord(c));
                                                                               
                if (ab && ac)
                {
                    const float split_point = is_min ? std::min(ab.value(), ac.value()) : std::max(ab.value(), ac.value());
                    const vertex_index_t iab = split_vertex(geometry_stream, a.i, b.i, (split_point - coord(a)) / (coord(b) - coord(a)));
                    const vertex_index_t iac = split_vertex(geometry_stream, a.i, c.i, (split_point - coord(a)) / (coord(c) - coord(a)));

                    q.insert(q.end(), {
                        { a.i, iab, iac },
                        { iab, b.i, c.i },
                        { iab, c.i, iac }
                    });

                    return true;
                }

                return false;
            };

            if (!(
                try_split([](indexed_point &p) { return p.p.x; }, false) 
                || try_split([](indexed_point &p) { return p.p.x; }, true) 
                || try_split([](indexed_point &p) { return p.p.y; }, false) 
                || try_split([](indexed_point &p) { return p.p.y; }, true)
            ))
                geometry_stream.indexes.insert(geometry_stream.indexes.end(), { a.i, b.i, c.i });
        }
    }

    mesh_subdivider::vertex_index_t mesh_subdivider::split_vertex(
        geometry_stream_t &geometry_stream, const vertex_index_t ia, const vertex_index_t ib, const float alpha
    ) {
        if (const auto it = split_vertex_cache.find({ ia, ib, alpha }); it != split_vertex_cache.end())
            return it->second;

        const auto index = geometry_stream.vertices.size();
        auto ab = attribute_operations::interpolate_vertex_attributes(
            geometry_stream.aoa_vao.format,
            geometry_stream.vertices.at(ia).attributes,
            geometry_stream.vertices.at(ib).attributes,
            alpha
        );
        geometry_stream.vertices.emplace_back(std::move(ab));

        split_vertex_cache.emplace(std::make_tuple(ia, ib, alpha), index);
        return index;
    };

    const static float EPS = 1e-3f;

    std::optional<float> mesh_subdivider::get_split_point(float a, float b)
    {
        if (abs(a - b) <= EPS)
            return {};

        bool flip = false;
        if (a > b)
        {
            std::swap(a, b);
            flip = true;
        }

        float a_cell, b_cell;
        std::modf((a + EPS) / cell_size, &a_cell);
        std::modf((b - EPS) / cell_size, &b_cell);

        if (a_cell < b_cell || a < -EPS && b > EPS)
        {
            const float ab = flip
                ? (a < -EPS ? a_cell : a_cell + 1) * cell_size
                : (b < -EPS ? b_cell - 1 : b_cell) * cell_size;

            const float alpha = (ab - a) / (b - a);

            if (EPS < alpha && alpha < 1 - EPS)
                return ab;
        }

        OSG_DEBUG << "Not splitting " << a << " & " << b << ": " << a_cell << " & " << b_cell << (flip ? " [FLIP]" : "") << std::endl;

        return {};
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
                        vertex.to_byte_buffer(vao_info.format, data_buffer);
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
