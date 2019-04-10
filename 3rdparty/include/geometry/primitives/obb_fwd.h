/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
    template <class Point>
        struct OBB_extents_t;

    typedef OBB_extents_t<point_3>  OBB_extents;
    typedef OBB_extents_t<point_3f> OBB_extents_f;

    template <class Point>
        struct OBB_t;

    typedef OBB_t<point_3 > OBB;
    typedef OBB_t<point_3f> OBB_f;
}
