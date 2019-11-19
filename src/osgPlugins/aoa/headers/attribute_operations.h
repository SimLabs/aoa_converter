#pragma once

#include "aurora_mesh_subdivider.h"
#include "packed.h"

namespace aurora {
    namespace attribute_operations
    {
        using attr_type_t = mesh_subdivider::vertex_format_t::vertex_attribute::type_t;
        using attr_mode_t = mesh_subdivider::vertex_format_t::vertex_attribute::mode_t;

        namespace 
        {
            template<typename T>
            uint32_t get_index(T *data) = delete;

            template<>
            uint32_t get_index<uint16_t>(uint16_t *data)
            {
                const auto index = *data;

                return (index >> 8) | ((index & 0xFF) << 8);
            }

            template<>
            uint32_t get_index<uint32_t>(uint32_t *data)
            {
                const auto index34 = get_index<uint16_t>(reinterpret_cast<uint16_t *>(data));
                const auto index12 = get_index<uint16_t>(reinterpret_cast<uint16_t *>(data) + 1);

                return (index12 << 16) | index34;
            }

            const std::map<attr_type_t, int> type_to_size{
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

                virtual void to_binary(const Outer&& attr, std::vector<uint8_t> &out) const = 0;
            };

            template<attr_type_t Type, typename Inner>
            struct float_attribute_converter : base_attribute_converter<float> {
                float from_binary(uint8_t const* data) const override
                {
                    return *reinterpret_cast<Inner const*>(data);
                }

                void to_binary(const float&& attr, std::vector<uint8_t> &out) const override
                {
                    const size_t prev_size = out.size();
                    out.resize(prev_size + type_to_size.at(Type));
                    *reinterpret_cast<Inner *>(out.data() + prev_size) = Inner(std::forward<const float>(attr));
                }
            };

            template<attr_type_t Type, typename Inner>
            struct int_attribute_converter : base_attribute_converter<float> {
                float from_binary(uint8_t const* data) const override
                {
                    return 1.0f * (*reinterpret_cast<Inner const*>(data)) / Inner(~0);
                }

