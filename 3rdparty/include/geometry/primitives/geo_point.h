/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geo_point_fwd.h"
#include "range.h"
#include "point.h"
#include "polar_point.h"

#include "geometry/common.h"
#include "geo_rect_fwd.h"

namespace geom
{
    geo_point_2 move_ref (geo_point_2 const & from, geo_point_2 const & to, geo_point_2 const & ref) ;

    bool eq( geo_point_2 const & a, geo_point_2 const & b, double eps );
    bool eq( geo_point_3 const & a, geo_point_3 const & b, double eps );

    point_2 as_metric(geo_point_2 const & gp ) ;
    point_3 as_metric(geo_point_3 const & gp ) ;

    geo_point_2 as_geo(point_2 const & mp ) ;
    geo_point_3 as_geo(point_3 const & mp ) ;

    bool line_cross_rect ( geo_point_2 const& p0, geo_point_2 const& p1, geo_rect_2 const& rect ) ;
    bool point_in_zone ( geo_point_2 const& p, geo_point_2 const * points, size_t count ) ;
}


namespace geom
{
#pragma pack (push,1)

    struct geo_point_2
    {
        double lat, lon ;

        geo_point_2 ()                         : lat ( 0     ), lon ( 0     ) {}
        geo_point_2 ( double lat, double lon ) : lat ( lat   ), lon ( lon   ) {}

        geo_point_2 const& as_2d () const { return *this ; }
        geo_point_2 &      as_2d ()       { return *this ; }

        explicit geo_point_2(polar_point_3 const& p) ;
        operator polar_point_3 () const ;
    } ;

    struct geo_point_3 : geo_point_2
    {
        double height ;

        geo_point_3 ()                                        : height ( 0 ) {}
        geo_point_3 ( double lat, double lon, double height ) : geo_point_2 ( lat, lon ), height ( height ) {}
        geo_point_3 ( const geo_point_2& p, double height   ) : geo_point_2 ( p ), height ( height ) {}

        geo_point_3 const& as_3d () const { return *this ; }
        geo_point_3 &      as_3d ()       { return *this ; }
    } ;

    struct geo_pos_2 : geo_point_2
    {
        double course ;

        geo_pos_2 ()                                        : course ( 0 ) {}
        geo_pos_2 ( double lat, double lon, double course ) : geo_point_2 ( lat, lon ), course ( course ) {}
        geo_pos_2 ( const geo_point_2& p, double course   ) : geo_point_2 ( p ), course ( course ) {}
    } ;

#pragma pack (pop)
}


//////////////////////////////////////////////////////////////////////////
// Implementation
namespace geom
{
    inline geo_point_2::geo_point_2(polar_point_3 const& p)
        : lat(p.pitch)
        , lon(-p.course)
    {

    }
    inline geo_point_2::operator polar_point_3 () const
    {
        return polar_point_3(1, -lon, lat);
    }

    inline geo_point_2 move_ref (geo_point_2 const & from, geo_point_2 const & to, geo_point_2 const & ref)
    {
        return geo_point_2 ( from.lat + to.lat - ref.lat, from.lon + to.lon - ref.lon ) ;
    }

    inline bool eq( geo_point_2 const & a, geo_point_2 const & b, double eps = epsilon< double >( ) )
    {
        return eq( a.lat, b.lat, eps )
            && eq( a.lon, b.lon, eps );
    }

    inline bool eq( geo_point_3 const & a, geo_point_3 const & b, double eps = epsilon< double >( ) )
    {
        return eq( a.lat,    b.lat,    eps )
            && eq( a.lon,    b.lon,    eps )
            && eq( a.height, b.height, eps );
    }

    inline point_2 as_metric(geo_point_2 const & gp )
    {
        return point_2 ( gp.lon, gp.lat ) ;
    }

    inline point_3 as_metric(geo_point_3 const & gp )
    {
        return point_3 ( gp.lon, gp.lat, gp.height ) ;
    }

