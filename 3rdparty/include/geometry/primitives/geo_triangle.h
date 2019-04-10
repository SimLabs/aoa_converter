/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geo_triangle_fwd.h"
#include "geometry/geo/geo_utils.h"

namespace geom
{
}

namespace geom
{
#pragma pack (push,1)

struct geo_triangle_2
{
    geo_point_2 v[3];

    geo_triangle_2 ()
    {}

    geo_triangle_2 ( const geo_point_2& p0, const geo_point_2& p1, const geo_point_2& p2 )
    {
        v[0] = p0, v[1] = p1, v[2] = p2;
    }

    inline bool contains( geo_point_2 const& p ) const;
};

#pragma pack (pop)


// Implementation

bool geo_triangle_2::contains( geo_point_2 const& p ) const
{
    return on_left_side(v[0], v[1], p) && on_left_side(v[1], v[2], p) && on_left_side(v[2], v[0], p);
}

}