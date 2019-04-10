#pragma once

#include "sphere_intersect.h"

namespace geom
{
    template< class S, size_t Dim >
    bool ray_intersection(point_t< S, Dim > const& p0, point_t< S, Dim > const& p1, arc_t< S, Dim > const& arc, S & ratio, S * ratio_next = NULL)
    {
        typedef point_t< S, Dim > point_type;
        typedef segment_t< S, Dim > segment_type;

        double r0, r1;
        if (ray_intersection(p0, p1, arc.sphere, r0, &r1))
        {
            segment_type seg(p0, p1);
            point_type isect0 = seg(r0);
            point_type isect1 = seg(r1);
            if (arc.contains(isect0))
            {
                ratio = r0;
                if (ratio_next)
                    *ratio_next = arc.contains(isect1) ? r1 : r0;
                return true;
            }
            else if (arc.contains(isect1))
            { 
                ratio = r1;
                if (ratio_next)
                    *ratio_next = r1;
                return true;
            }
        }

        return false;
    }
}

