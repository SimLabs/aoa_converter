/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "point_fwd.h"

namespace geom
{
   template < class S, size_t D >
   struct sphere_t
   {
      enum  { dimension = D };
      typedef S             scalar_type;
      typedef S             scalar_t;
      typedef point_t<S, D> point_type;
      typedef point_t<S,D>  point_t;

      sphere_t(point_t const &c, scalar_t r) ;
      sphere_t() ;

      template <class Scalar>
         sphere_t( sphere_t<Scalar, D> const& s );

      explicit sphere_t ( sphere_t<S, D+1> const& s );

      bool contains( point_t const& p ) const ;
      point_type closest_point(point_type const& p) const;

      point_t  center;
      scalar_t radius;
   };

   template<class S, size_t D> sphere_t<S, D> sphere_by_rectangle(rectangle_t<S, D> const& rect);

   // --------------------------------------------------------------------
   template < class S, size_t D >
      sphere_t< S, D > :: sphere_t( point_t const &c, S r )
      : center( c )
      , radius( r )
   {}

   template < class S, size_t D >
      sphere_t< S, D > :: sphere_t()
      : radius( 0 )
   {}

   template < class S, size_t D > template < class Scalar >
      sphere_t< S, D > :: sphere_t( sphere_t< Scalar, D > const& s )
      : center( s.center )
      , radius( S (s.radius) )
   {}

   template < class S, size_t D >
      sphere_t< S, D >::sphere_t( sphere_t< S, D + 1 > const& s )
      : center( s.c )
      , radius( s.r )
   {}


   // -------------------------------------------------------------------- sphere_details
   namespace sphere_details
   {
      template < class S, size_t D >
         bool contains( sphere_t<S, D> const& s, point_t<S, D> const& p )
      {
         return distance(s.center, p) <= s.radius ;
      }
   }

   template < class S, size_t D >
      bool sphere_t< S, D > :: contains( point_t const& p ) const
   {
      return sphere_details::contains(*this, p) ;
   }

    template < class S, size_t D >
    typename sphere_t< S, D >::point_type sphere_t< S, D >::closest_point(point_type const& p) const
    {
        return geom::normalized_safe(p - center) * radius;
    }

    //

    template<class S, size_t D> sphere_t<S, D> sphere_by_rectangle(rectangle_t<S, D> const& rect)
    {
        return sphere_t<S, D>(rect.center(), geom::norm(rect.size() / 2));
    }


} // geom
