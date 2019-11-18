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
            > std::pair<attr_type_t, base_attribute_converter<float> *> converter_entry()
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
            > std::pair<attr_type_t, base_attribute_converter<float> *> converter_entry()
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
            > std::pair<attr_type_t, base_attribute_converter<geom::point_4f> *> converter_entry()
            {
                return std::make_pair(Type, new packed_attribute_converter<Type, Inner>);
            }

            static_assert(sizeof(half_float) == 2);

            const std::map<attr_type_t, const base_attribute_converter<float> *> float_converters{
                converter_entry<attr_type_t::FLOAT, float>(),
                converter_entry<attr_type_t::HALF_FLOAT, half_float>(),
                converter_entry<attr_type_t::UNSIGNED_INT, uint32_t>(),
                converter_entry<attr_type_t::INT, int32_t>(),
                converter_entry<attr_type_t::UNSIGNED_BYTE, uint8_t>(),
                converter_entry<attr_type_t::BYTE, int8_t>()
            };

            const std::map<attr_type_t, const base_attribute_converter<geom::point_4f> *> point_4f_converters{
                converter_entry<attr_type_t::UNSIGNED_INT_2_10_10_10_REV, u_packed_data_t>(),
                converter_entry<attr_type_t::INT_2_10_10_10_REV,          packed_data_t  >()
            };
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

        inline geom::point_3f to_point_3f(const std::vector<float> &coordinate_attribute) {
            assert(coordinate_attribute.size() == 3);

            return geom::point_3f(
                coordinate_attribute.at(0),
                coordinate_attribute.at(1),
                coordinate_attribute.at(2)
            );
        };

        inline mesh_subdivider::vertex_attributes_t vertex_attributes_from_bytes(const mesh_subdivider::vertex_format_t &vertex_format, uint8_t const *bytes)
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

        inline mesh_subdivider::vertex_attributes_t interpolate_vertex_attributes(
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

        template<typename T>
        void convert_and_add_to_back(
            const attr_type_t type,
            T&& value,
            std::vector<uint8_t> &byte_buffer
        ) {
            using T2 = typename std::remove_const<T>::type;
            static_assert(std::is_same<geom::point_4f, T2>::value || std::is_same<float, T2>::value);

            const base_attribute_converter<T2> *converter = [&] {
                if constexpr (std::is_same_v<float, T2>)
                    return float_converters.at(type);
                else 
                    return point_4f_converters.at(type);
            }();

            auto bytes = converter->to_binary(std::forward<const T>(value));
            std::copy(bytes.begin(), bytes.end(), std::back_inserter(byte_buffer));
        }
    }
}
