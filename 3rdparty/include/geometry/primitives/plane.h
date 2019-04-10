/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "plane_fwd.h"

#include <common/no_deduce.h>
#include <geometry/common.h>

#include "point.h"
#include "line.h"
#include "segment.h"


namespace geom
{
   #define MAX_LINE(S1, S2) line_t< MAX_TYPE(S1, S2), 3 >
   #define MAX_POINT(S1, S2) point_t< MAX_TYPE(S1, S2), 3 >

   #define MAX_TYPE_3(S1, S2, S3) MAX_TYPE(MAX_TYPE(S1, S2), S3)
   #define MAX_POINT_3(S1, S2, S3) point_t< MAX_TYPE_3(S1, S2, S3), 3 >

   //
   #pragma pack (push, 1)
   template<typename S>
      struct plane_t
   {
      inline plane_t ( ) ;
      inline plane_t ( point_t<S,3> const& n, S d ) ;
      inline plane_t ( point_t<S,3> const& n, point_t<S,3> const& p ) ;

      template<typename S1>
         inline plane_t( plane_t<S1> const& other ) ;

      inline point_t<S,3> const& n ( void ) const ;
      inline S const& d ( void ) const ;

      inline S operator() ( point_t<S,3> const& p ) const ;

      inline plane_t& offset ( S d ) ;
      inline plane_t& offset ( point_t<S,3> const &offset ) ;


   private:

      point_t<S,3> n_;
      S d_;

      template<typename> friend struct plane_t;
   };
   #pragma pack (pop)


   //
   // operations
   //

   // distance
   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( plane_t<S1> const& p1, plane_t<S2> const& p2 );

   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( plane_t<S1> const& p, line_t<S2, 3> const& l );

   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( line_t<S1, 3> const& l, plane_t<S2> const& p );

   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( plane_t<S1> const& p, point_t<S2, 3> const& v );

   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( point_t<S1, 3> const& v, plane_t<S2> const& p );

   // intersection of two planes
   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p1, plane_t<S2> const& p2, MAX_LINE(S1, S2) *l = NULL );

   // intersection of three planes
   template<typename S1, typename S2, typename S3>
      bool has_intersection( plane_t<S1> const& p1, plane_t<S2> const& p2, plane_t<S3> const& p3, MAX_POINT_3(S1, S2, S3) *v = NULL );

   // intersection with line
   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, line_t<S2, 3> const& l, MAX_TYPE(S1, S2) *t = NULL );

   template<typename S1, typename S2>
      bool has_intersection( line_t<S1, 3> const& l, plane_t<S2> const& p, MAX_TYPE(S1, S2) *t = NULL );

   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, line_t<S2, 3> const& l, MAX_POINT(S1, S2) *v );

   template<typename S1, typename S2>
      bool has_intersection( line_t<S1, 3> const& l, plane_t<S2> const& p, MAX_POINT(S1, S2) *v );

   // intersection with segment
   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, segment_t<S2, 3> const& l, MAX_TYPE(S1, S2) *t = NULL );

   template<typename S1, typename S2>
      bool is_inside( plane_t<S1> const& p, segment_t<S2, 3> const& l );

   template<typename S1, typename S2>
      bool has_intersection( segment_t<S1, 3> const& l, plane_t<S2> const& p, MAX_TYPE(S1, S2) *t = NULL );

   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, segment_t<S2, 3> const& l, MAX_POINT(S1, S2) *v );

   template<typename S1, typename S2>
      bool has_intersection( segment_t<S1, 3> const& l, plane_t<S2> const& p, MAX_POINT(S1, S2) *v );

   // projection
   template<typename S1, typename S2>
      MAX_POINT(S1, S2) orthogonal_projection( plane_t<S1> const& p, point_t<S2,3> const& v );
}

// implementation
namespace geom
{
   template<typename S>
      plane_t<S>::plane_t ( )
      : n_(0, 0, 1)
      , d_(0)
   {
   }

   template<typename S>
      plane_t<S>::plane_t ( point_t<S,3> const& n, S d )
      : n_(n)
      , d_(d)
   {
      //Assert(eq_zero(norm(n) - 1)); not work with float
   }

   template<typename S>
      plane_t<S>::plane_t ( point_t<S,3> const& n, point_t<S,3> const& p )
      : n_(n)
      , d_(-(n_ * p))
   {
      //Assert(eq_zero(norm(n) - 1)); not work with float
   }

   template<typename S> template<typename S1>
      plane_t<S>::plane_t ( plane_t<S1> const& p )
      : n_(p.n_)
      , d_((S)p.d_)
   {
   }

   template<typename S>
      point_t<S,3> const& plane_t<S>::n ( void ) const
   {
      return n_;
   }

   template<typename S>
      S const& plane_t<S>::d ( void ) const
   {
      return d_;
   }

   template<typename S>
      S plane_t<S>::operator() ( point_t<S,3> const& p ) const
   {
      return n_ * p + d_;
   }

   template<typename S>
   plane_t<S>& plane_t<S>::offset ( S d )
   {
      d_ -= d ;

      return *this ;
   }

   template<typename S>
   plane_t<S>& plane_t<S>::offset ( point_t<S,3> const& offset )
   {
      d_ -= n_ * offset ;

      return *this ;
   }


   //
   // operations
   //

