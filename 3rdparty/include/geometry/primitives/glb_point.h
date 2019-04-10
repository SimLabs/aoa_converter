/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "glb_point_fwd.h"

#include "range.h"
#include "point.h"

#include <geometry/common.h>

namespace geom
{
   struct glb_point_2 ;
   struct glb_point_3 ;
   struct glb_rect_2 ;
   struct glb_rect_3 ;

   struct glb_pos_2 ;

   glb_point_2 blend (glb_point_2 const & a, glb_point_2 const & b, double t) ;
   glb_point_3 blend (glb_point_3 const & a, glb_point_3 const & b, double t) ;

   bool operator == ( glb_point_2 const & a, glb_point_2 const & b );
   bool operator == ( glb_point_3 const & a, glb_point_3 const & b );

   bool operator != ( glb_point_2 const & a, glb_point_2 const & b );
   bool operator != ( glb_point_3 const & a, glb_point_3 const & b );

   bool eq( glb_point_2 const & a, glb_point_2 const & b, double eps = epsilon< double >( ) );
   bool eq( glb_point_3 const & a, glb_point_3 const & b, double eps = epsilon< double >( ) );

   bool eq( glb_rect_2 const & a, glb_rect_2 const & b, double eps = epsilon< double >( ) );
   bool eq( glb_rect_3 const & a, glb_rect_3 const & b, double eps = epsilon< double >( ) );

   bool has_intersection( glb_rect_2 const& a, glb_rect_2 const& b ) ;
   bool has_intersection( glb_rect_3 const& a, glb_rect_3 const& b ) ;

   glb_rect_2 bounds ( glb_point_2 const * points, size_t count ) ;

   point_2 as_metric(glb_point_2 const & gp ) ;
   point_3 as_metric(glb_point_3 const & gp ) ;

   glb_point_2 as_glb(point_2 const & mp ) ;
   glb_point_3 as_glb(point_3 const & mp ) ;

   bool line_cross_rect ( glb_point_2 const& p0, glb_point_2 const& p1, glb_rect_2 const& rect ) ;
   bool point_in_zone ( glb_point_2 const& p, glb_point_2 const * points, size_t count ) ;
}


namespace geom
{
   #pragma pack (push,1)

   struct glb_point_2
   {
      double X, Y ;

      glb_point_2 ()                     : X ( 0   ), Y ( 0   ) {}
      glb_point_2 ( double X, double Y ) : X ( X   ), Y ( Y   ) {}

      glb_point_2 const& as_2d () const { return *this ; }
      glb_point_2 &      as_2d ()       { return *this ; }
   } ;

   struct glb_point_3 : glb_point_2
   {
      double Z ;

      glb_point_3 ()                                   : Z ( 0 ) {}
      glb_point_3 ( double X, double Y, double Z )     : glb_point_2 ( X, Y ), Z ( Z ) {}
      glb_point_3 ( const glb_point_2& p, double Z   ) : glb_point_2 ( p ), Z ( Z ) {}

      glb_point_3 const& as_3d () const { return *this ; }
      glb_point_3 &      as_3d ()       { return *this ; }
   } ;

   struct glb_pos_2 : glb_point_2
   {
      double course ;

      glb_pos_2 ()                                        : course ( 0 ) {}
      glb_pos_2 ( double X, double Y, double course )     : glb_point_2 ( X, Y ), course ( course ) {}
      glb_pos_2 ( const glb_point_2& p, double course   ) : glb_point_2 ( p ), course ( course ) {}
   } ;

   struct glb_rect_2
   {
      glb_point_2 beg, end;

      glb_rect_2 ()
      {
      }
      glb_rect_2 ( double X0, double Y0, double X1, double Y1 )
         : beg ( X0, Y0 )
         , end ( X1, Y1 )
      {
      }

      glb_rect_2 ( const glb_point_2& p0, const glb_point_2& p1 )
         : beg ( p0 )
         , end ( p1 )
      {
      }

      glb_point_2 center() const
      {
         return blend ( beg, end, 0.5 ) ;
      }

      point_2 size() const
      {
         return point_2 ( end.X-beg.X, end.Y-beg.Y ) ;
      }

      bool contains( glb_point_2 const& p ) const
      {
         if ( !geom::range_2( beg.X, end.X ).contains( p.X ) )
            return false ;
         if ( !geom::range_2( beg.Y, end.Y ).contains( p.Y ) )
            return false ;

         return true ;
      }

