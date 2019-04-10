/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geo_rect_fwd.h"

namespace geom
{
    bool has_intersection( geo_rect_2 const& a, geo_rect_2 const& b ) ;
    bool has_intersection( geo_rect_3 const& a, geo_rect_3 const& b ) ;

    bool eq( geo_rect_2 const & a, geo_rect_2 const & b, double eps );
    bool eq( geo_rect_3 const & a, geo_rect_3 const & b, double eps );

    geo_rect_2 bounding ( geo_point_2 const * points, size_t count ) ;
    geo_rect_3 bounding ( geo_point_3 const * points, size_t count ) ;
    geo_rect_2 bounding ( geo_segment_2 const& seg );
    geo_rect_2 bounding ( geo_triangle_2 const& tr );

    geo_rect_2 operator |  ( geo_rect_2 const& r1, geo_rect_2 const& r2 );
}

namespace geom
{
#pragma pack (push,1)

    struct geo_rect_2
    {
        geo_rect_2 ();
        geo_rect_2 ( range_2 const& lat, degree180_range const& lon );

        geo_point_2 center() const;
        bool contains( geo_point_2 const& p ) const;

        geo_rect_2& operator |= ( geo_point_2 const& p );
        geo_rect_2& operator |= ( geo_rect_2 const& p );

        range_2         lat;
        degree180_range lon;
    };

    struct geo_rect_3
    {
        geo_point_3 beg, end;

        geo_rect_3 ()
        {
        }
        geo_rect_3 ( double lat0, double lon0, double height0, double lat1, double lon1, double height1 )
            : beg ( lat0, lon0, height0 )
            , end ( lat1, lon1, height1 )
        {
        }

        geo_rect_3 ( const geo_point_3& p0, const geo_point_3& p1 )
            : beg ( p0 )
            , end ( p1 )
        {
        }

        geo_point_3 center() const
        {
            return blend ( beg, end, 0.5 ) ;
        }

        bool contains( geo_point_2 const& p ) const
        {
            if ( !geom::range_2( beg.lat, end.lat ).contains( p.lat ) )
                return false ;
            if ( !geom::range_2( beg.lon, end.lon ).contains( p.lon ) )
                return false ;

            return true ;
        }

        bool contains( geo_point_3 const& p ) const
        {
            if ( !contains(p.as_2d()) )
                return false ;
            if ( !geom::range_2( beg.height, end.height ).contains( p.height ) )
                return false ;

            return true ;
        }

        geo_rect_3& operator |= ( geo_point_3 const& p )
        {
            make_min(beg.lat,    p.lat) ;
            make_min(beg.lon,    p.lon) ;
            make_min(beg.height, p.height) ;
            make_max(end.lat,    p.lat) ;
            make_max(end.lon,    p.lon) ;
            make_max(end.height, p.height) ;

            return *this ;
        }
    };

#pragma pack (pop)
}

// Implementation
namespace geom
{
    inline geo_rect_2::geo_rect_2 ()
    {
    }
    inline geo_rect_2::geo_rect_2 ( range_2 const& lat, degree180_range const& lon )
        : lat ( lat )
        , lon ( lon )
    {
    }

    inline geo_point_2 geo_rect_2::center() const
    {
        return geo_point_2(lat.center(), lon.center()) ;
    }

    inline bool geo_rect_2::contains( geo_point_2 const& p ) const
    {
        return lat.contains(p.lat) && lon.contains(p.lon);
    }

    inline geo_rect_2& geo_rect_2::operator |= ( geo_point_2 const& p )
    {
        lat |= p.lat;
        lon |= p.lon;

        return *this ;
    }

    inline geo_rect_2& geo_rect_2::operator |= ( geo_rect_2 const& r )
    {
        lat |= r.lat;
        lon |= r.lon;

        return *this ;
    }



    inline bool eq( geo_rect_2 const & a, geo_rect_2 const & b, double eps = epsilon<double>() )
    {
        return eq ( a.lat, b.lat, eps ) && eq ( a.lon, b.lon, eps ) ;
    }

    inline bool eq( geo_rect_3 const & a, geo_rect_3 const & b, double eps = epsilon<double>() )
    {
        return eq ( a.beg, b.beg, eps ) && eq ( a.end, b.end, eps ) ;
    }

    inline bool has_intersection( geo_rect_2 const& a, geo_rect_2 const& b )
    {
        return has_intersection(a.lat, b.lat) && has_intersection(a.lon, b.lon) ;
    }

    inline bool has_intersection( geo_rect_3 const& a, geo_rect_3 const& b )
    {
        return has_intersection( range_2(a.beg.lat, a.end.lat), range_2(b.beg.lat, b.end.lat) ) &&
            has_intersection( range_2(a.beg.lon, a.end.lon), range_2(b.beg.lon, b.end.lon) ) &&
            has_intersection( range_2(a.beg.height, a.end.height), range_2(b.beg.height, b.end.height) ) ;
    }

    inline geo_rect_2 bounding ( geo_point_2 const * points, size_t count )
    {
        geo_rect_2 rect;
        for ( size_t i = 0 ; i < count ; i ++ )
            rect |= points[i];
        return rect ;
    }

    inline geo_rect_3 bounding ( geo_point_3 const * points, size_t count )
    {
        geo_rect_3 rect ( points[0], points[0] ) ;
        for ( size_t i = 1 ; i < count ; i ++ )
        {
            rect.beg.lat    = min(rect.beg.lat,    points[i].lat) ;
            rect.beg.lon    = min(rect.beg.lon,    points[i].lon) ;
            rect.beg.height = min(rect.beg.height, points[i].height) ;
            rect.end.lat    = max(rect.end.lat,    points[i].lat) ;
            rect.end.lon    = max(rect.end.lon,    points[i].lon) ;
            rect.end.height = max(rect.end.height, points[i].height) ;
        }
        return rect ;
    }

    inline geo_rect_2 bounding ( geo_segment_2 const& seg )
    {
        Assert(seg.small);

        point_3 an = polar_point_3(1., 90 - seg.p0.lon, seg.p0.lat);
        point_3 bn = polar_point_3(1., 90 - seg.p1.lon, seg.p1.lat);

        point_3 n = an ^ bn;
        point_3 side = point_3(0,0,1) ^ n;
        point_3 x = n ^ side; // increase lat direction

        range_2 lat(seg.p0.lat, seg.p1.lat);

        if ((an*side) > 0 && (bn*side) < 0)
            lat |= polar_point_3(x).pitch;
        else if ((an*side) < 0 && (bn*side) > 0)
            lat |= polar_point_3(-x).pitch;

        geom::degree180_range lon;
        if (path(geom::degree180_value(seg.p0.lon), geom::degree180_value(seg.p1.lon)) <= 180.)
            lon = geom::degree180_range(seg.p0.lon, seg.p1.lon);
        else
            lon = geom::degree180_range(seg.p1.lon, seg.p0.lon);

        return geo_rect_2(lat, lon);
    }

    inline geo_rect_2 bounding ( geo_triangle_2 const& tr )
    {
        point_3 n[3];

        for (size_t i = 0; i < 3; ++ i)
            n[i] = polar_point_3(1., 90 - tr.v[i].lon, tr.v[i].lat);

        geo_rect_2 rc;
        for (size_t i = 0; i < 3; ++ i)
            rc |= bounding(geo_segment_2(tr.v[i], tr.v[(i+1) % 3], true));

        return rc;
    }

    inline geo_rect_2 operator |  ( geo_rect_2 const& r1, geo_rect_2 const& r2 )
    {
        return geo_rect_2(r1) |= r2;
    }


}
