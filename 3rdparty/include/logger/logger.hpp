/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "common/macro_helper.h"
#include <osg/Notify>
                                                        
#define LogDebug(message)   OSG_DEBUG << message
#define LogInfo(message)    OSG_INFO << message
#define LogWarn(message)    OSG_WARN << message
#define LogError(message)   OSG_FATAL<< message

///////////
/// VERIFY

struct verify_error : runtime_error { verify_error(string err) : runtime_error(err){} };

#define Verify(expr)                                                        \
do{ if (!(expr))                                                            \
{                                                                           \
    std::stringstream ss;                                                   \
    ss << "Verification failed: " << #expr << " at \""                      \
       << BOOST_CURRENT_FUNCTION << "\" in " << __FILE__ << ":" << __LINE__;\
    LogError(ss.str());                                                     \
    throw verify_error(ss.str());                                           \
} } while MACRO_FALSE_CONDITION

#define VerifyMsg(expr, msg)                                                    \
do{ if (!(expr))                                                                \
{                                                                               \
    std::stringstream ss;                                                       \
    ss << "Verification failed: " << #expr << ". Message: " << msg << ". At \"" \
       << BOOST_CURRENT_FUNCTION << "\" in " << __FILE__ << ":" << __LINE__;    \
    LogError(ss.str());                                                         \
    throw verify_error(ss.str());                                               \
} } while MACRO_FALSE_CONDITION

#define SUPRESS_VERIFY \
catch(verify_error const&){}

//////////
/// ASSERT
#define LOG_ASSERT
#include "assert.hpp"
