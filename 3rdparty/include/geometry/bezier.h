/*
    * Copyright (C) 2015 SimLabs LLC - All rights reserved.
    * Unauthorized copying of this file or its part is strictly prohibited.
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{

template<typename S, typename T>
struct bezier3_t
{
    typedef S scalar_type;
    typedef T point_type;
    typedef array<point_type, 4> points_type;

    bezier3_t(points_type const &points)
        : points_(points)
    {}

    bezier3_t(points_type const &p0, points_type const &p1, points_type const &p2, points_type const &p3)
        : points_({p0, p1, p2, p3})
    {}

    point_type operator()(scalar_type t) const
    {
        scalar_type omt = scalar_type(1) - t;

        return omt * omt * omt * points_[0] 
            + scalar_type(3) * omt * omt * t * points_[1]
            + scalar_type(3) * omt * t * t * points_[2]
            + t * t * t * points_[3];
    }

    points_type const &points() const
    {
        return points_;
    }

private:
    points_type points_;
};

template<typename T>
using bezier3_f = bezier3_t<float, T>;

template<typename T>
using bezier3 = bezier3_t<double, T>;

} // namespace geom