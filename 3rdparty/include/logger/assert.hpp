/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <assert.h>
#include <sstream>
#include "common/macro_helper.h"

//////////
/// ASSERT
#ifdef LOG_ASSERT
#   define AssertLogError(msg)  LogError(msg)
#else
#   define AssertLogError(msg)
#endif

#ifdef Assert
#   undef Assert
#endif

// 1. "if (false)" avoids evaluation 
// 2. condition "0, 0" avoids warning C4127 (conditional expression is constant)
// 3. avoids warning C4189 (local variable is initialized but not referenced)

#if defined(NDEBUG) && !defined(_TESTBED)
#   define Assert(expr)          \
    do                           \
    {                            \
        if MACRO_FALSE_CONDITION \
        {                        \
            (void)(expr);        \
        }                        \
    } while MACRO_FALSE_CONDITION
#else
#   define Assert(expr)                                                     \
do{                                                                         \
if (!(expr))                                                                \
{                                                                           \
    std::stringstream ss;                                                   \
    ss << "Assertion failed: " << #expr << " at \""                         \
       << BOOST_CURRENT_FUNCTION << "\" in " << __FILE__ << ":" << __LINE__;\
    AssertLogError(ss.str());                                               \
    assert(expr);                                                           \
} } while MACRO_FALSE_CONDITION
#endif
