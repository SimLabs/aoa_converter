#pragma once

namespace cpp_utils
{
    template<typename Value, typename Tag>
    struct tagged_struct
    {
        tagged_struct()
        {}
        
        explicit tagged_struct(Value const &value)
            : val(value)
        {}
    
        Value val;


        REFL_INNER(tagged_struct)
            REFL_ENTRY(val)
        REFL_END()
    };

    template<typename Value, typename Tag>
    bool operator==(tagged_struct<Value, Tag> const &lhs, tagged_struct<Value, Tag> const &rhs)
    {
        return lhs.val == rhs.val;
    }

    template<typename Value, typename Tag>
    bool operator!=(tagged_struct<Value, Tag> const &lhs, tagged_struct<Value, Tag> const &rhs)
    {
        return lhs.val != rhs.val;
    }

    template<typename Value, typename Tag>
    bool operator<(tagged_struct<Value, Tag> const &lhs, tagged_struct<Value, Tag> const &rhs)
    {
        return lhs.val < rhs.val;
    }
    
    template<typename Value, typename Tag>
    bool operator>(tagged_struct<Value, Tag> const &lhs, tagged_struct<Value, Tag> const &rhs)
    {
        return lhs.val < rhs.val;
    }
    
    template<typename Value, typename Tag>
    bool operator<=(tagged_struct<Value, Tag> const &lhs, tagged_struct<Value, Tag> const &rhs)
    {
        return lhs.val <= rhs.val;
    }
    
    template<typename Value, typename Tag>
    bool operator>=(tagged_struct<Value, Tag> const &lhs, tagged_struct<Value, Tag> const &rhs)
    {
        return lhs.val >= rhs.val;
    }
} // namespace cpp_utils

namespace boost
{
    template<typename Value, typename Tag>
    size_t hash_value(cpp_utils::tagged_struct<Value, Tag> const &item)
    {
        return hash_value(item.val);
    }    
}

namespace std
{
    template<typename Value, typename Tag>
    struct hash<cpp_utils::tagged_struct<Value, Tag>>
    {
        size_t operator()(cpp_utils::tagged_struct<Value, Tag> const &val) const
        {
            return value_hash_(val.val);
        }

    private:
        std::hash<Value> value_hash_;
    };
}

namespace dict 
{
    template<typename Value, typename Tag>
    void write(dict_t& dict, cpp_utils::tagged_struct<Value, Tag> const &item)
    {
        write(dict, item.val);
    }

    template<typename Value, typename Tag>
    void read(dict_t const& dict, cpp_utils::tagged_struct<Value, Tag> &item, bool log_on_absence)
    {
        read(dict, item.val, log_on_absence);
    }

} // namespace dict

namespace binary 
{
    template<typename Value, typename Tag>
    void write(output_stream& os, cpp_utils::tagged_struct<Value, Tag> const &item)
    {
        write(os, item.val);
    }

    template<typename Value, typename Tag>
    void read (input_stream & is, cpp_utils::tagged_struct<Value, Tag> &item)
    {
        read(is, item.val);
    }

} // namespace binary