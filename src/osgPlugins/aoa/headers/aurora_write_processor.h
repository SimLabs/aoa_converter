#pragma once
#include "aurora_format.h"


namespace aurora
{

template<class T>
struct is_leaf_type
{
    enum
    {
        value = (std::is_same_v<std::remove_reference_t<T>, std::string>  ||
                 std::is_arithmetic_v<std::remove_reference_t<T>> ||
                 std::is_enum_v<std::remove_reference_t<T>>
         )
    };
};

template<class T>
struct is_flat_type
{
    enum
    {
        value = (std::is_same_v<std::remove_reference_t<T>, aurora::data_buffer::vao_buffer::format_offset> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::data_buffer::vao_buffer::vertex_format::vertex_attribute> ||
                 std::is_same_v<std::remove_reference_t<T>, aurora::data_buffer::offset_size>
        )
    };
};

template<class T>
constexpr bool is_leaf_type_v = is_leaf_type<T>::value;

template<class T>
constexpr bool is_flat_type_v = is_flat_type<T>::value;

namespace detail
{

struct flat_struct_processor
{
    template<class Type>
    void operator()(Type& value, const char* key, std::enable_if_t<is_leaf_type_v<Type>>* = nullptr)
    {
        out_ << value << " ";
    }

    string value()
    {
        return out_.str();
    }

    private:
        std::ostringstream out_;
};

template<typename T>
string serialize_flat(T&& v)
{
    flat_struct_processor p;
    reflect(p, std::forward<T>(v));
    return p.value();
}

}


struct write_processor
{
    write_processor(string const& filename)
        : out_(filename)
    {}

    void operator()(string const& value, const char* key)
    {
        string quote = value.find(' ') != std::string::npos ? "\"" : "";
        out_ << indent_ << key << " " << quote << value << quote << std::endl;
    }

    template<class Type>
    void operator()(Type const& value, const char* key, std::enable_if_t<is_leaf_type_v<Type>>* = nullptr)
    {
        out_ << indent_ << key << " " << value << std::endl;
    }

    template<class Type>
    void operator()(Type const& value, const char* key, std::enable_if_t<is_flat_type_v<Type>>* = nullptr)
    {
        out_ << indent_ << key << " " << detail::serialize_flat(value) << std::endl;
    }

    template<class Type>
    void operator()(Type const& value, const char* key, std::enable_if_t<!is_leaf_type_v<Type> && !is_flat_type_v<Type>>* = nullptr)
    {
        out_ << indent_ << key << " {" << std::endl;
        add_indent();
        reflect(*this, value);
        remove_indent();
        out_ << indent_ << "}" << std::endl;
    }

    template<class Type>
    void operator()(vector<Type> const& value, const char* key, aurora_vector_field_tag const& t = aurora_vector_field_tag())
    {
        if(t.size_field)
            this->operator()(value.size(), t.size_field);
        for(auto const& v: value)
        {
            this->operator ()(v, key);
        }
    }

private:
    void add_indent()
    {
        indent_ += '\t';
    }

    void remove_indent()
    {
        indent_.pop_back();
    }

private:
    std::ofstream out_;
    string indent_;
};

}
