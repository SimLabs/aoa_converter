/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

    #pragma once

#include <geometry/common.h>

#include <common/no_deduce.h>

#include "point.h"

#include "polar_point_fwd.h"

namespace geom
{
   template<typename scalar, int N> struct polar_point_t ;

   // operations
   template<typename scalar, int N> polar_point_t<scalar,N> operator * (polar_point_t<scalar,N> const & a, NO_DEDUCE(scalar) b) ;
   template<typename scalar, int N> polar_point_t<scalar,N> operator * (NO_DEDUCE(scalar) b, polar_point_t<scalar,N> const & a) ;
}

//
namespace geom
{
#pragma pack(push,1)

   template<typename scalar>
   struct polar_point_t<scalar,2>
   {
      scalar range, course;

      polar_point_t(scalar range = 1, scalar course = 0) ;

      template <class _scalar> polar_point_t(polar_point_t<_scalar,2> const & other ) ;

      polar_point_t(point_t<scalar,2> const & pt) ;
      operator point_t<scalar,2> () const ;

      polar_point_t & operator *= (scalar m ) ;
   };

   template<typename scalar>
   struct polar_point_t<scalar,3>
   {
      scalar range, course, pitch;

      polar_point_t(scalar range = 1, scalar course = 0, scalar pitch = 0) ;

      template <class _scalar> polar_point_t(polar_point_t<_scalar,3> const & other ) ;

      polar_point_t(point_t<scalar,3> const & pt) ;
      operator point_t<scalar,3> () const ;

      polar_point_t  & operator *= (scalar m) ;
   };

#pragma pack(pop)
}



/////////////////////////////////////////////////////////////////////////
// Implementation
namespace geom
{
   template<typename S>
   polar_point_t<S,2>::polar_point_t(S range, S course)
      : range(range)
      , course(course)
   {
   }

   template <class S> template <class _S>
   polar_point_t<S,2>::polar_point_t(polar_point_t<_S,2> const & other )
      : range  ( (S)other.range  )
      , course ( (S)other.course )
   {
   }

   template <class S>
   polar_point_t<S,2>::polar_point_t(point_t<S,2> const & pt)
      : range  ( (S)geom::sqrt(sqr(pt.x) + sqr(pt.y)) )
      , course ( (S)rad2grad(atan2(pt.x, pt.y)) )
   {
   }

   template <class S>
   polar_point_t<S,2>::operator point_t<S,2> () const
   {
      return point_t<S,2>( (S)range * sin(grad2rad(course)),
                           (S)range * cos(grad2rad(course)));
   }

   template <class S>
   polar_point_t<S,2> & polar_point_t<S,2>::operator *= (S m )
   {
      range *= m;
      return *this;
   }

   //
   template<typename S>
   polar_point_t<S,3>::polar_point_t(S range, S course, S pitch)
      : range (range)
      , course(course)
      , pitch (pitch)
   {
   }

   template <class S> template <class _S>
   polar_point_t<S,3>::polar_point_t(polar_point_t<_S,3> const & other )
      : range  ( (S)other.range  )
      , course ( (S)other.course )
      , pitch  ( (S)other.pitch  )
   {
   }

   template <class S>
   polar_point_t<S,3>::polar_point_t(point_t<S,3> const & pt)
   {
      S n2d = (S)(sqr(pt.x) + sqr(pt.y));

      range   = (S)geom::sqrt (n2d + (S)sqr(pt.z));
      course  = (S)rad2grad(atan2 (pt.x, pt.y));
      pitch   = (S)rad2grad(atan2 ((S)pt.z, geom::sqrt(n2d) ));
   }

   template <class S>
   polar_point_t<S,3>::operator point_t<S,3> () const
   {
      S st = cos(grad2rad(pitch));

      return point_t<S,3>( (S) range * st * sin(grad2rad(course)),
                           (S) range * st * cos(grad2rad(course)),
                           (S) range * sin(grad2rad(pitch)));
   }

   template <class S>
   polar_point_t<S,3> & polar_point_t<S,3>::operator *= (S m )
   {
      range *= m;
      return *this;
   }

   // operations
   template<typename S, int N>
   polar_point_t<S,N> operator * (polar_point_t<S,N> const & a, NO_DEDUCE(S) b)
   {
      return polar_point_t<S,N>(a) *= b ;
   }

   template<typename S, int N>
   polar_point_t<S,N> operator * (NO_DEDUCE(S) b, polar_point_t<S,N> const & a)
   {
      return polar_point_t<S,N>(a) *= b ;
   }

   template<typename S>
   polar_point_t<S,2> blend(polar_point_t<S,2> const & a, polar_point_t<S,2> const & b, double t)
   {
      return polar_point_t<S,2> ( blend(a.range, b.range, t), norm180(b.course + norm180(a.course-b.course)*t) ) ;
   }

   template<typename S>
   polar_point_t<S,3> blend(polar_point_t<S,3> const & a, polar_point_t<S,3> const & b, double t)
   {
      return polar_point_t<S,3> ( blend(a.range, b.range, t),
                                  norm180(b.course + norm180(a.course-b.course)*t),
                                  norm180(b.pitch + norm180(a.pitch-b.pitch)*t) ) ;
   }

   template<typename S>
   bool eq( polar_point_t<S,2> const & a, polar_point_t<S,2> const & b, S eps = epsilon<S>() )
   {
      return eq_zero(norm180(a.course-b.course), eps) && eq(a.range, b.range, eps) ;
   }

   template<typename S>
   bool eq( polar_point_t<S,3> const & a, polar_point_t<S,3> const & b, S eps = epsilon<S>() )
   {
      return eq_zero(norm180(a.course-b.course), eps) && eq_zero(norm180(a.pitch-b.pitch), eps) && eq(a.range, b.range, eps) ;
   }
}
