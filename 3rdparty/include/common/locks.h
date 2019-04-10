/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "logger/assert.hpp"

namespace locks
{
struct bool_lock
    : noncopyable
{
    bool_lock(bool &val, bool no_recursive = true)
        : val_(&val)
    {
        if (no_recursive)
            Assert(!val);
        *val_ = true;
    }

    bool_lock(bool_lock&& other)
        : val_(other.val_)
    {
        other.val_ = 0;
    }

    void release()
    {
        if (val_)
        {
            *val_ = false;
        }
    }

    ~bool_lock()
    {
        release();
    }

    private:
        bool* val_;
};

template<class T>
struct optional_lock
    : noncopyable
{
    typedef optional<T> opt_type;
    typedef opt_type&   opt_type_ref;
    typedef opt_type*   opt_type_ptr;

    optional_lock(opt_type_ref ref, T const& value)
        : ref_(&ref)
    {
        *ref_ = value;
    }

    optional_lock(optional_lock&& other)
        : ref_(other.ref_)
    {
        other.ref_ = nullptr;
    }

    ~optional_lock()
    {
        if (ref_)
            *ref_ = boost::none;
    }
    
private:
    opt_type_ptr ref_;
};

} // namespace locks
