/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "reflection/proc/io_streams_refl.h"
// #include "reflection/proc/prop_attr.h"
#include "serialization/io_streams_fwd.h"

namespace geom
{
    template<class S, class processor>
    REFL_STRUCT_BODY(point_t<S, 2>)
        REFL_ENTRY(x)
        REFL_ENTRY(y)
    REFL_END()
    
    template<class S, class processor>
    REFL_STRUCT_BODY(point_t<S, 3>)
        REFL_ENTRY(x)
        REFL_ENTRY(y)
        REFL_ENTRY(z)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(point_t<S, 4>)
        REFL_ENTRY(x)
        REFL_ENTRY(y)
        REFL_ENTRY(z)
        REFL_ENTRY(w)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(color_t<S>)
        REFL_ENTRY(r)
        REFL_ENTRY(g)
        REFL_ENTRY(b)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(colora_t<S>)
        REFL_ENTRY(r)
        REFL_ENTRY(g)
        REFL_ENTRY(b)
        REFL_ENTRY(a)
    REFL_END()

    template<class S, size_t D, class processor>
    REFL_STRUCT_BODY(segment_t<S, D>)
        REFL_ENTRY(p0_)
        REFL_ENTRY(p1_)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(cpr_t<S>)
        REFL_ENTRY(course)
        REFL_ENTRY(pitch )
        REFL_ENTRY(roll  )
    REFL_END()
    
    template<class S, class processor>
    REFL_STRUCT_BODY(dcpr_t<S>)
        REFL_ENTRY(dcourse)
        REFL_ENTRY(dpitch )
        REFL_ENTRY(droll  )
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(polar_point_t<S, 2>)
        REFL_ENTRY(range)
        REFL_ENTRY(course)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(polar_point_t<S, 3>)
        REFL_ENTRY(range)
        REFL_ENTRY(course)
        REFL_ENTRY(pitch)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(quaternion_t<S>)
        REFL_ENTRY(w)
        REFL_ENTRY(v)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(range_t<S>)
        REFL_ENTRY(lo_)
        REFL_ENTRY(hi_)
    REFL_END()

    template <class value, class processor>
    REFL_STRUCT_BODY(cycle_value<value>)
        REFL_ENTRY(x)
    REFL_END()

    template <class value, class processor>
    REFL_STRUCT_BODY(range_t<cycle_value<value> >)
        REFL_ENTRY(lo_)
        REFL_ENTRY(hi_)
    REFL_END()
    
    template<class S, class processor>
    REFL_STRUCT_BODY(rectangle_t<S, 2>)
        REFL_ENTRY(x)
        REFL_ENTRY(y)
    REFL_END()

    template<class S, class processor>
    REFL_STRUCT_BODY(rectangle_t<S, 3>)
        REFL_ENTRY(x)
        REFL_ENTRY(y)
        REFL_ENTRY(z)
    REFL_END()

    template< class S, size_t D, class processor>
    REFL_STRUCT_BODY(sphere_t<S,D>)
        REFL_ENTRY(center)
        REFL_ENTRY(radius)
    REFL_END()

    struct aa_transform;
    REFL_STRUCT(aa_transform)
        REFL_ENTRY(org_ )
        REFL_ENTRY(unit_)
    REFL_END()

    REFL_STRUCT(glb_point_2)
        REFL_ENTRY(X)
        REFL_ENTRY(Y)
    REFL_END()

    REFL_STRUCT(glb_point_3)
        REFL_ENTRY(X)
        REFL_ENTRY(Y)
        REFL_ENTRY(Z)
    REFL_END()

    REFL_STRUCT(geo_point_2)
        REFL_ENTRY(lat)
        REFL_ENTRY(lon)
    REFL_END()

    REFL_STRUCT(geo_point_3)
        REFL_ENTRY(lat)
        REFL_ENTRY(lon)
        REFL_ENTRY(height)
    REFL_END()

    REFL_STRUCT(geo_triangle_2)
        REFL_ENTRY(v[0])
        REFL_ENTRY(v[1])
        REFL_ENTRY(v[2])
    REFL_END()

    REFL_STRUCT(geo_base_2)
        REFL_CHAIN(geo_point_2)
    REFL_END()

    REFL_STRUCT(geo_base_3)
        REFL_CHAIN(geo_point_3)
    REFL_END()

//     REFL_STRUCT(geo_rect_2)
//         REFL_ENTRY(lat)
//         REFL_ENTRY(lon)
//     REFL_END()
// 
//     REFL_STRUCT(geo_rect_3)
//         REFL_ENTRY(beg)
//         REFL_ENTRY(end)
//     REFL_END()

} // geom
