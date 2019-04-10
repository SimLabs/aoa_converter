/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "crc.h"


template<typename type>
struct DataCRC
{
    __forceinline DataCRC() : crc(0) {}

    __forceinline unsigned get_crc() const { return crc; }

    __forceinline void actualize_crc()
    {
        crc = 0;
        crc = cyclic_redundancy_check(this, sizeof(type), 0);
    }


private:

    unsigned crc;
};

template<typename T>
__forceinline bool operator == ( const DataCRC<T> & a, const DataCRC<T> & b )
{
    return a.get_crc() == b.get_crc();
}

template<typename T>
__forceinline bool operator != ( const DataCRC<T> & a, const DataCRC<T> & b )
{
    return a.get_crc() != b.get_crc();
}
