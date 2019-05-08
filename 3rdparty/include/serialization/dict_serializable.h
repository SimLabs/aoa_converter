#pragma once

#include "dict_fwd.h"

namespace dict
{
    
struct serializable
{
    virtual ~serializable() {};

    virtual void write_to_dict(dict_t &dst) const = 0;
};


namespace type_traits
{
    template<typename T>
    constexpr bool is_serializable_v = std::is_base_of_v<serializable, T>;
    
    template<typename T>
    struct is_serializable_ptr
        : std::integral_constant<bool, false>
    {};

    template<typename T>
    struct is_serializable_ptr<std::shared_ptr<T>>
        : std::integral_constant<bool, is_serializable_v<T>>
    {};

    template<typename T>
    constexpr bool is_serializable_ptr_v = is_serializable_ptr<T>::value;

} // namespace type_traits

} // namespace dict