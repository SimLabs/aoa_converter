/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "binary/bytes_array.h"
#include "serialization/io_streams.h"

namespace cpp_utils
{
    struct fast_string
    {                               
        typedef const char* const_pointer;

        explicit fast_string(const_pointer str = 0);

        fast_string(fast_string const& other);
        fast_string(fast_string&& other);

        fast_string& operator=(fast_string other);
        fast_string& operator=(const_pointer str);

    public:
        fast_string::const_pointer c_str() const;

        size_t size () const;
        bool   empty() const;

    public:
        const_pointer begin() const;
        const_pointer end  () const;

    public:
        bool operator==(const_pointer str) const;
        bool operator!=(const_pointer str) const;

    public:
        bool operator==(fast_string const& other) const;
        bool operator< (fast_string const& other) const;

    public:
        char& operator[](size_t index);
        char  operator[](size_t index) const;

    private:
        binary::bytes_array impl_;
    };

    inline void write(binary::output_stream& os, fast_string const& str);
    inline void read (binary::input_stream & is, fast_string& str);

    inline size_t hash_value(fast_string const& str);

} // cpp_utils 



/// IMPLEMENTATION ////////////////////////////////////////////////////////



namespace cpp_utils
{
    inline fast_string::fast_string(const_pointer str)
    {
        if (str)
            impl_.append(str, strlen(str) + 1);
    }

    inline fast_string::fast_string(fast_string const& other)
        : impl_(other.impl_)
    {}

    inline fast_string::fast_string(fast_string&& other)
        : impl_(move(other.impl_))
    {}

    inline fast_string& fast_string::operator=(fast_string other)
    {
        impl_ = move(other.impl_);
        return *this;
    }

    inline fast_string& fast_string::operator=(const_pointer str)
    {
        if (str)
            impl_.append(str, strlen(str) + 1);

        return *this;
    }

    inline fast_string::const_pointer fast_string::c_str() const
    {
        return impl_.data();
    }

    inline size_t fast_string::size() const
    {
        return impl_.size() - 1;
    }

    inline bool fast_string::empty() const
    {
        return size() == 0;
    }

    inline fast_string::const_pointer fast_string::begin() const { return impl_.data(); }
    inline fast_string::const_pointer fast_string::end  () const { return impl_.data() + impl_.size(); }

    inline bool fast_string::operator==(const_pointer str) const
    {
        return strcmp(impl_.data(), str) == 0;
    }

    inline bool fast_string::operator!=(const_pointer str) const
    {
        return !operator==(str);
    }

    inline bool fast_string::operator==(fast_string const& other) const
    {
        return strcmp(impl_.data(), other.impl_.data()) == 0;
    }

    inline bool fast_string::operator<(fast_string const& other) const
    {
        return strcmp(impl_.data(), other.impl_.data()) < 0;
    }

    inline char& fast_string::operator[](size_t index)       { return impl_[index]; }
    inline char  fast_string::operator[](size_t index) const { return impl_[index]; }


    inline void write(binary::output_stream& os, fast_string const& str)
    {
        os.append(str.c_str(), str.size() + 1);
    }

    inline void read(binary::input_stream& is, fast_string& str)
    {
        str = is.curr();
        is.skip(str.size() + 1);
    }

    namespace details
    {
        // 'stolen' from MSVC 2013 STL 
        inline size_t str_hash(const unsigned char *data, size_t count)
        {
            static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");

            // FNV-1a hash function for bytes in [_First, _First+_Count)
            const size_t offset_basis = 14695981039346656037ULL;
            const size_t prime = 1099511628211ULL;

            size_t value = offset_basis;
            for (size_t next = 0; next < count; ++next)
            {	// fold in another byte
                value ^= (size_t)data[next];
                value *= prime;
            }

            value ^= value >> 32;
            return value;
        }
    }

    inline size_t hash_value(fast_string const& str)
    {
        return details::str_hash((const unsigned char*)str.c_str(), str.size());
    }
} // cpp_utils 

