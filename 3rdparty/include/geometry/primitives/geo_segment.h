/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geo_segment_fwd.h"
#include "geometry/geo/geo_utils.h"

namespace geom
{
}

namespace geom
{
#pragma pack (push,1)

// ATL workaround (it defines type 'small'... thanks god, it defines and not typedefs :)
#undef small

struct geo_segment_2
{
    geo_point_2 p0;
    geo_point_2 p1;
    bool small;

    geo_segment_2 ();
    geo_segment_2 ( const geo_point_2& p0, const geo_point_2& p1, bool small );

    geo_point_2 operator () (double t) const ;
    double operator () (geo_point_2 const& p) const ;
    geo_point_2 closest_point(geo_point_2 const&p) const;
    double length() const;
};

struct geo_segment_3
{
    geo_point_3 p0;
    geo_point_3 p1;
    bool small;

    geo_segment_3 ();
    geo_segment_3 ( const geo_point_3& p0, const geo_point_3& p1, bool small );

    geo_point_3 operator () (double t) const ;
};

#pragma pack (pop)


inline geo_segment_2::geo_segment_2 ()
{}

inline geo_segment_2::geo_segment_2 ( const geo_point_2& p0, const geo_point_2& p1, bool small )
    : p0(p0)
    , p1(p1)
    , small(small)
{
}

inline geo_point_2 geo_segment_2::operator () (double t) const
{
    return geom::blend(p0, p1, t);
}


inline double geo_segment_2::operator () (geo_point_2 const& p) const
{
    double dst = distance(p0, p1);
    if (dst < 10000)
    {
        return bound(segment_2(point_2(), geo_base_2(p0)(p1))(geo_base_2(p0)(p)), 0., 1.);
    }
    else
    {
        point_3 na = polar_point_3(p0);
        point_3 nb = polar_point_3(p1);
        point_3 np = polar_point_3(p);
        point_3 n = geom::normalized_safe(na^nb);
        return bound(asin(bound((na^np)*n, -1., 1.)) / asin(bound((na^nb)*n, -1., 1.)), 0., 1.);
    }
}

inline geo_point_2 geo_segment_2::closest_point(geo_point_2 const&p) const
{
    return geo_closest_point(p0, p1, p);
}

inline double geo_segment_2::length() const
{
    return geom::distance(p0, p1);
}


inline geo_segment_3::geo_segment_3 ()
{}

inline geo_segment_3::geo_segment_3 ( const geo_point_3& p0, const geo_point_3& p1, bool small )
    : p0(p0)
    , p1(p1)
    , small(small)
{
}

inline geo_point_3 geo_segment_3::operator () (double t) const
{
    return geom::blend(p0, p1, t);
}


inline optional<geo_point_2> intersection(geo_segment_2 const& seg0, geo_segment_2 const& seg1)
{
    point_3 n00 = polar_point_3(seg0.p0);
    point_3 n01 = polar_point_3(seg0.p1);
    point_3 n10 = polar_point_3(seg1.p0);
    point_3 n11 = polar_point_3(seg1.p1);

    point_3 n0 = geom::normalized_safe(n00^n01);
    point_3 n1 = geom::normalized_safe(n10^n11);

    double a0 = asin(bound((n00 ^ n01) * n0, -1., 1.));
    double a1 = asin(bound((n10 ^ n11) * n1, -1., 1.));

    point_3 n = geom::normalized_safe(n0 ^ n1);

    double b0 = acos(bound(n00 * n, -1., 1.));
    double b1 = acos(bound(n10 * n, -1., 1.));

    if ((n00 ^ n)*n0 < 0)
        b0 *= -1;
    if ((n10 ^ n)*n1 < 0)
        b1 *= -1;

    if (b0 >= 0 && b0 <= a0 && b1 >= 0 && b1 <= a1)
        return geo_point_2(polar_point_3(n));

    else
    {
        point_3 n = -geom::normalized_safe(n0 ^ n1);

        double b0 = acos(bound(n00 * n, -1., 1.));
        double b1 = acos(bound(n10 * n, -1., 1.));

        if ((n00 ^ n)*n0 < 0)
            b0 *= -1;
        if ((n10 ^ n)*n1 < 0)
            b1 *= -1;

        if (b0 >= 0 && b0 <= a0 && b1 >= 0 && b1 <= a1)
            return geo_point_2(polar_point_3(n));
    }

    return boost::none;
}


}

namespace geom
{
}