                void to_binary(const float&& attr, std::vector<uint8_t> &out) const override
                {
                    const size_t prev_size = out.size();
                    out.resize(prev_size + type_to_size.at(Type));
                    *reinterpret_cast<Inner *>(out.data() + prev_size) = Inner(attr * Inner(~0));
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

                void to_binary(const geom::point_4f&& attr, std::vector<uint8_t> &out) const override
                {
                    const size_t prev_size = out.size();
                    out.resize(prev_size + type_to_size.at(Type));
                    Inner out_value = {
                        Inner::Packing(attr.x * Inner::mod_10),
                        Inner::Packing(attr.y * Inner::mod_10),
                        Inner::Packing(attr.z * Inner::mod_10),
                        Inner::Packing(attr.w * Inner::mod_2)
                    };
                    *reinterpret_cast<Inner *>(out.data() + prev_size) = out_value;
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
            > base_attribute_converter<float> *converter()
            {
                static auto converter_ptr = new float_attribute_converter<Type, Inner>;
                return converter_ptr;
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
            > base_attribute_converter<float> *converter()
            {
                static auto converter_ptr = new int_attribute_converter<Type, Inner>;
                return converter_ptr;
            }

            template<
                attr_type_t Type,
                typename Inner,
                std::enable_if_t<
                Type == attr_type_t::UNSIGNED_INT_2_10_10_10_REV ||
                Type == attr_type_t::INT_2_10_10_10_REV,
                int
                > = 0
            > base_attribute_converter<geom::point_4f> *converter()
            {
                static auto converter_ptr = new packed_attribute_converter<Type, Inner>;
                return converter_ptr;
            }

            static_assert(sizeof(half_float) == 2);

            template<typename T>
            constexpr const base_attribute_converter<T> *get_converter(attr_type_t type)
            {
                if constexpr (std::is_same_v<geom::point_4f, T>)
                    switch (type)
                    {
                        case attr_type_t::UNSIGNED_INT_2_10_10_10_REV: return converter<attr_type_t::UNSIGNED_INT_2_10_10_10_REV, u_packed_data_t>();
                        case attr_type_t::         INT_2_10_10_10_REV: return converter<attr_type_t::INT_2_10_10_10_REV, packed_data_t>();
                        default:                                       return nullptr;
                    }
                else {
                    switch (type)
                    {
                        case attr_type_t::                      FLOAT: return converter<attr_type_t::FLOAT, float>();
                        case attr_type_t::                 HALF_FLOAT: return converter<attr_type_t::HALF_FLOAT, half_float>();
                        case attr_type_t::               UNSIGNED_INT: return converter<attr_type_t::UNSIGNED_INT, uint32_t>();
                        case attr_type_t::                        INT: return converter<attr_type_t::INT, int32_t>();
                        case attr_type_t::              UNSIGNED_BYTE: return converter<attr_type_t::UNSIGNED_BYTE, uint8_t>();
                        case attr_type_t::                       BYTE: return converter<attr_type_t::BYTE, int8_t>();
                        default:                                       return nullptr;
                    }
                }
            }
        }

        inline int vertex_format_stride(const mesh_subdivider::vertex_format_t &vertex_format)
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

        inline int vertex_coordinate_attribute_index(const mesh_subdivider::vertex_format_t &vertex_format)
        {
            return 0; // TODO: anything better?
        }

        inline geom::point_3f to_point_3f(const std::vector<float> &attributes, const size_t coordinate_attribute_offset) {
            return geom::point_3f(
                attributes.at(coordinate_attribute_offset + 0),
                attributes.at(coordinate_attribute_offset + 1),
                attributes.at(coordinate_attribute_offset + 2)
            );
        };

        inline mesh_subdivider::vertex_attributes_t vertex_attributes_from_bytes(const mesh_subdivider::vertex_format_t &vertex_format, uint8_t const *bytes)
        {
            std::vector<float> result;

            for (auto &attribute : vertex_format.attributes)
                if (attribute.mode == attr_mode_t::ATTR_MODE_PACKED)
                {
                    assert(attribute.size == 4); // packed 10_10_10_2 should always be vec4

                    const geom::point_4f point4 = get_converter<geom::point_4f>(attribute.type)->from_binary(bytes);
                    result.insert(result.end(), { point4.x, point4.y, point4.z, point4.w });
                    bytes += type_to_size.at(attribute.type);
                }
                else
                    for (size_t i = 0; i < attribute.size; ++i) {
                        result.emplace_back(get_converter<float>(attribute.type)->from_binary(bytes));
                        bytes += type_to_size.at(attribute.type);
                    }

            return result;
        }

        inline mesh_subdivider::vertex_attributes_t interpolate_vertex_attributes(
            const mesh_subdivider::vertex_format_t &vertex_format,
            const std::vector<size_t> &attribute_offsets,
            const mesh_subdivider::vertex_attributes_t &a_attributes,
            const mesh_subdivider::vertex_attributes_t &b_attributes,
            const float alpha
        ) {
            assert(attribute_offsets.size() == vertex_format.attributes.size());

            mesh_subdivider::vertex_attributes_t ab_attributes(a_attributes.size());

            for (size_t i = 0; i < vertex_format.attributes.size(); ++i)
            {
                const auto current_offset = attribute_offsets[i];
                const auto &attribute_format = vertex_format.attributes[i];

                assert(attribute_format.size == (i + 1 == attribute_offsets.size() ? a_attributes.size() : attribute_offsets[i + 1]) - current_offset);
                assert(attribute_format.size == (i + 1 == attribute_offsets.size() ? b_attributes.size() : attribute_offsets[i + 1]) - current_offset);

                if (attribute_format.mode == attr_mode_t::ATTR_MODE_PACKED)
                {
                    // normal should be slerped
                    assert(a_attributes[current_offset + 3] == 0 && b_attributes[current_offset + 3] == 0);

                    geom::point_3f a(a_attributes[current_offset + 0], a_attributes[current_offset + 1], a_attributes[current_offset + 2]);
                    normalize(a);

                    geom::point_3f b(b_attributes[current_offset + 0], b_attributes[current_offset + 1], b_attributes[current_offset + 2]);
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

                    ab_attributes[current_offset + 0] = ab.x;
                    ab_attributes[current_offset + 1] = ab.y;
                    ab_attributes[current_offset + 2] = ab.z;
                    // current_offset + 3 == 0 already (w coordinate)
                }
                else
                    for (size_t j = 0; j < attribute_format.size; ++j)
                        ab_attributes[current_offset + j] = geom::blend(a_attributes[current_offset + j], b_attributes[current_offset + j], alpha);
            }

            return ab_attributes;
        }

        inline std::vector<size_t> vertex_attribute_offsets(const mesh_subdivider::vertex_format_t& vertex_format)
        {
            std::vector<size_t> out = { 0 };

            for (int i = 0; i + 1 < vertex_format.attributes.size(); ++i)
            {
                const auto &attribute = vertex_format.attributes.at(i);
                if (attribute.mode == attr_mode_t::ATTR_MODE_PACKED)
                    assert(vertex_format.attributes.at(i).size == 4);
                out.emplace_back(out.back() + attribute.size);
            }

            return out;
        }

        template<typename T>
        void convert_and_add_to_back(
            const attr_type_t type,
            T&& value,
            std::vector<uint8_t> &byte_buffer
        ) {
            get_converter<std::remove_const_t<T>>(type)->to_binary(std::forward<const T>(value), byte_buffer);
        }
    }
}
