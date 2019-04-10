/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

#if defined(_WIN32) && !defined(NDEBUG) && defined(VLD_CHECK)

#include "system_utils.h"
#include "time_hlp.h"

struct leak_checker
{
    leak_checker()
    {
        VLDSetReportOptions(VLD_OPT_REPORT_TO_DEBUGGER | VLD_OPT_REPORT_TO_STDOUT, L"");
        VLDSetOptions(VLDGetOptions() | VLD_OPT_AGGREGATE_DUPLICATES, 100000, 100000);
    }

   ~leak_checker()
    {
        auto count = VLDGetLeaksCount();
        if (count != 0)
        {
            if (!fs::is_directory("leaks"))
                fs::create_directories("leaks");

            auto    filename = "leaks/pending_" + get_curr_exe_name() + "_" + bt::to_file_string(bt::loc_time()) + ".txt";

            wchar_t filename_w [0x400] = {};
            mbstowcs(filename_w, filename.c_str(), 0x400);

            VLDSetReportOptions(VLD_OPT_REPORT_TO_DEBUGGER | VLD_OPT_REPORT_TO_STDOUT | VLD_OPT_REPORT_TO_FILE, filename_w);
        }
    }

};

#else 

struct leak_checker
{
};

#endif
