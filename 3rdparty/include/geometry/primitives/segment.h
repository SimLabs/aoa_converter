/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "segment_fwd.h"

#include "common/no_deduce.h"
#include "geometry/common.h"

#include "point.h"

#define MAX_POINT(scalar_a, scalar_b, dim)   point_t< MAX_TYPE(scalar_a, scalar_b), dim >
#define MAX_SEGMENT(scalar_a, scalar_b, dim) segment_t< MAX_TYPE(scalar_a, scalar_b), dim >

namespace geom
{
   #pragma pack (push, 1)
   template < class S, size_t D>
      struct segment_t
      {
         typedef S scalar_type;
         enum  { dimension = D };
         typedef point_t<S, D> point_type ;

         typedef point_t<S,D> point_t ;

         segment_t(point_t const &p0, point_t const &p1) ;
         segment_t() ;

         template <class Scalar>
            segment_t( segment_t<Scalar, D> const& s );

         explicit segment_t ( segment_t<S, D+1> const& s );

         point_t operator () (S t) const ;
         S       operator () (point_t const &p) const ;

      public:
         bool contains ( point_t const &p, S eps=epsilon<S>() ) const ;

         point_t closest_point ( point_t const &p ) const ;
         point_t closest_point ( segment_t const &s ) const ;

         point_t const &P0() const ;
         point_t const &P1() const ;

         point_t const & operator [] (int idx) const
         {
            Assert(idx == 0 || idx == 1);
            return idx == 0 ? P0() : P1();
         }

         point_t & operator [] (int idx)
         {
            Assert(idx == 0 || idx == 1);
            return idx == 0 ? p0_ : p1_;
         }

         // for serialization 
         template<class Ss, size_t Dd, class processor> friend void reflect2(processor&, segment_t<Ss, Dd> const&, segment_t<Ss, Dd> const&);

      // not implemented yet
      private:
         S       operator () (segment_t const &s) const ;

      private:
         point_t   p0_, p1_;
      };
   #pragma pack (pop)

   //template < class S, size_t D >
   //   __forceinline bool operator == ( segment_t< S, D > const & a, segment_t< S, D > const & b );

   // operations
   template <class S, size_t D>
      segment_t<S, D> opposite(segment_t<S, D> const & s) ;

   template <class S1, class S2, size_t D>
      MAX_TYPE(S1, S2) distance_sqr(segment_t<S1, D> const &s, point_t<S2, D> const &p ) ;

   template <class S1, class S2, size_t D>
      MAX_TYPE(S1, S2) distance_sqr(point_t<S1, D> const &p, segment_t<S2, D> const &s );

   template <class S1, class S2, size_t D>
      MAX_TYPE(S1, S2) distance(segment_t<S1, D> const &s, point_t<S2, D> const &p);

   template <class S1, class S2, size_t D>
      MAX_TYPE(S1, S2) distance(point_t< S1, D > const &p, segment_t< S2, D > const &s ) ;

   template <class S>
      point_t< S, 2> normal( segment_t<S, 2> const &s);

   template <class S, size_t D>
      point_t< S, D> direction( segment_t<S, D> const &s);

   template <class S, size_t D>
      point_t< S, D> rand ( segment_t<S, D> const &s);

   template <class S, size_t D>
      point_t< S, D> center( segment_t<S, D> const &s);

   template <class S, size_t D>
      S length( segment_t<S, D> const & s );

   template <class S, size_t D>
      S length_sqr( segment_t<S, D> const & s );

   template <class S, size_t D>
      bool is_finite( segment_t<S, D> const & s );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementation
namespace geom
{
   template <class S, size_t D>
      __forceinline segment_t<S,D>::segment_t(point_t const &p0, point_t const &p1)
         : p0_(p0)
         , p1_(p1)
      {}

   template <class S, size_t D>
      __forceinline segment_t<S,D>::segment_t()
      {}

   template <class S, size_t D> template <class Scalar>
      __forceinline segment_t<S,D>::segment_t( segment_t<Scalar, D> const& s )
         : p0_(s.P0())
         , p1_(s.P1())
      {}

   template <class S, size_t D>
      __forceinline segment_t<S,D>::segment_t ( segment_t<S, D+1> const& s )
         : p0_(s.P0())
         , p1_(s.P1())
      {}

