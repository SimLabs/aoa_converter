#pragma once

namespace cpp_utils
{

template<typename T>
std::string const &enum_to_string(T val)
{
    static_assert(std::is_enum<T>::value, "enum_to_string for non-enum type");

    static auto const m = []()
    {
        std::map<T, std::string> result;

        auto const &base = enum_string_match_detail(static_cast<T const *>(nullptr));

        for (auto const &r : base)
            result.insert(r);

        return result;
    }();
    
    return m.at(val);
}

template<typename T>
optional<T> string_to_enum(std::string const &str)
{
    static_assert(std::is_enum<T>::value, "string_to_enum for non-enum type");

    static auto const m = []() 
    {
        std::map<std::string, T> result;

        auto const &base = enum_string_match_detail(static_cast<T const *>(nullptr));

        for (auto const &r : base)
            result.emplace(r.second, r.first);

        return result;
    }();

    auto const it = m.find(str);
    if (it == m.end())
        return none;

    return it->second;
}

} // namespace cpp_utils
