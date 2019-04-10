/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

/// auxiliary functions
inline std::string get_current_executable_path()
{
    const char* CANT_GET_EXEC_NAME = "Can't get current executable name";
    const size_t MAX_FILE_NAME = 256;

#ifdef _WIN32

    char szFileName[MAX_FILE_NAME];
    if (!GetModuleFileNameA(NULL, szFileName, MAX_FILE_NAME))
    {
        return std::string(CANT_GET_EXEC_NAME);
    }
    return std::string(szFileName);

#elif __linux__

    char executable_name[MAX_FILE_NAME];
    int num_bytes = readlink("/proc/self/exe", executable_name, MAX_FILE_NAME);
    if (num_bytes == -1)
    {
        return std::string(CANT_GET_EXEC_NAME);
    }
    executable_name[num_bytes] = 0;
    return std::string(executable_name);

#else
#error "Unsupported platform"
#endif
}

inline std::string get_curr_exe_name()
{
    // avoiding usage of boost::filesystem in common header 

    string path = get_current_executable_path();
    size_t slash_pos     = path.find_last_of('/');
    size_t rev_slash_pos = path.find_last_of('\\');
    size_t dot_pos       = path.find_last_of('.');

    slash_pos       = (slash_pos     == string::npos ? 0 : slash_pos     + 1);
    rev_slash_pos   = (rev_slash_pos == string::npos ? 0 : rev_slash_pos + 1);

    slash_pos = std::max(slash_pos, rev_slash_pos);
    return path.substr(slash_pos, dot_pos - slash_pos);
}
