/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <iostream>
#include <float.h>

#ifdef __linux__
//#include <fpu_control.h>
#include <fenv.h>
#endif

/// Enable error generation on floating point operations.
static void floating_point_exceptions(bool enable)
{

#ifdef _WIN32
    /**
     * This code allows to catch SIGFPE on Windows
    **/
    // Set the x86 floating-point control word according to what
    // exceptions you want to trap.
    unsigned int cw = 0;
    _controlfp_s(&cw, 0, 0);
    static unsigned int cwOriginal = cw; // remember the state when called for the first time
    if (enable)
    {
        // Set the exception masks off for exceptions that you want to trap.
        // When a mask bit is set, the corresponding floating-point
        // exception is blocked from being generating.
        cw &= ~(EM_OVERFLOW | EM_UNDERFLOW | EM_ZERODIVIDE |
                EM_DENORMAL | EM_INVALID);
        // For any bit in the second parameter (mask) that is 1, the
        // corresponding bit in the first parameter is used to update
        // the control word.
        _controlfp_s(NULL, cw, MCW_EM);
    }
    else
    {
        _controlfp_s(NULL, cwOriginal, MCW_EM);
    }
#elif __linux__
    static int defExcept = fegetexcept();
    if (enable)
    {        
        feenableexcept(FE_ALL_EXCEPT);
    }
    else
    {
        fedisableexcept(FE_ALL_EXCEPT);
        feenableexcept(defExcept);
    }
#else
#error "Platform not supported"
#endif

}
