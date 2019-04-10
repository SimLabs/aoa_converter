/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "obb_fwd.h"
#include "rectangle.h"
#include "geometry/polygon/convex_hull.h"

namespace geom
{
#pragma pack (push, 1)

template< typename Point >
   struct OBB_extents_t
{
   OBB_extents_t(){}

   OBB_extents_t( Point const &s1, Point const &s2, Point const &s3 )
   {
      extents_[0] = s1;
      extents_[1] = s2;
      extents_[2] = s3;
   }

   Point const & operator()( size_t i ) const { Assert(i < 3); return extents_[i] ; }

   void scale( typename Point::scalar_type s )
   {
      extents_[0] *= s;
      extents_[1] *= s;
      extents_[2] *= s;
   }

private:
   Point extents_[3];
};

template <class Point>
   struct OBB_t
{
   OBB_t ()
   {}

   OBB_t (Point const &org, Point const &s1, Point const &s2, Point const &s3)
      :   origin (org)
      ,   extents(s1, s2, s3)
   {}

   OBB_t (Point const &org, OBB_extents_t<Point> const & ext)
      :   origin (org)
      ,   extents(ext)
   {}

   OBB_t ( Point const *pts, size_t p_num )
   {
      typedef typename Point::scalar_type    scalar_type;

      if ( p_num < 3 )
      {
         Point center( 0, 0, 0 );
         for ( size_t i = 0; i != p_num; ++i )
            center += pts[i];

         origin = center / (float)p_num;

         extents =  OBB_extents_t<Point>
                        (  Point( pts[0].x - origin.x, 0, 0 ),
                           Point( 0, pts[0].y - origin.y, 0 ),
                           Point( 0, 0, pts[0].z - origin.z ) );
         return;
      }

      // build naa rect
      std::vector<geom::point_2> const v2( pts, pts + p_num );

      geom::naa_rect_2 naa_rect;
      geom::build_naa_rect( &v2[0], v2.size(), naa_rect );

      rectangle_2 rect  = naa_rect.getrect ();
      double      angle = naa_rect.getangle();

      double z_min = pts[0].z;
      double z_max = pts[0].z;
      for ( size_t i = 1; i != p_num; ++i )
      {
         geom::make_min<double>( z_min, pts[i].z );
         geom::make_max<double>( z_max, pts[i].z );
      }

      // calc halfsides
      double const sina = sin(angle);
      double const cosa = cos(angle);

      double const x_len =    rect.x.size() / 2;
      double const y_len =    rect.y.size() / 2;
      double const z_len = (z_max - z_min) / 2;

      point_2 const p1 = rect.lo();
      point_2 const p2 = rect.hi();

      point_2 const p1_( p1.x*cosa - p1.y*sina, p1.x*sina + p1.y*cosa );
      point_2 const p2_( p2.x*cosa - p2.y*sina, p2.x*sina + p2.y*cosa );

      origin  = Point( (p1_+p2_)/2, (scalar_type)(z_max+z_min)/2 );

      extents =  OBB_extents_t<Point>
                  ( Point( (scalar_type) cosa, (scalar_type)sina,  0 ) * x_len,
                    Point( (scalar_type)-sina, (scalar_type)cosa,  0 ) * y_len,
                    Point(                   0,                0,  1 ) * z_len );
   }

   OBB_t& unite( OBB_t const & other )
   {
      Point points[16];

      for (size_t i = 0; i < 8; i++)
         points[i] = vertex(*this, i);

      for (size_t i = 0; i < 8; i++)
         points[8 + i] = vertex(other, i);

      *this = OBB_t (points, 16);

      return *this;
   }

   static size_t face_count() { return 3; }

   __forceinline Point corner( int i ) const
   {
      switch ( i )
      {
      case 0: return origin - extents(0) - extents(1) - extents(2);
      case 1: return origin + extents(0) - extents(1) - extents(2);
      case 2: return origin - extents(0) + extents(1) - extents(2);
      case 3: return origin + extents(0) + extents(1) - extents(2);
      case 4: return origin - extents(0) - extents(1) + extents(2);
      case 5: return origin + extents(0) - extents(1) + extents(2);
      case 6: return origin - extents(0) + extents(1) + extents(2);
      case 7: return origin + extents(0) + extents(1) + extents(2);
      default: Assert( !"Invalid index" ); return corner(0);
      };
   }


   Point                 origin;
   OBB_extents_t<Point>  extents;
};
#pragma pack (pop)

template<class Point>
inline geom::rectangle_3 OBB2AABB( OBB_t<Point> const& obb )
{
   point_3 a = obb.origin, b = obb.origin;
   for( int i = 0; i != 3; ++i )
   {
      point_3 const p( obb.extents( i ) );

      for( int j = 0; j != 3; ++j )
      {
            const double d = geom::abs( p[ j ] );
            a[ j ] -= d;
            b[ j ] += d;
      }
   }
   return geom::rectangle_3( a, b );
}

inline OBB AABB2OBB( rectangle_3 const & aabb )
{
   point_3 const c = aabb.center();
   point_3 const u = aabb.XYZ() - c;
   return OBB( c, point_3( u.x, 0., 0. ),
                  point_3( 0., u.y, 0. ),
                  point_3( 0., 0., u.z ) );
}

template <class Point>
OBB_t<Point> & scale( OBB_t<Point> & obb, typename Point::scalar_type s )
{
   obb.extents.scale(s);
   return obb;
}

} // end of namespace geom
