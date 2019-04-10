/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "common/time_counter.h"

namespace logging
{
    struct item;

    struct LOGGER_API timer
    {
        timer(string const& name);
        timer(string const& name, string const& file, size_t line);

        ~timer();

    private:
        static timer*& cur_profiler();
        void add_child(item&& it);

    private:
        string              name_;
        string              place_;
        timer*              parent_;
        time_counter        counter_;
        std::vector<item>   children_;
    };
}

#define ProfileScope(name) logging::timer t(name, __FILE__, __LINE__)


inline PerformanceCounter& __pc__()
{
    static PerformanceCounter pc;
    return pc;
}

struct profile_section
{
    struct section_intance_t
    {
        section_intance_t(profile_section& section)
            : start_time_(__pc__().time())
            , section_(section)
        {}

        ~section_intance_t()
        {
            section_.add_time(__pc__().time() - start_time_);
        }

    private:
        uint64_t start_time_;
        profile_section& section_;
    };

    profile_section(std::string const& name)
        : name_(name)
        , total_time_(0)
    {}

    ~profile_section()
    {
        double all_time = PerformanceCounter::delta_s(total_time_);
        LogDebug(name_ << " time: " << all_time << " sec");
    }

    void add_time(uint64_t time)
    {
        total_time_ += time;
    }

    section_intance_t create_instance()
    {
        return section_intance_t(*this);
    }

private:
    std::string name_;
    uint64_t total_time_;
};


inline std::string create_profile_label_name(std::string const& filename, size_t line_num, std::string const& func_name)
{
    std::stringstream ss;
    ss << filename << ": " << line_num << " " << func_name;
    return ss.str();
}

#define DECLARE_PROFILE_SECTION()                                                        \
static profile_section sec(create_profile_label_name(__FILE__, __LINE__, __FUNCTION__)); \
auto ctx = sec.create_instance(); 
