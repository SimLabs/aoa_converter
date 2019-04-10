#pragma once

#include <type_traits>

namespace cpp_utils
{
namespace details
{

template<size_t I, class Func, class Tuple, class... Args>
auto apply_impl(Func&& f, Tuple&& /* t */, Args&&... args)
    -> typename std::enable_if<I == 0, typename std::decay<Func>::type::result_type>::type
{
    return std::forward<Func>(f)(std::forward<Args>(args)...);
}

template<size_t I, class Func, class Tuple, class... Args>
auto apply_impl(Func&& f, Tuple&& t, Args&&... args)
    -> typename std::enable_if<I != 0, typename std::decay<Func>::type::result_type>::type
{
    return apply_impl<I - 1>(std::forward<Func>(f), std::forward<Tuple>(t), std::get<I - 1>(t), std::forward<Args>(args)...);
}

}

template<class Func, class Tuple>
auto apply(Func&& f, Tuple&& t)
    -> typename std::decay<Func>::type::result_type
{
    const size_t size = std::tuple_size< std::decay<Tuple>::type >::value;
    return details::apply_impl<size>(std::forward<Func>(f), std::forward<Tuple>(t));
}

} // cpp_utils
