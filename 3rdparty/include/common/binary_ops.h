/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

template<typename type>
__forceinline bool binary_eq( const type & a, const type & b )
{
    return a == b;
}

template<>
__forceinline bool binary_eq<float>( const float & a, const float & b )
{
    return reinterpret_cast<const uint32_t &>(a) == reinterpret_cast<const uint32_t &>(b);
}

template<>
__forceinline bool binary_eq<double>( const double & a, const double & b )
{
    return reinterpret_cast<const uint64_t &>(a) == reinterpret_cast<const uint64_t &>(b);
}


template<typename type>
__forceinline void binary_swap( type & a, type & b )
{
    std::swap(a, b);
}

template<>
__forceinline void binary_swap<float>( float & a, float & b )
{
    uint32_t tmp = reinterpret_cast<const uint32_t &>(a);
    reinterpret_cast<uint32_t &>(a) = reinterpret_cast<const uint32_t &>(b);
    reinterpret_cast<uint32_t &>(b) = tmp;
}

template<>
__forceinline void binary_swap<double>( double & a, double & b )
{
    uint64_t tmp = reinterpret_cast<const uint64_t &>(a);
    reinterpret_cast<uint64_t &>(a) = reinterpret_cast<const uint64_t &>(b);
    reinterpret_cast<uint64_t &>(b) = tmp;
}


__forceinline unsigned get_max_set_bit( unsigned val )
{
#if defined(_M_IX86)
    __asm bsr eax, val
#elif defined(_M_X64)
    //if (val == 0) // this would more right but not as BSR
    //    return -1;
    unsigned bit = 0;
    if (val > 0xFFFF) bit += 16, val >>= 16;
    if (val > 0x00FF) bit +=  8, val >>=  8;
    if (val > 0x000F) bit +=  4, val >>=  4;
    if (val > 0x0003) bit +=  2, val >>=  2;
    if (val > 0x0001) bit +=  1;
    return bit; 
#endif
}
