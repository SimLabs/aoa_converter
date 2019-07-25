#pragma once

namespace aurora
{

template<class T>
struct is_leaf_type
{
    enum
    {
        value = (std::is_same_v<std::remove_reference_t<T>, std::string> ||
        std::is_arithmetic_v<std::remove_reference_t<T>>
                 )
    };
};

template<class... T>
struct is_flat_type;

template<class... Args>
struct is_flat_type<std::tuple<Args...>> : std::integral_constant<bool, true>
{};

template<class Type, size_t Size>
struct is_flat_type<std::array<Type, Size>> : std::integral_constant<bool, true>
{};

template<class T>
struct is_flat_type<T>
{
    enum
    {
        value = (std::is_same_v<std::remove_reference_t<T>, aurora::refl::data_buffer::vao_buffer::format_offset> ||
        std::is_same_v<std::remove_reference_t<T>, aurora::refl::data_buffer::vao_buffer::vertex_format::vertex_attribute> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::refl::offset_size> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::refl::format_offset_size> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::refl::node::mesh_t::mesh_face::mesh_face_offset_count_base_mat> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::refl::node::mesh_t::mesh_bbox> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::refl::node::mesh_t::mesh_bsphere> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::refl::node::mesh_t::mesh_vao_ref> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::refl::quoted_string>
                 )
    };
};

template<class T>
constexpr bool is_leaf_type_v = is_leaf_type<T>::value;

template<class T>
constexpr bool is_flat_type_v = is_flat_type<T>::value;

}