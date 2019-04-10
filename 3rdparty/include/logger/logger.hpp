/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "common/dyn_lib.h"
#include "common/macro_helper.h"
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

#ifdef LOGGER_LIB
# define LOGGER_API __HELPER_DL_EXPORT
#else
# define LOGGER_API __HELPER_DL_IMPORT
#endif

namespace logging
{
/////////////////////////////////////////////////////
enum level
{
    L_DEBUG  = 0,
    L_TRACE,
    L_INFO,
    L_WARN,
    L_ERROR
};

/////////////////////////////////////////////////////
#if defined _WIN32 || defined __CYGWIN__
#  pragma warning (push)
#  pragma warning (disable:4251)
#endif

struct LOGGER_API properties
{
    /* Reading logger properties from file
     *
     * File should contains lines like below.
     * Use leading # for comment.
     * File could contain line with default behavior description. If no 'default' entry found, default is set to INFO
     * Valid levels: TRACE, DEBUG, INFO, WARN, ERROR
     *
     *    default=INFO
     *    AbraCaDabra=TRACE
     *    #BubaZuba=WARN
     *    HariKrishna=INFO
     *
     */
    properties(level default_level = L_DEBUG, std::string filename = "");

    unordered_map<std::string, level>           levels;
    level                                       def_level;
};

#if defined _WIN32 || defined __CYGWIN__
#  pragma warning (pop)
#endif

////////////////////////////////////////////////////
struct writer
{
    virtual ~writer(){}
    virtual void write_msg(pt::time_duration td, level l, const char* file, size_t line_num, const char* source, const char* msg) = 0;
};
typedef std::shared_ptr<writer> writer_ptr;

/////////////////////////////////////////////////////

// just helper for your own logger
LOGGER_API std::string time_string(pt::time_duration td);
LOGGER_API std::string msg_string (pt::time_duration td, level l, const char* file, size_t line_num, const char* source, const char* msg);

LOGGER_API void add_console_writer     ();
LOGGER_API void add_default_file_writer();

LOGGER_API void add_writer             (writer_ptr);
LOGGER_API void del_writer             (writer_ptr);

/////////////////////////////////////////////////////
LOGGER_API void reset_logger(properties const& props);
LOGGER_API void reset_time  (boost::posix_time::time_duration const& td);

} // logging


/////////////////////////////////////////////////////
/// client macroses /////////////////////////////////
#include "logger_impl.hpp"

#define DECL_LOGGER(source)                             \
    struct __logger_scope__                             \
    {                                                   \
        static string format_name()                     \
        {                                               \
            std::string strsrc(source);                 \
            boost::replace_all(strsrc, "(", "[");       \
            boost::replace_all(strsrc, ")", "]");       \
            return strsrc;                              \
        }                                               \
                                                        \
        static const char* name()                       \
        {                                               \
            static std::string strsrc(format_name());   \
            return strsrc.c_str();                      \
        }                                               \
    }
                                                        
#define LogDebug(message)   LOG_MSG(logging::L_DEBUG, message)
#define LogTrace(message)   LOG_MSG(logging::L_TRACE, message)
#define LogInfo(message)    LOG_MSG(logging::L_INFO , message)
#define LogWarn(message)    LOG_MSG(logging::L_WARN , message)
#define LogError(message)   LOG_MSG(logging::L_ERROR, message)

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
