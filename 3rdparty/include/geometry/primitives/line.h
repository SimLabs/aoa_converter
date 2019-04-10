/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "line_fwd.h"

#include <common/no_deduce.h>
#include <geometry/common.h>

#include "point.h"

namespace geom
{
   #define MAX_POINT(scalar_a, scalar_b, dim)   point_t< MAX_TYPE(scalar_a, scalar_b), dim >
   #define MAX_LINE(scalar_a, scalar_b, dim)    line_t< MAX_TYPE(scalar_a, scalar_b), dim >

   namespace line
   {
      struct by_points_tag    {};
      struct by_direction_tag {};

      const by_points_tag    by_points      = by_points_tag   ();
      const by_direction_tag by_direction   = by_direction_tag();
   }

   #pragma pack (push, 1)
   template < class S, size_t D >
      struct line_t
   {
      typedef S scalar_type;
      enum  { dimension = D };

      line_t ( ) ;
      line_t ( point_t<S, D> const &p, point_t<S, D> const &r ) ;
      line_t ( point_t<S, D> const &p, point_t<S, D> const &r, line::by_direction_tag ) ;
      line_t ( point_t<S, D> const &p1, point_t<S, D> const &p2, line::by_points_tag );

      template <class Scalar>
         line_t( line_t<Scalar, D> const& l );

      explicit line_t ( line_t<S, D+1> const& l );

      point_t<S, D> operator () ( S t ) const ;

      S operator () ( point_t<S, D> const &p ) const ;
      S operator () ( line_t const &l ) const ;

      point_t<S, D> closest_point ( point_t<S, D> const &p ) const ;
      point_t<S, D> closest_point ( line_t const &l ) const ;

      bool empty() const ;

      point_t<S, D> const & p() const ;
      point_t<S, D> const & r() const ;

   private:
      point_t<S, D> p_, r_;
   };
   #pragma pack (pop)

   // operations
   template < class S1, class S2, size_t D >
      bool parallel ( line_t<S1, D> const &l1, line_t<S2, D> const &l2 ) ;

   template < class S1, class S2, size_t D >
      MAX_TYPE(S1,S2) distance ( line_t<S1, D> const &l, point_t<S2, D> const & p ) ;

   template < class S1, class S2, size_t D >
      MAX_TYPE(S1,S2) distance ( point_t<S1, D> const & p, line_t<S2, D> const &l );

   template < class S1, class S2, size_t D >
      MAX_TYPE(S1,S2) distance ( line_t<S1, D> const &l1, line_t<S2, D> const &l2 );

   template < class S1, class S2 >
      bool on_right_side ( line_t<S1, 2> const &l, point_t<S2, 2> const &p );

   template < class S1, class S2 >
      bool on_left_side ( line_t<S1, 2> const &l, point_t<S2, 2> const &p );

   template < class S, size_t D >
      void find_closest_point ( line_t<S, D> const &l1, line_t<S, D> const &l2, S * t1 = NULL, S * t2 = NULL ) ;

   template < class S, size_t D >
      bool has_intersection ( line_t<S, D> const &l1, line_t<S, D> const &l2, S * t1 = NULL, S * t2 = NULL ) ;

   template < class S, size_t D >
      bool has_intersection ( line_t<S, D> const &l1, line_t<S, D> const &l2, point_t<S, D> & at ) ;

      template < class S1, class S2, size_t D >
      MAX_POINT(S1,S2,D) operator & ( line_t<S1, D> const &l1, line_t<S2, D> const &l2 );

   template < class S >
      point_t<S, 2> normal ( line_t<S, 2> const &l ) ;
}

// implemetation
namespace geom
{
   template < class S, size_t D >
      line_t<S, D>::line_t ( )
      {
      }

   template < class S, size_t D >
      line_t<S, D>::line_t ( point_t<S, D> const &p, point_t<S, D> const &r )
         : p_(p)
         , r_(r)
      {
         Assert ( eq_zero(norm(r) - 1) ) ;
      }

   template < class S, size_t D >
      line_t<S, D>::line_t ( point_t<S, D> const &p, point_t<S, D> const &r, line::by_direction_tag )
         : p_(p)
         , r_(normalized(r))
      {
      }