   template <class S, size_t D>
      __forceinline point_t<S,D> segment_t<S,D>::operator () (S t) const
      {
         return (1 - t)*p0_ + t*p1_;
      }

   template <class S, size_t D>
      __forceinline S segment_t<S,D>::operator () (point_t const &p) const
      {
         if ( eq(p0_, p1_) )
            return 0;

         point_t v = p1_ - p0_;
         point_t w = p - p0_;

         return (w * v) / (v * v) ;
      }

//   template <class S, size_t D>
//      __forceinline S segment_t<S,D>::operator () (segment_t<S,D> const &s) const
//      {
//         char a[-1] ;
//      }

   template <class S, size_t D>
      __forceinline bool segment_t<S,D>::contains ( point_t const &p , S eps ) const
      {
         return eq_rel ( closest_point(p), p, eps ) ;
      }

   template <class S, size_t D>
      __forceinline point_t<S,D> segment_t<S,D>::closest_point ( point_t const &p ) const
      {
         if ( eq(p0_, p1_) )
            return p0_ ;

         return (*this)(bound<S>((*this)(p), 0, 1)) ;
      }

   template <class S, size_t D>
      __forceinline point_t<S,D> segment_t<S,D>::closest_point ( segment_t<S,D> const &s ) const
      {
         if ( eq(p0_, p1_) )
            return p0_ ;

         return (*this)(bound<S>((*this)(s), 0, 1)) ;
      }

   template <class S, size_t D>
      __forceinline point_t<S,D> const &segment_t<S,D>::P0() const
      {
         return p0_;
      }

   template <class S, size_t D>
      __forceinline point_t<S,D> const &segment_t<S,D>::P1() const
      {
         return p1_;
      }

   template < class S, size_t D >
      __forceinline bool operator == ( segment_t< S, D > const & a, segment_t< S, D > const & b )
   {
      return a.P0() == b.P0() && a.P1() == b.P1() || a.P0() == b.P1() && a.P1() == b.P0();
   }

   // operations
   template <class S, size_t D>
      __forceinline segment_t<S, D> opposite(segment_t<S, D> const & s)
   {
      return segment_t<S, D>(s.P1(),s.P0());
   }

   template < class S1, class S2, size_t D >
      __forceinline MAX_TYPE(S1, S2) distance_sqr(segment_t<S1, D> const &s, point_t<S2, D> const &p)
      {
         return distance_sqr( static_cast<MAX_SEGMENT(S1,S2,D) const&>(s).closest_point(p), p ) ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_TYPE(S1, S2) distance_sqr(point_t<S1, D> const &p, segment_t<S2, D> const &s )
      {
         return distance_sqr(s,p);
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_TYPE(S1, S2) distance(segment_t<S1, D> const &s, point_t<S2, D> const &p)
      {
         return geom::sqrt(distance_sqr(s,p));
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_TYPE(S1, S2) distance(point_t<S1, D> const &p, segment_t<S2, D> const &s )
      {
         return geom::sqrt(distance_sqr(s,p));
      }

   template <class S >
      __forceinline point_t<S, 2> normal( segment_t<S, 2> const &s)
      {
         point_t< S, 2 > p = s.P1() - s.P0() ;
         return point_t<S, 2>(-p.y, p.x);
      }

   template <class S, size_t D>
      __forceinline point_t<S, D> direction( segment_t<S, D> const &s)
      {
         return s.P1() - s.P0();
      }

   template <class S, size_t D>
      __forceinline point_t< S, D> center( segment_t<S, D> const &s)
      {
         return (s.P0() + s.P1()) / S (2);
      }

   template <class S, size_t D>
      __forceinline point_t< S, D> rand ( segment_t<S, D> const &s)
      {
         return s ( rand((S)1) ) ;
      }

   template <class S, size_t D>
      __forceinline S length( segment_t<S, D> const & s )
      {
         return distance( s.P0(), s.P1() );
      }

   template <class S, size_t D>
      __forceinline S length_sqr( segment_t<S, D> const & s )
      {
         return distance_sqr( s.P0(), s.P1() );
      }

   template <class S, size_t D>
      __forceinline bool is_finite( segment_t<S, D> const & s )
      {
         return is_finite( s[0] ) && is_finite( s[1] );
      }
}

#undef MAX_POINT
#undef MAX_SEGMENT