    inline geo_point_2 as_geo(point_2 const & mp )
    {
        return geo_point_2 ( mp.y, mp.x ) ;
    }

    inline geo_point_3 as_geo(point_3 const & mp )
    {
        return geo_point_3 ( mp.y, mp.x, mp.z ) ;
    }

    __forceinline bool operator < ( geo_point_2 const & a, geo_point_2 const & b )
    {
        if (a.lat != b.lat)
            return a.lat < b.lat;
        if (a.lon != b.lon)
            return a.lon < b.lon;
        return false ;
    }

    __forceinline bool operator < ( geo_point_3 const & a, geo_point_3 const & b )
    {
        if (a.lat != b.lat)
            return a.lat < b.lat;
        if (a.lon != b.lon)
            return a.lon < b.lon;
        if (a.height != b.height)
            return a.height < b.height;
        return false ;
    }

    __forceinline bool operator < ( geo_pos_2 const & a, geo_pos_2 const & b )
    {
        if (a.lat != b.lat)
            return a.lat < b.lat;
        if (a.lon != b.lon)
            return a.lon < b.lon;
        if (a.course != b.course)
            return a.course < b.course;
        return false ;
    }


//     inline bool line_cross_rect ( geo_point_2 const& p0, geo_point_2 const& p1, geo_rect_2 const& rect )
//     {
//         if (  p0.lat < rect.beg.lat && p1.lat < rect.beg.lat
//             || p0.lat > rect.end.lat && p1.lat > rect.end.lat
//             || p0.lon < rect.beg.lon && p1.lon < rect.beg.lon
//             || p0.lon > rect.end.lon && p1.lon > rect.end.lon )
//             return false ;
// 
//         double nlat = - (p1.lon - p0.lon) ;
//         double nlon = + (p1.lat - p0.lat) ;
// 
//         double dlat0 = rect.beg.lat - p0.lat ;
//         double dlon0 = rect.beg.lon - p0.lon ;
//         double dlat1 = rect.end.lat - p0.lat ;
//         double dlon1 = rect.end.lon - p0.lon ;
// 
//         double d0 = dlat0 * nlat + dlon0 * nlon ;
//         double d1 = dlat1 * nlat + dlon0 * nlon ;
//         double d2 = dlat1 * nlat + dlon1 * nlon ;
//         double d3 = dlat0 * nlat + dlon1 * nlon ;
// 
//         if ( d0 > 0 && d1 > 0 && d2 > 0 && d3 > 0 )
//             return false ;
// 
//         if ( d0 < 0 && d1 < 0 && d2 < 0 && d3 < 0 )
//             return false ;
// 
//         return true ;
//     }

//     inline bool point_in_zone ( geo_point_2 const& p, geo_point_2 const * points, size_t count )
//     {
//         if ( !count )
//             return false ;
// 
//         double lat = p.lat ;
//         double lon = p.lon ;
// 
//         int horCrossings = 0;
//         int verCrossings = 0;
// 
//         // note. were don't care of result, if point is on the edge of polygon
//         for( size_t i = 0; i < count; i++)
//         {
//             const geo_point_2 &A = points[i];
//             const geo_point_2 &B = points[(i+1)%count];
// 
//             if ( (lon >= A.lon && lon < B.lon || lon <= A.lon && lon > B.lon) && (A.lat > lat || B.lat > lat) )
//             {
//                 double yy = A.lat + (lon - A.lon)*(B.lat - A.lat)/(B.lon - A.lon) - lat;
//                 if ( yy > 0 )
//                     verCrossings ++ ;
//             }
// 
//             if ( (lat >= A.lat && lat < B.lat || lat <= A.lat && lat > B.lat) && (A.lon > lon || B.lon > lon) )
//             {
//                 double xx = A.lon + (lat - A.lat)*(B.lon - A.lon)/(B.lat - A.lat) - lon ;
//                 if ( xx > 0 )
//                     horCrossings ++ ;
//             }
//         }
// 
//         return ((horCrossings & 1) && (verCrossings & 1));
//     }

}