   // distance
   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( plane_t<S1> const& p1, plane_t<S2> const& p2 )
   {
      const MAX_TYPE(S1, S2) dot = p1.n() * p2.n();

      if (eq(dot, MAX_TYPE(S1, S2)(1)))
         return abs(p1.d() - p2.d());
      else if (eq(dot, MAX_TYPE(S1, S2)(-1)))
         return abs(p1.d() + p2.d());
      else
         return 0; // intersected
   }

   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( plane_t<S1> const& p, line_t<S2, 3> const& l )
   {
      const MAX_TYPE(S1, S2) dot = p.n() * l.r();

      if (!eq_zero(dot))
         return 0; // intersected

      return distance(p, l.p());
   }

   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( line_t<S1, 3> const& l, plane_t<S2> const& p )
   {
      return distance(p, l);
   }


   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( plane_t<S1> const& p, point_t<S2, 3> const& v )
   {
      return abs(p.n() * v + p.d());
   }

   template<typename S1, typename S2>
      MAX_TYPE(S1, S2) distance( point_t<S1, 3> const& v, plane_t<S2> const& p )
   {
      return distance(p, v);
   }

   // intersection
   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p1, plane_t<S2> const& p2, MAX_LINE(S1, S2) *l )
   {
      MAX_TYPE(S1, S2) const dot_val = p1.n() * p2.n();
      if (eq(dot_val, 1))
         return false;
      if (l != NULL)
      {
         MAX_POINT(S1, S2) const dir = normalized(p1.n() ^ p2.n());
         MAX_POINT(S1, S2) const pt = (p1.n() * (-p1.d() + p2.d() * dot_val) + p2.n() * (-p2.d() + p1.d() * dot_val)) / (1 - dot_val * dot_val);
         *l = MAX_LINE(S1, S2)(pt, dir);
      }
      return true;
   }

   template<typename S1, typename S2, typename S3>
      bool has_intersection( plane_t<S1> const& p1, plane_t<S2> const& p2, plane_t<S3> const& p3, MAX_POINT_3(S1, S2, S3) *v )
   {
      const MAX_POINT_3(S1, S2, S3)
         n1(p1.n()),
         n2(p2.n()),
         n3(p3.n());

      const MAX_TYPE_3(S1, S2, S3) det = determinant(
         n1.x, n2.x, n3.x,
         n1.y, n2.y, n3.y,
         n1.z, n2.z, n3.z);

      if (eq_zero(det))
         return false; // when some planes are parallel or equal (opposite-equal)

      if (v != NULL)
      {
         const MAX_POINT_3(S1, S2, S3) p =
            (n2 ^ n3) * (-p1.d()) +
            (n3 ^ n1) * (-p2.d()) +
            (n1 ^ n2) * (-p3.d());
         *v = p * (1 / det);
      }
      return true;
   }

   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, line_t<S2, 3> const& l, MAX_TYPE(S1, S2) *t )
   {
      const MAX_TYPE(S1, S2) dot = p.n() * l.r();
      if (eq_zero(dot))
         return false;
      if (t != NULL)
         *t = -p(l.p()) / dot;
      return true;
   }

   template<typename S1, typename S2>
      bool has_intersection( line_t<S1, 3> const& l, plane_t<S2> const& p, MAX_TYPE(S1, S2) *t )
   {
      return has_intersection(p, l, t);
   }

   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, line_t<S2, 3> const& l, MAX_POINT(S1, S2) *v )
   {
      MAX_TYPE(S1, S2) t;
      bool ret = has_intersection(p, l, &t);
      *v = l(t);
      return ret;
   }

   template<typename S1, typename S2>
      bool has_intersection( line_t<S2, 3> const& l, plane_t<S1> const& p, MAX_POINT(S1, S2) *v )
   {
      return has_intersection(p, l, v);
   }

   // intersection with segment
   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, segment_t<S2, 3> const& s, MAX_TYPE(S1, S2) *t )
   {
      const MAX_TYPE(S1, S2)
         d0 = p(s.P0()),
         d1 = p(s.P1());
      if (d0 * d1 > 0)
         return false; // no intersection
      if (t != NULL)
         *t = -d0 / (d1 - d0);
      return true;
   }

   template<typename S1, typename S2>
      bool is_inside( plane_t<S1> const& p, segment_t<S2, 3> const& s )
   {
      const MAX_TYPE(S1, S2)
         d0 = p(s.P0()),
         d1 = p(s.P1());
      return d1 - d0 == 0;
   }

   template<typename S1, typename S2>
      bool has_intersection( segment_t<S1, 3> const& s, plane_t<S2> const& p, MAX_TYPE(S1, S2) *t )
   {
      return has_intersection(p, s, t);
   }

   template<typename S1, typename S2>
      bool has_intersection( plane_t<S1> const& p, segment_t<S2, 3> const& s, MAX_POINT(S1, S2) *v )
   {
      MAX_TYPE(S1, S2) t;
      if (!has_intersection(p, s, &t))
         return false;
      if (v != NULL)
         *v = s(t);
      return true;
   }

   template<typename S1, typename S2>
      bool has_intersection( segment_t<S2, 3> const& s, plane_t<S1> const& p, MAX_POINT(S1, S2) *v )
   {
      return has_intersection(p, s, v);
   }

   // projection
   template<typename S1, typename S2>
      MAX_POINT(S1, S2) orthogonal_projection( plane_t<S1> const& p, point_t<S2,3> const& v )
   {
      return v - p(v) * p.n();
   }

   #undef MAX_LINE
   #undef MAX_POINT

   #undef MAX_TYPE_3
   #undef MAX_POINT_3
}