      glb_rect_2& operator |= ( glb_point_2 const& p )
      {
         make_min(beg.X, p.X) ;
         make_min(beg.Y, p.Y) ;
         make_max(end.X, p.X) ;
         make_max(end.Y, p.Y) ;

         return *this ;
      }
   };

   struct glb_rect_3
   {
      glb_point_3 beg, end;

      glb_rect_3 ()
      {
      }
      glb_rect_3 ( double X0, double Y0, double Z0, double X1, double Y1, double Z1 )
         : beg ( X0, Y0, Z0 )
         , end ( X1, Y1, Z1 )
      {
      }

      glb_rect_3 ( const glb_point_3& p0, const glb_point_3& p1 )
         : beg ( p0 )
         , end ( p1 )
      {
      }

      glb_point_3 center() const
      {
         return blend ( beg, end, 0.5 ) ;
      }

      point_3 size() const
      {
         return point_3 ( end.X-beg.X, end.Y-beg.Y, end.Z-beg.Z ) ;
      }

      bool contains( glb_point_2 const& p ) const
      {
         if ( !geom::range_2( beg.X, end.X ).contains( p.X ) )
            return false ;
         if ( !geom::range_2( beg.Y, end.Y ).contains( p.Y ) )
            return false ;

         return true ;
      }

      bool contains( glb_point_3 const& p ) const
      {
         if ( !contains(p.as_2d()) )
            return false ;
         if ( !geom::range_2( beg.Z, end.Z ).contains( p.Z ) )
            return false ;

         return true ;
      }

      glb_rect_3& operator |= ( glb_point_3 const& p )
      {
         make_min(beg.X, p.X) ;
         make_min(beg.Y, p.Y) ;
         make_min(beg.Z, p.Z) ;
         make_max(end.X, p.X) ;
         make_max(end.Y, p.Y) ;
         make_max(end.Z, p.Z) ;

         return *this ;
      }
   };


   #pragma pack (pop)
}


//////////////////////////////////////////////////////////////////////////
// Implementation
namespace geom
{
   inline glb_point_2 blend(glb_point_2 const & a, glb_point_2 const & b, double t)
   {
      return glb_point_2 ( blend(a.X, b.X, t), blend(a.Y, b.Y, t) ) ;
   }

   inline glb_point_3 blend(glb_point_3 const & a, glb_point_3 const & b, double t)
   {
      return glb_point_3 ( blend(a.as_2d(), b.as_2d(), t), blend(a.Z, b.Z, t) ) ;
   }

   inline bool operator == ( glb_point_2 const & a, glb_point_2 const & b )
   {
      return a.X == b.X && a.Y == b.Y ; 
   }

   inline bool operator == ( glb_point_3 const & a, glb_point_3 const & b )
   {
      return a.X == b.X && a.Y == b.Y && a.Z == b.Z; 
   }

   inline bool operator != ( glb_point_2 const & a, glb_point_2 const & b )
   {
      return a.X != b.X || a.Y != b.Y ; 
   }

   inline bool operator != ( glb_point_3 const & a, glb_point_3 const & b )
   {
      return a.X != b.X || a.Y != b.Y || a.Z != b.Z; 
   }

   inline bool eq( glb_point_2 const & a, glb_point_2 const & b, double eps )
   {
      return eq( a.X, b.X, eps )
          && eq( a.Y, b.Y, eps );
   }

   inline bool eq( glb_point_3 const & a, glb_point_3 const & b, double eps )
   {
      return eq( a.X,    b.X,    eps )
          && eq( a.Y,    b.Y,    eps )
          && eq( a.Z, b.Z, eps );
   }

   inline bool eq( glb_rect_2 const & a, glb_rect_2 const & b, double eps )
   {
      return eq ( a.beg, b.beg, eps ) && eq ( a.end, b.end, eps ) ;
   }

   inline bool eq( glb_rect_3 const & a, glb_rect_3 const & b, double eps )
   {
      return eq ( a.beg, b.beg, eps ) && eq ( a.end, b.end, eps ) ;
   }

   inline bool has_intersection( glb_rect_2 const& a, glb_rect_2 const& b )
   {
      return has_intersection( range_2(a.beg.X, a.end.X), range_2(b.beg.X, b.end.X) ) &&
             has_intersection( range_2(a.beg.Y, a.end.Y), range_2(b.beg.Y, b.end.Y) ) ;
   }

   inline bool has_intersection( glb_rect_3 const& a, glb_rect_3 const& b )
   {
      return has_intersection( range_2(a.beg.X, a.end.X), range_2(b.beg.X, b.end.X) ) &&
             has_intersection( range_2(a.beg.Y, a.end.Y), range_2(b.beg.Y, b.end.Y) ) &&
             has_intersection( range_2(a.beg.Z, a.end.Z), range_2(b.beg.Z, b.end.Z) ) ;
   }

