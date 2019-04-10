/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/intersection/segments_intersection.h"

namespace geom
{

template<class points_t> bool polyline_has_intersection(points_t const& prim_points, bool is_closed)
{
    std::vector<point_2> points;
    geom::rotation_3 rot(cpr(prim_points.front().lon, prim_points.front().lat, 0.));
    geom::rectangle_2 bound;
    for (size_t i = 0; i < prim_points.size(); ++i)
    {
        point_3 n = polar_point_3(1., prim_points[i].lon, prim_points[i].lat);
        polar_point_3 nr = n * rot;
        points.push_back(point_2(nr.course, nr.pitch)) ;
        bound |= points.back();
    }

    bool has_intersection = false;
    geom::segments_intersection cdt(bound);
    for (size_t i = 0; i+1 < points.size(); ++i)
    {
        point_2 dir = geom::normalized_safe(points[i+1] - points[i]);
        geom::segment_2 seg_to_check(points[i]+dir*geom::epsilon<float>(), points[i+1]);
        if (cdt.has_intersection(seg_to_check))
        {
            has_intersection = true;
            break;
        }
        cdt.add_segment(geom::segment_2(points[i], points[i+1]));
    }

    if (has_intersection)
        return true;

    if (is_closed)
    {
        point_2 dir = geom::normalized_safe(points.front() - points.back());
        geom::segment_2 seg_to_check(points.back()+dir*geom::epsilon<float>(), points.front()-dir*geom::epsilon<float>());
        if (cdt.has_intersection(seg_to_check))
            return true;
    }

    return false;
}

} // namespace geom