/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "common/time_counter.h"
#include "cpp_utils/thread_safe_object.h"

struct __logger_scope__{ static const char* name() { return "=General="; }};


namespace logging
{

inline string level_name(level l)
{
    static string names [] =
    {
        "DEBUG",
        "TRACE",
        "INFO" ,
        "WARN" ,
        "ERROR"
    };

    return names[l];
}

namespace details
{

#if defined _WIN32 || defined __CYGWIN__
#  pragma warning (push)
#  pragma warning (disable:4251)
#endif

struct LOGGER_API logger
{
    logger(properties const& props);
    logger(logger&& other);
    
    ~logger();
    
    //
    bool need_log(level desired_level, const char* source) const;
    void log(level desired_level, const char* file, size_t line_num, const char* source, const char* msg);

    //
    void add_writer(writer_ptr ptr);
    void del_writer(writer_ptr ptr);

    void reset_time(boost::posix_time::time_duration const& td);

private:
    logger(logger const &other);

private:
    properties              props_  ;
    std::set<writer_ptr>    writers_;

// time sync routine 
private:
    time_counter::time_duration last_offset_;
    time_counter                time_       ;
};

#if defined _WIN32 || defined __CYGWIN__
#  pragma warning (pop)
#endif

typedef thread_ops::thread_safe_obj<logger>::data_monitor log_mon_t;   
LOGGER_API log_mon_t __logger();

} // details
} // logging


#define LOG_MSG(l, msg)                                                                     \
  do {                                                                                      \
    auto lg = std::move(logging::details::__logger());                                      \
                                                                                            \
    if (lg->need_log((l), __logger_scope__::name())) {                                      \
      std::stringstream logger__str;                                                        \
      logger__str << msg; /* don't use brackets here! */                                    \
      lg->log((l), __FILE__, __LINE__, __logger_scope__::name(), logger__str.str().c_str());\
    }                                                                                       \
  } while(0)
