/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "geometry/primitives/obb_fwd.h"
#include "common/util.h"

namespace geom
{
namespace details
{
    template<class Point>
    range_t<typename Point::scalar_type> projection_to_axis(OBB_t<Point> const& p, Point const& axis)
    {
        typedef typename Point::scalar_type Scalar;
        Scalar halfLength = geom::abs(p.extents(0) * axis) + geom::abs(p.extents(1) * axis) + geom::abs(p.extents(2) * axis);
        Scalar center = p.origin * axis;

        return range_t<Scalar>(center - halfLength, center + halfLength );
    }

    template<class S, size_t D>
    range_t<S> projection_to_axis(segment_t<S, D> const& seg, point_t<S,D> const& axis)
    {
        return range_t<S>(seg.P0() * axis, seg.P1() * axis) ;
    }

    template<class S, size_t D>
    range_t<S> projection_to_axis(rectangle_t<S, D> const& rc, point_t<S,D> const& axis)
    {
        S halfLength = 0;
        for ( size_t n = 0; n != D; ++n )
        {
            point_t<S,D> e;
            e[n] = rc[n].size() / 2;
            halfLength += fabs(e * axis);
        }

        S center = rc.center() * axis;

        return range_t<S>(center - halfLength, center + halfLength );
    }

}


template<class S>
bool has_intersection(segment_t<S, 2> const &seg, rectangle_t<S,2> const &rc)
{
    typedef S Scalar;
    typedef point_t<S,2> point_type;
    typedef range_t<S>     range_type;

    point_type axes[] = {geom::normal(seg), point_2(0,1), point_2(1, 0)};

    for(size_t i = 0; i < util::array_size(axes); ++i)
    {            
        range_type proj1 = details::projection_to_axis(seg, axes[i]);
        range_type proj2 = details::projection_to_axis(rc, axes[i]);

        if (!geom::has_intersection(proj1, proj2))
            return false;
    }

    return true;
}

template<class S>
bool has_intersection(segment_t<S, 3> const &seg, rectangle_t<S,3> const &rc)
{
    typedef S Scalar;
    typedef point_t<S,3> point_type;
    typedef range_t<S>     range_type;

    std::vector<point_type> axes;
    for (size_t i = 0; i < 3; ++i)
    {
        point_type d;
        d[i] = 1;
        axes.push_back(d);
        axes.push_back(d ^ (seg.P1() - seg.P0()));
    }

    for(size_t i = 0; i < axes.size(); ++i)
    {            
        range_type proj1 = details::projection_to_axis(seg, axes[i]);
        range_type proj2 = details::projection_to_axis(rc, axes[i]);

        if (!geom::has_intersection(proj1, proj2))
            return false;
    }

    return true;
}

template<class S, size_t D>
bool has_intersection(rectangle_t<S,D> const &rc, segment_t<S, D> const &seg)
{
    return has_intersection(seg, rc);
}

}
