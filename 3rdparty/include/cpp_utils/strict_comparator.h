#pragma once

#include "comparator.h"
#include "type_traits_aux.h"
#include <boost/optional.hpp>

namespace cpp_utils
{

struct strict_comparator_t
{
    template<typename T>
    struct is_simple_type
        : std::integral_constant<bool, 
               
               std::is_fundamental<T>::value 
            || std::is_pointer    <T>::value
            || std::is_enum       <T>::value
        >
    {};

    strict_comparator_t()
    {}

    deque<string> const& path() const
    {
        return path_;
    }

    void append_path(string const& field)
    {
        path_.push_front(field);
    }

    template<typename T>
    typename std::enable_if<is_simple_type<T>::value, bool>::type
    operator()(T a, T b) const
    {
        return a == b;
    }

    template<typename T>
    typename std::enable_if<has_cbegin_cend<T>::value, bool>::type
    operator()(T const &a, T const &b) const
    {
        return boost::equal(a, b, *this);
    }

    template <>
    bool operator()<string>(std::string const& a, std::string const& b) const
    {
        return a == b;
    }

    template<typename T>
    typename std::enable_if<!is_simple_type<T>::value && !has_cbegin_cend<T>::value, bool>::type
    operator()(T const &a, T const &b) const
    {
        return apply_comp_refl(*this, a, b);
    }

    template<typename T>
    bool operator()(boost::optional<T> const &a, boost::optional<T> const &b) const
    {
        if (bool(a) != bool(b))
            return false;

        if (!a)
            return true;

        return operator()(*a, *b);
    }

    template<typename T1, typename T2>
    bool operator()(std::pair<T1, T2> const &a, std::pair<T1, T2> const &b) const
    {
        return operator()(a.first, b.first)
            && operator()(a.second, b.second);
    }

private:
    deque<string> path_;
};

template<typename T>
bool strict_compare(T const &lhs, T const &rhs)
{
    strict_comparator_t comp;
    return comp(lhs, rhs);
}

template<typename T>
optional<string> diff_compare(T const& lhs, T const& rhs, string const& start_name = "")
{
    strict_comparator_t comp;

    auto type_name = typeid(T).name();
    auto name = start_name.empty() ? string(type_name, type_name + strlen(type_name)) : start_name;
    return comp(lhs, rhs) ? none : optional<string>(name + "|" + boost::algorithm::join(comp.path(), "|"));
}

} // namespace cpp_utils