   template < class S, size_t D >
      line_t<S, D>::line_t ( point_t<S, D> const &p1, point_t<S, D> const &p2, line::by_points_tag )
         : p_(p1)
         , r_(normalized(p2 - p1))
      {
      }

   template < class S, size_t D > template <class Scalar>
         line_t<S, D>::line_t ( line_t<Scalar, D> const& l )
         : p_ ( l.p() )
         , r_ ( l.r() )
      {
      }

   template < class S, size_t D >
      line_t<S, D>::line_t ( line_t<S, D+1> const& l )
         : p_ ( l.p() )
         , r_ ( l.r() )
      {
      }

   template < class S, size_t D >
      point_t<S, D> line_t<S, D>::operator () ( S t ) const
      {
         return p_ + r_ * t ;
      }

   template < class S, size_t D >
      S line_t<S, D>::operator () ( point_t<S, D> const &p ) const
      {
         return (p - p_) * r_ ;
      }

   template < class S, size_t D >
      S line_t<S, D>::operator () ( line_t<S, D> const &l ) const
      {
         S t ;
         find_closest_point ( *this, l, &t, static_cast<S *>( NULL ) ) ;
         return t;
      }

   template < class S, size_t D >
      point_t<S, D> line_t<S, D>::closest_point ( point_t<S, D> const &p ) const
      {
         return (*this)( (*this)(p) ) ;
      }

   template < class S, size_t D >
      point_t<S, D> line_t<S, D>::closest_point ( line_t<S, D> const &l ) const
      {
         return (*this)( (*this)(l) ) ;
      }

   template < class S, size_t D >
      bool line_t<S, D>::empty() const
      {
         return eq_zero(r_);
      }

   template < class S, size_t D >
      point_t<S, D> const & line_t<S, D>::p() const
      {
         return p_;
      }

   template < class S, size_t D >
      point_t<S, D> const & line_t<S, D>::r() const
      {
         return r_;
      }

   // operations
   template < class S1, class S2, size_t D >
      bool parallel ( line_t<S1, D> const &l1, line_t<S2, D> const &l2 )
      {
         return eq_zero ( l1.r() ^ l2.r() );
      }

   template < class S, size_t D >
      void find_closest_point ( line_t<S, D> const &l1, line_t<S, D> const &l2, S * t1, S * t2 )
      {
         Assert ( t1 != NULL || t2 != NULL ) ;

         if ( parallel(l1, l2) )
         {
            Assert ( 0 ) ;
            if ( t1 ) *t1 = 0 ;
            if ( t2 ) *t2 = l2 (l1.p()) ;
         }
         else
         {
            // D = (p1 + r1*t1 - p2 - r2*t2)*(p1 + r1*t1 - p2 - r2*t2) -> min

            // r1*r1 = r2*r2 = 1
            // D = (p1 - p2)*(p1 - p2) + 2*(p1 - p2)*r1*t1 - 2*(p1 - p2)*r2*t2 + t1*t1 + t2*t2 - 2*r1*r2*t1*t2 -> min

            // D by t1 =  2*(p1 - p2)*r1 + 2*t1 - 2*r1*r2*t2 = 0
            // D by t2 = -2*(p1 - p2)*r2 + 2*t2 - 2*r1*r2*t1 = 0

            // (p1-p2)*r1 + t1 - r1*r2*t2 = 0
            // (p2-p1)*r2 + t2 - r1*r2*t1 = 0

            S k1 = (l1.p()-l2.p())*l1.r() ;
            S k2 = (l2.p()-l1.p())*l2.r() ;
            S k3 = l1.r()*l2.r() ;

            // k1 + t1 - k3*t2 = 0
            // k2 + t2 - k3*t1 = 0

            S d = k3*k3 - 1;

            if (t1) *t1 = (k1 + k2*k3) / d ;
            if (t2) *t2 = (k2 + k1*k3) / d ;
         }
      }

   template < class S1, class S2, size_t D >
      MAX_TYPE(S1,S2) distance ( line_t<S1, D> const &l, point_t<S2, D> const & p )
      {
         MAX_LINE(S1,S2,D)  const& ml = l ;
         MAX_POINT(S1,S2,D) const& mp = p ;

         return norm(mp - ml.closest_point(mp));
      }

