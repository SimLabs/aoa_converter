/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#ifdef _WIN32
#   include <windows.h>
#elif defined(__linux__)
#   include <time.h>
#   include <unistd.h>
#endif


struct PerformanceCounter
{
    typedef void (*undefined_tag)();
    static __forceinline void undefined() {}


    __forceinline PerformanceCounter( const undefined_tag )
    {
    }

    __forceinline PerformanceCounter()
    {
        reset();
    }

    __forceinline void reset()
    {
        counter = get_counter();
    }

    // returns delta between counter and now
    __forceinline uint64_t time() const
    {
        return get_counter() - counter;
    }

    // returns delta in seconds
    static __forceinline double delta_s( uint64_t delta )
    {
        return (double)delta / get_frequency();
    }

    // returns delta between counter and now (in seconds)
    __forceinline double time_s() const
    {
        return delta_s(time());
    }

    // returns delta in milliseconds
    static __forceinline double delta_ms( uint64_t delta )
    {
        return (double)delta / get_frequency();
    }

    // returns delta between counter and now (in milliseconds)
    __forceinline double time_ms() const
    {
        return delta_ms(time() * 1000);
    }

    static __forceinline uint64_t get_counter()
    {
#ifdef _WIN32
        return __rdtsc();
        //__asm rdtsc
        // result will be in EAX:EDX
#elif defined(__linux__)
#   if defined(__i386__)
        __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
        return x;
#   elif defined(__x86_64__)
        unsigned hi, lo;
        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t)lo) | (((uint64_t)hi) << 32);
#   endif
#endif
    }

    // calculates once and uses frequency
    static uint64_t get_frequency()
    {
        static uint64_t frequency = -1;

        // need to evaluate
        if (frequency == uint64_t(-1))
        {
#ifdef _WIN32
            // get start timings
            uint64_t sysStart;
            ::QueryPerformanceCounter((LARGE_INTEGER *)&sysStart);
            const uint64_t cpuStart = get_counter();

            // sleep awhile
            ::Sleep(50);

            // get finish timings
            uint64_t sysFinish;
            ::QueryPerformanceCounter((LARGE_INTEGER *)&sysFinish);
            const uint64_t cpuFinish = get_counter();

            // get ratio
            ::QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
            const double ratio = double(cpuFinish - cpuStart) / (sysFinish - sysStart);

            if (ratio < (1. - 1e-10) || ratio > (1. + 1e-10))
                frequency = uint64_t(frequency * ratio);
#elif defined(__linux__)
            // get start timings
            struct timespec sysStart;
            clock_gettime(CLOCK_MONOTONIC_RAW, &sysStart);
            const uint64_t cpuStart = get_counter();

            // sleep awhile
            static const unsigned microsec_to_wait = 50000;
            usleep(microsec_to_wait);

            // get finish timings
            struct timespec sysFinish;
            clock_gettime(CLOCK_MONOTONIC_RAW, &sysFinish);
            const uint64_t cpuFinish = get_counter();

            static const int nanosec_in_sec = 1000000000;
            int nanos_passed = sysFinish.tv_nsec - sysStart.tv_nsec;
            if (nanos_passed < 0)
                nanos_passed += nanosec_in_sec;

            const double ratio = double(cpuFinish - cpuStart) / nanos_passed;
            frequency = uint64_t(ratio * nanosec_in_sec);
#endif
        }

        return frequency;
    }


private:

    uint64_t counter;
};
