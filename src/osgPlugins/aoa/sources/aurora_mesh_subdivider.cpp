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

            Packing w : 2;
            Packing z : 10;
            Packing y : 10;
            Packing x : 10;

            const static inline float mod_10 = 0b1111111111;
            const static inline float mod_2 = 0b11;
        };
        struct packed_data_t
        {
            typedef int32_t Packing;
            
            Packing w : 2;
            Packing z : 10;
            Packing y : 10;
            Packing x : 10;

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
                    Inner::Packing(attr.w * Inner::mod_2), 
                    Inner::Packing(attr.z * Inner::mod_10), 
                    Inner::Packing(attr.y * Inner::mod_10), 
                    Inner::Packing(attr.x * Inner::mod_10)
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
                if (false && attribute.mode == attr_mode_t::ATTR_MODE_PACKED) // TODO: fix
                {
                    geom::point_4f a(a_attribute.at(0), a_attribute.at(1), a_attribute.at(2), a_attribute.at(3));
                    normalize(a);

                    geom::point_4f b(b_attribute.at(0), b_attribute.at(1), b_attribute.at(2), b_attribute.at(3));
                    normalize(b);

                    geom::point_4f ab;

                    float dot = a.x * b.x + a.y * b.y + a.z * b.z;

                    /*
                    dot = cos(theta)
                    if (dot < 0), v1 and v2 are more than 90 degrees apart,
                    so we can invert one to reduce spinning
                    */

                    if (dot < 0.95)
                    {
                        const auto angle = acos(dot);
                        const auto sin_angle = sin(angle);
                        const auto sin_angle_alpha = sin(angle * alpha);
                        const auto sin_angle_inv_alpha = sin(angle * (1 - alpha));

                        ab = (a * sin_angle_inv_alpha + b * sin_angle_alpha) / sin_angle;
                    }
                    else
                        // if the angle is small, use linear interpolation
                        ab = geom::blend(a, b, alpha);
                    normalize(ab);

                    ab_attributes.back() = { ab.x, ab.y, ab.z, ab.w };

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

        data_buffer_in = { content.begin(), content.end() };

        subdivide();
        fill_processed_data();
    }

    void mesh_subdivider::subdivide()
    {
        std::map<unsigned, unsigned> vao_vertex_stride;
        std::map<unsigned, unsigned> vao_vertex_count;
        for (auto& nd : aoa.nodes)
            if (auto& mesh = nd.mesh) {
                const auto [stream_id, vao_id] = mesh->vao_ref;
                const auto &vao = aoa.buffer_data.vaos.at(vao_id);
                const auto &stream = get_geometry_stream(stream_id);
                const auto vertex_stride = attribute_operations::vertex_format_stride(vao.format);
                vao_vertex_stride[vao_id] = vertex_stride;
                vao_vertex_count[vao_id] += stream.vertex_offset_size.size / vertex_stride;
            }

        for (auto &nd : aoa.nodes)
            if (auto &mesh = nd.mesh) {
                const auto vao_ref = mesh->vao_ref;
                const auto &vao = aoa.buffer_data.vaos.at(vao_ref.vao_id);
                const auto &stream = get_geometry_stream(vao_ref.geom_stream_id);

                const auto &vertex_format = vao.format;
                const auto vertex_stride = vao_vertex_stride.at(vao_ref.vao_id);

                vertex_index_data_t vertex_index_data;
                
                const auto& vao_offset = vao.vertex_format_offset.offset;
                {
                    const uint8_t* vertex_start = data_buffer_in.data()
                        + aoa.buffer_data.vertex_file_offset_size.offset
                        + vao_offset;
                    const auto vertex_count = vao_vertex_count.at(vao_ref.vao_id);

                    vertex_index_data.vertices.reserve(vertex_count);
                    for (size_t i = 0; i < vertex_count; ++i)
                        vertex_index_data.vertices.emplace_back(vertex_format, vertex_start + i * vertex_stride);
                }

                std::vector<mesh_face_data_t> face_data;

                split_vertex_cache.clear();
                for (auto &mesh_face : mesh->face_array) {
                    if (auto &face_info = mesh_face.with_shadow_mat)
                        face_data.push_back(subdivide_mesh_face(vao, stream, vertex_stride, face_info.value(), vertex_index_data));
                }

                data_cache.insert({
                    { mesh->vao_ref.geom_stream_id, mesh->vao_ref.vao_id },
                    { mesh.value(), std::move(vertex_index_data), std::move(face_data) }
                });
            }
    }

    mesh_subdivider::mesh_face_data_t mesh_subdivider::subdivide_mesh_face(
        const refl::data_buffer::vao_buffer &vao, 
        const geometry_stream_t &stream,
        const unsigned vertex_stride, 
        const mesh_face_data_t &face_info,
        vertex_index_data_t &mesh_vertex_index_data
    ) {
        mesh_face_data_t output_face_info = face_info;

        const auto &vertex_format = vao.format;
        const auto is_short_index = vao.vertex_format_offset.format == 0xFFFF;

        const auto index_size = is_short_index ? 2 : 4;
        const auto indexes_begin = data_buffer_in.data() + aoa.buffer_data.index_file_offset_size.offset
            + stream.index_offset_size.offset + index_size * face_info.offset;
        const auto base_vertex = face_info.base_vertex - stream.vertex_offset_size.offset / vertex_stride;
        
        const auto prev_index_count = mesh_vertex_index_data.indexes.size();

        const auto subdivide_face_triangles = [&, this](const auto indexes_in_begin)
        {
            for (unsigned i = 0; i < face_info.count; ++i)
                subdivide_triangle(
                    vertex_format, mesh_vertex_index_data, base_vertex, 
                    indexes_in_begin[3 * i + 0],
                    indexes_in_begin[3 * i + 1],
                    indexes_in_begin[3 * i + 2]
                );
        };

        if (is_short_index)
            subdivide_face_triangles(reinterpret_cast<uint16_t *>(indexes_begin));
        else
            subdivide_face_triangles(reinterpret_cast<uint32_t *>(indexes_begin));

        output_face_info.offset = prev_index_count;
        output_face_info.base_vertex = base_vertex;
        output_face_info.num_vertices = mesh_vertex_index_data.indexes.size() - prev_index_count;
        assert(output_face_info.num_vertices % 3 == 0);
        output_face_info.count = output_face_info.num_vertices / 3;

        OSG_DEBUG << face_info.count << " -> " << output_face_info.count << " triangles\n";

        return output_face_info;
    }
    
    void mesh_subdivider::subdivide_triangle(
        const vertex_format_t &vertex_format, 
        vertex_index_data_t &face_data,
        const vertex_index_t base_vertex,
        const vertex_index_t ia0, const vertex_index_t ib0, const vertex_index_t ic0
    ) {
        using namespace attribute_operations;

        const auto vertex_coordinate_index = vertex_coordinate_attribute_index(vertex_format);

        std::deque<std::tuple<vertex_index_t, vertex_index_t, vertex_index_t>> q = { { ia0, ib0, ic0 } };

        while (!q.empty())
        {
            struct indexed_point_t
            {
                vertex_index_t i_local, i_global;
                geom::point_3f p;

                indexed_point_t(vertex_index_t i, vertex_index_t base_vertex, vertex_index_data_t &face_data, const int vertex_coordinate_index)
                    : i_local(i)
                    , i_global(i + base_vertex)
                    , p(to_point_3f(face_data.vertices.at(i_global).attributes.at(vertex_coordinate_index)))
                {}
            };

            indexed_point_t a(std::get<0>(q.front()), base_vertex, face_data, vertex_coordinate_index);
            indexed_point_t b(std::get<1>(q.front()), base_vertex, face_data, vertex_coordinate_index);
            indexed_point_t c(std::get<2>(q.front()), base_vertex, face_data, vertex_coordinate_index);
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
                    const vertex_index_t iab = split_vertex(
                        vertex_format, face_data, a.i_global, b.i_global, (split_point - coord(a)) / (coord(b) - coord(a))
                    ) - base_vertex;
                    const vertex_index_t iac = split_vertex(
                        vertex_format, face_data, a.i_global, c.i_global, (split_point - coord(a)) / (coord(c) - coord(a))
                    ) - base_vertex;

                    q.insert(q.end(), {
                        { a.i_local, iab, iac },
                        { iab, b.i_local, c.i_local },
                        { iab, c.i_local, iac }
                    });

                    return true;
                }

                return false;
            };

            if (!(
                try_split([](indexed_point_t &p) { return p.p.x; }, false) || 
                try_split([](indexed_point_t &p) { return p.p.x; }, true) ||
                try_split([](indexed_point_t &p) { return p.p.y; }, false) ||
                try_split([](indexed_point_t &p) { return p.p.y; }, true)
            ))
                face_data.indexes.insert(face_data.indexes.end(), { a.i_local, b.i_local, c.i_local });
        }
    }

    mesh_subdivider::vertex_index_t mesh_subdivider::split_vertex(
        const vertex_format_t &vertex_format, vertex_index_data_t &face_data,
        const vertex_index_t ia, const vertex_index_t ib, const float alpha)
    {
        if (const auto it = split_vertex_cache.find({ ia, ib, alpha }); it != split_vertex_cache.end())
            return it->second;

        const auto index = face_data.vertices.size();
        auto ab = attribute_operations::interpolate_vertex_attributes(
            vertex_format,
            face_data.vertices.at(ia).attributes,
            face_data.vertices.at(ib).attributes,
            alpha
        );
        face_data.vertices.emplace_back(std::move(ab));

        split_vertex_cache.emplace(std::make_tuple(ia, ib, alpha), index);
        return index;
    };

    const static float EPS = 1e-3f;

    std::optional<float> mesh_subdivider::get_split_point(float a, float b) const
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

    void mesh_subdivider::fill_processed_data() // filling output data buffer and updating aoa
    {
        std::map<unsigned, unsigned> stream_vao_mapping;
        std::map<unsigned, std::set<unsigned>> vao_stream_mapping;
        std::map<unsigned, vertex_index_data_t> stream_data;
        
        for (auto &[stream_vao_id, mesh_data] : data_cache)
        {
            auto &[stream_id, vao_id] = stream_vao_id;
            if (stream_vao_mapping.find(stream_id) == stream_vao_mapping.end()) { // ensure that each geometry stream belong to at most one vao
                stream_vao_mapping.emplace(stream_id, vao_id);
                vao_stream_mapping[vao_id].insert(stream_id); // create new if needed
            } 
            else
                assert(stream_vao_mapping.at(stream_id) == vao_id);

            vertex_index_data_t &vertex_index_data = stream_data[stream_id]; // create new if needed

            auto &[mesh_info, mesh_vertex_index_data, mesh_face_info] = mesh_data;

            const uint64_t vertex_offset = vertex_index_data.vertices.size();

            if (auto &tmp_vertex = mesh_vertex_index_data.vertices; !tmp_vertex.empty())
                std::copy(tmp_vertex.begin(), tmp_vertex.end(), std::back_inserter(vertex_index_data.vertices));

            if (auto &tmp_index = mesh_vertex_index_data.indexes; !tmp_index.empty())
                std::copy(tmp_index.begin(), tmp_index.end(), std::back_inserter(vertex_index_data.indexes));

            for (auto &face_info : mesh_face_info)
                face_info.base_vertex += vertex_offset; // later add stream offset so we actually count from VAO start
        }

        auto add_to_end = [](uint8_t *from, std::vector<uint8_t> &to, int offset, int size) // copy from input data buffer to output without changes
        {
            const auto new_offset = to.size();
            std::copy(from + offset, from + offset + size, std::back_inserter(to));

            return new_offset;
        };

        auto add_to_end_in_out = [this, &add_to_end](int offset, int size) // copy from input data buffer to output without changes
        {
            return add_to_end(data_buffer_in.data(), data_buffer_out, offset, size);
        };

        add_to_end_in_out(0, 24); // fill header TODO: magic + sizes instead of just copying

        for (auto& light_stream : root_node.controllers.object_param_controller->buffer.light_streams) // copy light streams TODO: needed?
        {
            auto&[offset, size] = light_stream.light_offset_size;
            offset = add_to_end_in_out(offset, size);
        }

        // fill index file
        auto &[index_offset, index_size] = aoa.buffer_data.index_file_offset_size;
        {
            index_offset = data_buffer_out.size();
            if (auto &collision_stream = root_node.controllers.object_param_controller->buffer.collision_stream) { // collision stream TODO: needed?
                auto &[offset, size] = collision_stream->index_offset_size;
                offset = add_to_end_in_out(offset, size);
            }

            for (auto &[vao_id, stream_ids] : vao_stream_mapping) // filling vao index info
            {
                int max_index = -1;
                for (auto stream_id : stream_ids) {
                    auto &stream_indexes = stream_data.at(stream_id).indexes;
                    if (auto max_iter = std::max_element(stream_indexes.begin(), stream_indexes.end()); max_iter != stream_indexes.end())
                        max_index = std::max(max_index, *max_iter);
                }
                const bool use_short = max_index <= 0xFFFF;
                aoa.buffer_data.vaos[vao_id].vertex_format_offset.format = use_short ? 0xFFFF : 0xFFFFFFFF; // vao index size

                assert(stream_ids.size() == *stream_ids.rbegin() + 1);
                for (auto stream_id : stream_ids)
                {
                    auto &stream_indexes = stream_data[stream_id].indexes;
                    auto &geometry_stream = get_geometry_stream(stream_id);
                    auto &[offset, size] = geometry_stream.index_offset_size;
                    size = stream_indexes.size() * (use_short ? 2 : 4);
                    if (use_short)
                    {
                        std::vector<uint16_t> indexes_to_add;
                        std::copy(stream_indexes.begin(), stream_indexes.end(), std::back_inserter(indexes_to_add));
                        offset = add_to_end(reinterpret_cast<uint8_t *>(indexes_to_add.data()), data_buffer_out, 0, size) - index_offset;
                    }
                    else
                    {
                        std::vector<uint32_t> indexes_to_add;
                        std::copy(stream_indexes.begin(), stream_indexes.end(), std::back_inserter(indexes_to_add));
                        offset = add_to_end(reinterpret_cast<uint8_t *>(indexes_to_add.data()), data_buffer_out, 0, size) - index_offset;
                    }
                }
            }

            index_size = data_buffer_out.size() - index_offset;
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
                auto &vao_info = aoa.buffer_data.vaos[vao_id];

                vao_info.vertex_format_offset.offset = data_buffer_out.size() - vertex_offset; // vao offset
                for (auto stream_id : stream_ids)
                {
                    auto &geometry_stream = get_geometry_stream(stream_id);
                    auto &[offset, size] = geometry_stream.vertex_offset_size;
                    offset = data_buffer_out.size() - vertex_offset;
                    for (auto &vertex : stream_data.at(stream_id).vertices)
                        vertex.to_byte_buffer(vao_info.format, data_buffer_out);
                    size = data_buffer_out.size() - vertex_offset - offset;
                }
            }

            vertex_size = data_buffer_out.size() - vertex_offset;
        }

        for (auto& [stream_vao_id, mesh_data] : data_cache) // final mesh face infos
        {
            auto& [stream_id, vao_id] = stream_vao_id;

            const auto& vao = aoa.buffer_data.vaos.at(vao_id);
            const auto &stream = get_geometry_stream(stream_id);

            const auto vertex_stride = attribute_operations::vertex_format_stride(vao.format);
            
            auto &[mesh_info, mesh_vertex_index_data, mesh_face_info] = mesh_data;

            unsigned face_index = 0;
            for (auto& face : mesh_info.face_array)
                if (auto& face_info = face.with_shadow_mat) {
                    face_info = mesh_face_info.at(face_index);
                    face_info->base_vertex += stream.vertex_offset_size.offset / vertex_stride;
                    
                    face_index++;
                }
        }
    }

    const refl::aurora_format &mesh_subdivider::processed_aoa() const
    {
        return aoa;
    }

    const std::vector<uint8_t> &mesh_subdivider::processed_aod() const
    {
        return data_buffer_out;
    }

    mesh_subdivider::geometry_stream_t& mesh_subdivider::get_geometry_stream(unsigned stream_id) const
    {
        return root_node.controllers.object_param_controller->buffer.geometry_streams.at(stream_id);
    }
}
