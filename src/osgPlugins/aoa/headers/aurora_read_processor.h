#pragma once
#include "serialization/dict.h"
#include "aurora_format.h"
#include "aurora_processor_traits.h"
#include "cpp_utils/enum_to_string.h"
#include "tree.h"

namespace aurora
{

using dict_t = ::tree;

namespace detail
{

struct flat_read_processor
{
    flat_read_processor(std::string buf)
        : in_(move(buf))
    {}

    template<class Type>
    void operator()(Type& value, const char* key, std::enable_if_t<is_leaf_type_v<Type>>* = nullptr)
    {
        in_ >> value;
    }

    template<class Type>
    void operator()(Type& value, const char* key, std::enable_if_t<std::is_enum_v<Type>>* = nullptr)
    {
        std::string val;
        in_ >> val;
        value = *cpp_utils::string_to_enum<Type>(val);
    }

    void operator()(refl::quoted_string& value, const char* key)
    {
        std::string val;
        in_ >> val;

        value = val.substr(1, val.size() - 2);
    }

private:
    std::istringstream in_;
};

template<typename T>
void deserialize_flat(T& v, std::string buf)
{
    flat_read_processor p(move(buf));
    reflect(p, std::forward<T>(v));
}

}

struct read_processor
{
    read_processor(const dict_t& d)
        : dict_(d)
    {}

    void operator()(refl::quoted_string & value, dict_t const& entry)
    {
        assert(distance(entry.data().begin(), entry.data().end()) >= 2);
        std::istringstream in_(std::string(std::next(entry.data().begin()), std::prev(entry.data().end())));
        std::string v;
        in_ >> v;
        value = v;
    }

    template<class Type>
    void operator()(Type & value, dict_t const& entry, std::enable_if_t<is_leaf_type_v<Type>>* = nullptr)
    {
        std::istringstream in_(std::string(entry.data().begin(), entry.data().end()));
        std::string val;
        in_ >> value;
    }

    template<class Type>
    void operator()(Type & value, dict_t const& entry, std::enable_if_t<is_flat_type_v<Type>>* = nullptr)
    {
        detail::deserialize_flat(value, std::string(entry.data().begin(), entry.data().end()));
    }

    template<class Type>
    void operator()(Type & value, dict_t const& entry, std::enable_if_t<!is_leaf_type_v<Type> && !is_flat_type_v<Type>>* = nullptr)
    {
        read_processor subp(entry);
        reflect(subp, value);
    }

    template<class Type>
    void operator()(vector<Type> & value, const char* key, refl::aurora_vector_field_tag const& t = refl::aurora_vector_field_tag())
    {
        auto [it_b, it_e] = dict_.find_all(key);
        size_t count = std::distance(it_b, it_e);

        if(t.size_field)
        {
            size_t size;
            this->operator()(size, t.size_field);
            assert(size == count);
        }

        value.resize(count);          

        for(unsigned i = 0; it_b != it_e; it_b++, i++)
        {
            this->operator() (value[i], it_b->second);
        }
    }

    template<class Type>
    void operator()(optional<Type> & value, dict_t const& entry)
    {
        Type v;
        this->operator() (v, entry);
        value = v;
    }

    template<class Type>
    void operator()(Type& value, const char* key)
    {
        dict_t const* entry = dict_.find(key);
        if(entry)
        {
            this->operator() (value, *entry);
        }
        else
            value = Type{};
    }

private:
    dict_t const& dict_;
};

}

