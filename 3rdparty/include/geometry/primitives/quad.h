/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "quad_fwd.h"

#include <common/no_deduce.h>
#include <geometry/common.h>

#include <geometry/primitives/point.h>

#define MAX_POINT(scalar_a, scalar_b, dim)   point_t< MAX_TYPE(scalar_a, scalar_b), dim >
#define MAX_QUAD(scalar_a, scalar_b, dim) quad_t< MAX_TYPE(scalar_a, scalar_b), dim >

namespace geom
{

#pragma pack (push, 1)

template < class S, size_t D >
   struct quad_t
{
   typedef S scalar_type;
   enum  { dimension = D };
   typedef point_t<S, D> point_type ;

   typedef point_t<S,D> point_t ;

   quad_t(point_t const &p0, point_t const &p1, point_t const &p2, point_t const &p3) ;
   quad_t() ;

   template <class Scalar>
      quad_t( quad_t<Scalar, D> const& t );

   explicit quad_t ( quad_t<S, D+1> const& t );

   bool contains( point_t const & p ) const;

   point_t const & operator [] (size_t index) const ;
   point_t       & operator [] (size_t index) ;

private:
   point_t   v_[4];
};

#pragma pack (pop)

//operations
/*
template <class S>
   point_t< S, 2> normal( quad_t<S, 2> const &t);

template <class S, size_t D>
   S square( quad_t<S, D> const &t) ;

template <class S, size_t D>
   bool singular ( quad_t<S, D> const &t) ;

template <class S, size_t D>
   point_t<S, D> describing_circle_center( quad_t<S, D> const &t );

template <class S, size_t D>
   point_t<S, D> inscribing_circle_center( quad_t<S, D> const &t );
*/
} // End of 'cg' namespace

/////////////////////////////////////////////////////////////////////////////////////////
// implementation
namespace geom
{
template <class S, size_t D>
   quad_t<S,D>::quad_t(point_t const &p0, point_t const &p1, point_t const& p2, point_t const &p3)
   {
      v_[0] = p0 ;
      v_[1] = p1 ;
      v_[2] = p2 ;
      v_[3] = p3 ;
   }

template <class S, size_t D>
   quad_t<S,D>::quad_t()
   {}

template <class S, size_t D> template <class Scalar>
   quad_t<S,D>::quad_t( quad_t<Scalar, D> const& v )
   {
      v_[0] = v[0];
      v_[1] = v[1];
      v_[2] = v[2];
      v_[3] = v[3];
   }

template <class S, size_t D>
   quad_t<S,D>::quad_t ( quad_t<S, D+1> const& v )
   {
      v_[0] = v[0];
      v_[1] = v[1];
      v_[2] = v[2];
      v_[3] = v[3];
   }

template <class S, size_t D>
   bool quad_t<S,D>::contains ( point_t const &p ) const
   {
      // TODO: optimize
      return triangle_t<S, D>(v_[0], v_[1], v_[2]).contains(p) ||
             triangle_t<S, D>(v_[0], v_[2], v_[3]).contains(p);
   }

template <class S, size_t D>
   point_t<S,D> const &quad_t<S,D>::operator[](size_t index) const
   {
      Assert ( index < 4 ) ;
      return v_[index];
   }

template <class S, size_t D>
   point_t<S,D> &quad_t<S,D>::operator[](size_t index)
   {
      Assert ( index < 4 ) ;
      return v_[index];
   }

//operations
   /*
template <class S >
   point_t<S, 3> normal( quad_t<S, 3> const &t)
   {
      point_decomposition_t< S, 3 >  const d = decompose( (t[1] - t[0]) ^ (t[2] - t[0]) );

      // ????
      if ( d.length == 0. )
         return point_t< S, 3 >( 0, 0, 1 );

      return d.direction;
   }

template <class S, size_t D>
   S square( quad_t<S, D> const &t)
   {
      return norm( (t[1] - t[0]) ^ (t[2] - t[0]) ) / 2;
   }

template <class S, size_t D>
   bool singular ( quad_t<S, D> const &t)
   {
      return eq_zero( (t[1] - t[0]) ^ (t[2] - t[0]) );
   }

template <class S>
   point_t<S, 2> describing_circle_center( quad_t<S, 2> const &t )
   {
      point_t<S,2> a = t[1] - t[0];
      point_t<S,2> b = t[2] - t[0];

      point_t<S,2> b_n ( -b.y, b.x );

      S g = a * b_n ;

      Assert ( !eq_zero( g ) ) ;

      return t[0] + b / 2 + b_n * (a * (a - b)) / (2 * g);
   }

template <class S>
   point_t<S, 2> inscribing_circle_center( quad_t<S, 2> const &t )
   {
      char a[-1];
   }

template <class S>
   point_t<S, 3> describing_circle_center( quad_t<S, 3> const &t )
   {
      char a[-1];
   }

template <class S>
   point_t<S, 3> inscribing_circle_center( quad_t<S, 3> const &t )
   {
      char a[-1];
   }
   */

} // End of 'cg' namespace

#undef MAX_POINT
#undef MAX_QUAD
