/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

#include "logger/logger.hpp"
#include "binary/binary.h"
#include "io_streams_fwd.h"

namespace binary
{

// -- output_stream -- 
// is defined in "io_streams_fwd.h" as bytes_array typedef 
   
struct input_stream
{
    input_stream(bytes_t const& bytes, size_t offset = 0)
        : begin_(raw_ptr(bytes))
        , end_  (begin_+ size(bytes))
        , cur_  (begin_ + offset)
    {
        Assert(cur_ <= end_);
    }

    input_stream(void const* data, size_t size)
        : begin_(static_cast<const char*>(data))
        , end_  (begin_ + size)
        , cur_  (begin_)
    {
    }

    void read(void* data, size_t size) // size in bytes
    {
        Verify(end_ - cur_ >= ptrdiff_t(size));

        memcpy(data, cur_, size);
        cur_ += size;
    }

    template<class T>
    void read(T& t)
    {
        read(&t, sizeof(T));
    }

    bool        eof     () const { return cur_ == end_; }
    size_t      left    () const { return end_ - cur_ ; }
    const char* curr    () const { return cur_; }
    size_t      curr_ofs() const { return cur_ - begin_; }

    void        reset(size_t offset = 0)    { cur_ = begin_ + offset; }
    void        skip (size_t size      )    { Verify(end_ - cur_ >= ptrdiff_t(size)); cur_ += size ;}

private:
    char const* begin_;
    char const* end_  ;
    char const* cur_  ;
};

} //binary
