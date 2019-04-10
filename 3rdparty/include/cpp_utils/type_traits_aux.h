/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

namespace cpp_utils 
{

// see second answer at http://stackoverflow.com/questions/11008189/is-it-possible-to-check-if-a-member-function-is-defined-for-a-class-even-if-the

template <class T>
struct has_cbegin_cend
{
    struct yes { char nothing;    };
    struct no  { yes  nothing[2]; };

    template <typename U>
    static auto test(U* u) -> decltype((*u).cbegin(), (*u).cend(), yes());
    static no   test(...);

    enum { value = (sizeof(yes) == sizeof test((T*)0)) };
    typedef boost::integral_constant<bool, value> type;
};


template <class T>
struct has_rbegin_rend
{
    struct yes { char nothing;    };
    struct no  { yes  nothing[2]; };

    template <typename U>
    static auto test(U* u) -> decltype((*u).rbegin(), (*u).rend(), yes());
    static no   test(...);

    enum { value = (sizeof(yes) == sizeof test((T*)0)) };
    typedef boost::integral_constant<bool, value> type;
};


} // cpp_utils 

#define ENABLE_IF_BEGIN typename std::enable_if
#define ENABLE_IF_END ::type* = 0
#define ENABLE_IF_MACRO(expr) typename std::enable_if<expr>::type* = 0