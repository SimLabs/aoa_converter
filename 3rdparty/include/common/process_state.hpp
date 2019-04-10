/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#ifdef _WIN32
#  include <PowrProf.h>
#  pragma comment(lib, "Powrprof.lib")
#  pragma comment(lib, "Psapi.lib")
#elif __linux__
#  include <sys/sysinfo.h>
#else
#  error "Unsupported platform"
#endif


namespace process_state
{


#ifdef _WIN32
namespace
{
typedef struct {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;
} // anonymous namespace


inline proc_id_t current_process_id()
{
    return proc_id_t(GetCurrentProcessId());
}

inline mem_size_t phys_memory()
{
    // hopefully total physical memory won't change during program execution
    static mem_size_t tot_phys_mem = 0;

    if (tot_phys_mem == 0)
    {
        MEMORYSTATUSEX mem_stat;
        mem_stat.dwLength = sizeof(mem_stat);
        GlobalMemoryStatusEx(&mem_stat);
        tot_phys_mem = mem_stat.ullTotalPhys;
    }

    return tot_phys_mem;
}


inline cpu_freq_t cpu_frequency()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

	ULONG const size = si.dwNumberOfProcessors * sizeof(PROCESSOR_POWER_INFORMATION);
    boost::scoped_array<char> buff(new char[size]);

    if (0 != CallNtPowerInformation(ProcessorInformation, NULL, 0, buff.get(), size))
    {
        LogInfo("CallNtPowerInformation failed");
        return 0;
    }

    PPROCESSOR_POWER_INFORMATION ppi = reinterpret_cast<PPROCESSOR_POWER_INFORMATION>(buff.get());
    return ppi[0].MaxMhz; // hopefully all processors have same frequency
}

inline uint16_t number_of_cpus()
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return uint16_t(sys_info.dwNumberOfProcessors);
}

#elif __linux__

inline mem_size_t multiplier(std::string specifier)
{
    std::transform(specifier.begin(), specifier.end(), specifier.begin(), ::tolower);

    if (specifier == "b")
        return 1;
    else if (specifier == "kb")
        return 1024;
    else if (specifier == "mb")
        return (1024 * 1024);
    else if (specifier == "gb")
        return mem_size_t(1024 * 1024) * 1024;
    else if (specifier == "tb")
        return mem_size_t(1024 * 1024) * mem_size_t(1024 * 1024);

    return 0;
}

inline void get_memory(mem_size_t& total_mem, mem_size_t& free_mem)
{
    total_mem = 0;
    free_mem  = 0;

    std::ifstream ifstr("/proc/meminfo");

    char specifier[3];
    std::string line;
    char flag = 0;
    while (std::getline(ifstr, line) && flag < 2)
    {
        if (line.size() >= 9 && line.substr(0, 9) == "MemTotal:")
        {
            sscanf(line.c_str(), "%*s %lu %s", &total_mem, specifier);
            total_mem = total_mem * multiplier(specifier);
            ++flag;
        }

        if (line.size() >= 8 && line.substr(0, 8) == "MemFree:")
        {
            sscanf(line.c_str(), "%*s %lu %s", &free_mem, specifier);
            free_mem = free_mem * multiplier(specifier);
            ++flag;
        }
    }
}


inline proc_id_t current_process_id()
{
    return proc_id_t(getpid());
}

inline mem_size_t phys_memory()
{
    // hopefully total physical memory won't change during program execution
    static mem_size_t tot_phys_mem = 0;

    if (tot_phys_mem == 0)
    {
        mem_size_t sys_total_mem = 0;
        mem_size_t sys_free_mem  = 0;
        get_memory(sys_total_mem, sys_free_mem);
        tot_phys_mem = sys_total_mem;
    }

    return tot_phys_mem;
}


inline cpu_freq_t cpu_frequency()
{
    std::ifstream ifstr("/proc/cpuinfo");

    std::string line;
    while (std::getline(ifstr, line))
    {
        if (line.size() >= 7 && line.substr(0, 7) == "cpu MHz" && line.find(':') != line.npos)
        {
            size_t col_pos = line.find(':');
            if (col_pos == line.size())
                return 0;

            std::string mhz_str = line.substr(col_pos + 1);

            float freq;
            sscanf(mhz_str.c_str(), "%f", &freq);
            return cpu_freq_t(freq);
        }
    }

    return 0;
}

inline uint16_t number_of_cpus()
{
    return uint16_t(get_nprocs());
}

#else
#  error "Unsupported platform"
#endif

} // namespace process_state
