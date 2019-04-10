/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once


// base routine
inline unsigned cyclic_redundancy_check( const void * pData, unsigned nSize, unsigned uStartCRC = 0 )
{
    static const unsigned g_nTableSize = 256;
    static unsigned g_auTable[g_nTableSize];
    static bool g_bTableFilled = false;

    // first fill table
    if (!g_bTableFilled)
    {
        for (unsigned i = 0; i < g_nTableSize; ++i)
        {
            unsigned uTableAccum = i << 24;
            for (unsigned j = 0; j < 8; ++j)
            {
                static const unsigned g_nPolynom = 0x04C11DB7;

                if (uTableAccum & 0x80000000L)
                    uTableAccum = (uTableAccum << 1) ^ g_nPolynom;
                else
                    uTableAccum <<= 1;
            }
            g_auTable[i] = uTableAccum;
        }
        g_bTableFilled = true;
    }

    // calculate CRC
    const unsigned char * pBytePtr = (const unsigned char *)pData;
    while (nSize-- > 0)
    {
        unsigned nTableIdx = (uStartCRC >> 24) ^ (*pBytePtr++);
        Assert(nTableIdx < 256);
        uStartCRC = (uStartCRC << 8) ^ g_auTable[nTableIdx];
    }

    // exit
    return uStartCRC;
}
