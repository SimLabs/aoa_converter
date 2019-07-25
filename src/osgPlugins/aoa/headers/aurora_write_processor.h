#pragma once
#include "aurora_format.h"
#include "aurora_processor_traits.h"
#include "cpp_utils/enum_to_string.h"

namespace aurora
{

namespace detail
{

struct flat_struct_processor
{
    template<class Type>
    void operator()(Type& value, const char* key, std::enable_if_t<is_leaf_type_v<Type>>* = nullptr)
    {
        out_ << value << "\t";
    }

    template<class Type>
    void operator()(Type& value, const char* key, std::enable_if_t<std::is_enum_v<Type>>* = nullptr)
    {
        out_ << cpp_utils::enum_to_string(value) << "\t";
    }

    void operator()(refl::quoted_string& value, const char* key)
    {
        out_ << "\"" << string(value) << "\"" << "\t";
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
    auto value = p.value();
    if(!value.empty())
        value.pop_back(); // remove trailing space
    return value;
}

}


struct write_processor
{
    write_processor()
        : out_()
    {}

    void operator()(refl::quoted_string const& value, const char* key)
    {
        string quote = "\"";
        out_ << indent_ << key << " " << quote << string(value) << quote << std::endl;
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
    void operator()(vector<Type> const& value, const char* key, refl::aurora_vector_field_tag const& t = refl::aurora_vector_field_tag())
    {
        if(t.size_field)
            this->operator()(value.size(), t.size_field);
        for(auto const& v: value)
        {
            this->operator ()(v, key);
        }
    }

    template<class Type>
    void operator()(optional<Type> const& value, const char* key)
    {
        if(value)
            this->operator ()(*value, key);
    }

    string result()
    {
        return out_.str();
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
    std::ostringstream out_;
    string indent_;
};

}
