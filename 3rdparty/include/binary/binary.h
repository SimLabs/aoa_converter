/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/
#pragma once 
#include "bytes_array.h"

namespace binary
{
    // size
    typedef uint32_t size_type;

    // byte sequence
    typedef bytes_array::byte_type          byte_type;

    typedef bytes_array::pointer            bytes_raw_ptr;
    typedef bytes_array::const_pointer      bytes_raw_cptr;

    typedef bytes_array                 bytes_t;
    typedef bytes_t&                    bytes_ref;
    typedef bytes_t const&              bytes_cref;
    typedef bytes_t&&                   bytes_rref;
    typedef std::shared_ptr<bytes_t>    bytes_ptr;
    typedef boost::optional<bytes_t>    bytes_opt;

    inline bytes_raw_ptr  raw_ptr(bytes_ref  bytes) { return bytes.data(); }
    inline bytes_raw_cptr raw_ptr(bytes_cref bytes) { return bytes.data(); }
    inline size_type      size   (bytes_cref bytes) { return size_type(bytes.size()); }

    // forward 
    template<class type>
    bytes_t wrap(type const& object);


    inline bytes_t        make_bytes(const void* data, size_t size)     { bytes_t b; b.append(data, size); return b; }

    inline bytes_ptr      make_bytes_ptr(bytes_cref bytes)              { return bytes_ptr(new bytes_t(bytes)); }
    inline bytes_ptr      make_bytes_ptr(bytes_t&&  bytes)              { return bytes_ptr(new bytes_t(std::move(bytes))); }
    inline bytes_ptr      make_bytes_ptr(const void* data, size_t size) { return make_bytes_ptr(make_bytes(data, size)); }

    template<class type>
    inline bytes_ptr      make_bytes_ptr(type const& obj)               { return make_bytes_ptr(wrap(obj)); }



    template<class T>
    T& looks_like(bytes_ref data, size_t offset = 0) { return *reinterpret_cast<T*>(raw_ptr(data) + offset); }

} // binary

using binary::bytes_t;
using binary::bytes_ref;
using binary::bytes_cref;
using binary::bytes_opt;

using bin_size = binary::size_type;
