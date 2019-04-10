/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "reflection/reflection_ext.h"

// font descriptor
struct font_descr
{
    std::string name;
    unsigned size;
    bool italic;
    int bold;

    font_descr() 
        : name("Times")
        , size(10)
        , italic(false)
        , bold(50)
    {}

    font_descr( std::string const & name, unsigned size, bool italic = false, int bold = 50 ) 
        : name(name)
        , size(size)
        , italic(italic)
        , bold(bold)
    {}

    bool operator < ( const font_descr & sec ) const
    {
        if (name != sec.name)
            return name < sec.name;
        if (size != sec.size)
            return size < sec.size;
        if (italic != sec.italic)
            return !italic;
        if (bold != sec.bold)
            return bold < sec.bold;
        return false;
    }

    bool operator != ( const font_descr & sec ) const
    {
        return (*this < sec || sec < *this);
    }

    bool operator == (const font_descr & sec) const
    {
        return !(*this != sec);
    }
};

REFL_STRUCT(font_descr)
    REFL_ENTRY(name)
    REFL_ENTRY(size)
    REFL_ENTRY(italic)
    REFL_ENTRY(bold)
REFL_END()