   template < class S1, class S2, size_t D >
      MAX_TYPE(S1,S2) distance ( point_t<S1, D> const & p, line_t<S2, D> const &l )
      {
         return distance(l,p) ;
      }

   template < class S1, class S2 >
      MAX_TYPE(S1,S2) distance ( line_t<S1, 2> const &l1, line_t<S2, 2> const &l2 )
      {
         MAX_LINE(S1,S2,2) const& ml1 = l1 ;
         MAX_LINE(S1,S2,2) const& ml2 = l2 ;

         return parallel(ml1, ml2) ? distance ( ml1, ml2.p() ) : 0 ;
      }

   template < class S1, class S2, size_t D >
      MAX_TYPE(S1,S2) distance ( line_t<S1, D> const &l1, line_t<S2, D> const &l2 )
      {
         MAX_LINE(S1,S2,D) const& ml1 = l1 ;
         MAX_LINE(S1,S2,D) const& ml2 = l2 ;

         if ( parallel(ml1, ml2) )
            return distance ( ml1, ml2.p() ) ;

         MAX_TYPE(S1,S2) t1, t2 ;
         find_closest_point ( ml1, ml2, &t1, &t2 ) ;

         return norm(ml1(t1) - ml2(t2)) ;
      }

   template < class S >
      bool has_intersection ( line_t<S, 2> const &l1, line_t<S, 2> const &l2, S * t1 = NULL, S * t2 = NULL )
      {
         S n = l1.r() ^ l2.r();
         if ( ! eq_zero ( n ) )
         {
            point_t<S, 2> d = l1.p() - l2.p() ;
            if ( t1 ) *t1 = (l2.r() ^ d) / n;
            if ( t2 ) *t2 = (l1.r() ^ d) / n;
            return true ;
         }
         return false ;
      }

   template < class S >
      bool has_intersection ( line_t<S, 3> const &l1, line_t<S, 3> const &l2, S * t1 = NULL, S * t2 = NULL )
      {
         point_t<S, 3> n = l1.r() ^ l2.r() ;
         if ( ! eq_zero ( n ) )
         {
            point_t<S, 3> d = l1.p() - l2.p() ;
            if ( eq_zero ( n * d ) )
            {
               if ( t1 ) *t1 = ((l2.r() ^ d) * n) / (n*n) ;
               if ( t2 ) *t2 = ((l1.r() ^ d) * n) / (n*n) ;

               return true ;
            }
         }
         return false ;
      }

   template < class S, size_t D >
      bool has_intersection ( line_t<S, D> const &l1, line_t<S, D> const &l2, point_t<S, D> & at )
      {
         S t ;
         if ( has_intersection(l1, l2, &t ) )
         {
            at = l1(t) ;
            return true ;
         }
         return false ;
      }

   template < class S1, class S2, size_t D >
      MAX_POINT(S1,S2,D) operator & ( line_t<S1, D> const &l1, line_t<S2, D> const &l2 )
      {
         MAX_LINE(S1,S2,D) const& ml1 = l1 ;
         MAX_LINE(S1,S2,D) const& ml2 = l2 ;

         MAX_TYPE(S1,S2) t ;
         if ( has_intersection ( ml1, ml2, &t ) )
            return ml1(t) ;

         Assert ( 0 ) ;
         return ml1.p() ;
      }

   template < class S1, class S2 >
      bool on_right_side ( line_t<S1, 2> const &l, point_t<S2, 2> const &p )
      {
         return (l.r() ^ (p - l.p())) <= 0;
      }

   template < class S1, class S2 >
      bool on_left_side ( line_t<S1, 2> const &l, point_t<S2, 2> const &p )
      {
         return (l.r() ^ (p - l.p())) >= 0;
      }

   template < class S >
      point_t<S, 2> normal ( line_t<S, 2> const &l )
      {
         return point_t<S, 2> ( -l.r().y, l.r().x ) ;
      }

   #undef MAX_POINT
   #undef MAX_LINE
}

