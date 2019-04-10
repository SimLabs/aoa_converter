/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace cpp_utils
{

struct reflect_move_processor
{
    template<typename T>
    void operator()(T &lhs, T &rhs, char const * /*name*/, ...)
    {
        // avoiding call of move assignment operator
        // e.g. useful in orm<U> objects, where I don't want to have move assignment operator 
        // no need for MSVC starting from ver. 2015, just declare 'T(T&&) = default' instead
        lhs.~T();
        new (&lhs) T(std::move(rhs));
    }
};

} // namespace cpp_utils

#define ENABLE_REFL_MOVE(type)                          \
type& operator=(type&& src)                             \
{                                                       \
    cpp_utils::reflect_move_processor proc;             \
    reflect2(proc, *this, src);                         \
    return *this;                                       \
}                                                       \
type(type&& src)                                        \
    : type()                                            \
{                                                       \
    *this = std::move(src);                             \
}                                          

#define ENABLE_DEFAULT_COPY(type)    \
    type(const type &src) = default; \
    type& operator=(const type &src) = default;
         
#define ENABLE_NO_COPY(type)        \
    type(const type &src) = delete; \
    type& operator=(const type &src) = delete;

#define ENABLE_REFL_MOVE_DEFAULT_COPY(type) \
    ENABLE_DEFAULT_COPY(type) \
    ENABLE_REFL_MOVE(type)

#define ENABLE_REFL_MOVE_NO_COPY(type) \
    ENABLE_NO_COPY(type) \
    ENABLE_REFL_MOVE(type)