   inline glb_rect_2 bounds ( glb_point_2 const * points, size_t count )
   {
      glb_rect_2 rect ( points[0], points[0] ) ;
      for ( size_t i = 1 ; i < count ; i ++ )
      {
         rect.beg.X = min(rect.beg.X, points[i].X) ;
         rect.beg.Y = min(rect.beg.Y, points[i].Y) ;
         rect.end.X = max(rect.end.X, points[i].X) ;
         rect.end.Y = max(rect.end.Y, points[i].Y) ;
      }
      return rect ;
   }

   inline glb_rect_3 bounds ( glb_point_3 const * points, size_t count )
   {
      glb_rect_3 rect ( points[0], points[0] ) ;
      for ( size_t i = 1 ; i < count ; i ++ )
      {
         rect.beg.X = min(rect.beg.X, points[i].X) ;
         rect.beg.Y = min(rect.beg.Y, points[i].Y) ;
         rect.beg.Z = min(rect.beg.Z, points[i].Z) ;
         rect.end.X = max(rect.end.X, points[i].X) ;
         rect.end.Y = max(rect.end.Y, points[i].Y) ;
         rect.end.Z = max(rect.end.Z, points[i].Z) ;
      }
      return rect ;
   }

   inline point_2 as_metric(glb_point_2 const & gp )
   {
      return point_2 ( gp.X, gp.Y ) ;
   }

   inline point_3 as_metric(glb_point_3 const & gp )
   {
      return point_3 ( gp.X, gp.Y, gp.Z ) ;
   }

   inline glb_point_2 as_glb(point_2 const & mp )
   {
      return glb_point_2 ( mp.x, mp.y ) ;
   }

   inline glb_point_3 as_glb(point_3 const & mp )
   {
      return glb_point_3 ( mp.x, mp.y, mp.z ) ;
   }

   inline bool line_cross_rect ( glb_point_2 const& p0, glb_point_2 const& p1, glb_rect_2 const& rect )
   {
      if (  p0.X < rect.beg.X && p1.X < rect.beg.X
         || p0.X > rect.end.X && p1.X > rect.end.X
         || p0.Y < rect.beg.Y && p1.Y < rect.beg.Y
         || p0.Y > rect.end.Y && p1.Y > rect.end.Y )
         return false ;

      double nX = - (p1.Y - p0.Y) ;
      double nY = + (p1.X - p0.X) ;

      double dX0 = rect.beg.X - p0.X ;
      double dY0 = rect.beg.Y - p0.Y ;
      double dX1 = rect.end.X - p0.X ;
      double dY1 = rect.end.Y - p0.Y ;

      double d0 = dX0 * nX + dY0 * nY ;
      double d1 = dX1 * nX + dY0 * nY ;
      double d2 = dX1 * nX + dY1 * nY ;
      double d3 = dX0 * nX + dY1 * nY ;

      if ( d0 > 0 && d1 > 0 && d2 > 0 && d3 > 0 )
         return false ;

      if ( d0 < 0 && d1 < 0 && d2 < 0 && d3 < 0 )
         return false ;

      return true ;
   }

   inline bool point_in_zone ( glb_point_2 const& p, glb_point_2 const * points, size_t count )
   {
      if ( !count )
         return false ;

      double X = p.X ;
      double Y = p.Y ;

      size_t horCrossings = 0;
      size_t verCrossings = 0;

      // note. were don't care of result, if point is on the edge of polygon
      for( size_t i = 0; i < count; i++)
      {
         const glb_point_2 &A = points[i];
         const glb_point_2 &B = points[(i+1)%count];

         if ( (Y >= A.Y && Y < B.Y || Y <= A.Y && Y > B.Y) && (A.X > X || B.X > X) )
         {
            double yy = A.X + (Y - A.Y)*(B.X - A.X)/(B.Y - A.Y) - X;
            if ( yy > 0 )
               verCrossings ++ ;
         }

         if ( (X >= A.X && X < B.X || X <= A.X && X > B.X) && (A.Y > Y || B.Y > Y) )
         {
            double xx = A.Y + (X - A.X)*(B.Y - A.Y)/(B.X - A.X) - Y ;
            if ( xx > 0 )
               horCrossings ++ ;
         }
      }

      return ((horCrossings & 1) && (verCrossings & 1));
   }

}
