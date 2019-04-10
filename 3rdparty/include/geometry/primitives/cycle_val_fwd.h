/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/primitives/range_fwd.h"
#include "geometry/primitives/rectangle_fwd.h"

namespace geom
{
    template < class >   struct cycle_value;

    struct degree180
    {
        static double min() { return -180. ; }
        static double max() { return  180. ; }
        static double def() { return    0. ; }
    } ;

    typedef cycle_value<degree180>    degree180_value ;
}