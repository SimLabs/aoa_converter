#pragma once

#include "point_fwd.h"

namespace geom
{
   template < class S, size_t D >
   struct arc_t
   {
      enum  { dimension = D };
      typedef S                 scalar_type;
      typedef S                 scalar_t;
      typedef point_t<S, D>     point_type;
      typedef point_t<S,D>      point_t;
      typedef sphere_t<S,D>     sphere_type;
      typedef degree180_range   range_type;

      arc_t(point_t const &c, scalar_t r, range_type const& range) ;
      arc_t() ;

      template <class Scalar>
         arc_t( arc_t<Scalar, D> const& s );

      bool contains( S angle ) const ;
      bool contains(point_type const& p) const ;
      
      scalar_type closest(point_type const& p) const;
      point_type operator()(scalar_type t) const;
      scalar_type operator()(point_type const& p) const;
      scalar_type length() const;
      scalar_type course(scalar_type t) const;

      sphere_type sphere;
      range_type  range;

   private:
       point_type get_dir(scalar_type t) const;
   };

   // --------------------------------------------------------------------
   template < class S, size_t D >
      arc_t< S, D > :: arc_t( point_t const &c, S r, range_type const& range )
      : sphere( c, r )
      , range( range )
   {}

   template < class S, size_t D >
      arc_t< S, D > :: arc_t()
   {}

   template < class S, size_t D > template < class Scalar >
      arc_t< S, D > :: arc_t( arc_t< Scalar, D > const& a )
      : sphere( a.sphere )
      , range( a.range )
   {}

   template < class S, size_t D >
      bool arc_t< S, D > :: contains( S angle ) const
   {
      return range.contains(angle) ;
   }

    template < class S, size_t D >
    bool arc_t< S, D >::contains(point_type const& p) const
    {
        point_type dir = geom::normalized_safe(p - sphere.center);
        return range.contains(polar_point_2(dir).course) && eq_zero(distance(dir * sphere.radius, p));
    }


    template < class S, size_t D >
    S arc_t< S, D >::closest(point_type const& p) const
    {
        typedef polar_point_t<S, D> polar_point_type;
        S a = polar_point_type(p - sphere.center).course;

        return range.closest(a);
    }

    template < class S, size_t D >
    point_t<S, D> arc_t< S, D >::operator()(S t) const
    {
        return sphere.center + get_dir(t);
    }

    template < class S, size_t D >
    typename arc_t< S, D >::scalar_type arc_t< S, D >::operator()(point_type const& p) const
    {
        Assert(contains(p));
        return closest(p);
    }

    template < class S, size_t D >
    S arc_t< S, D >::length() const
    {
        return sphere.radius * grad2rad(range.size());
    }

    template < class S, size_t D >
    typename arc_t< S, D >::scalar_type arc_t< S, D >::course(scalar_type t) const
    {
        return polar_point_t<S,D>(-normal(get_dir(t))).course;
    }


    template < class S, size_t D >
    typename arc_t< S, D >::point_type arc_t< S, D >::get_dir(scalar_type t) const
    {
        return (rotation_2(t * range.size()) * point_type(polar_point_2(sphere.radius, range.lo())));
    }


} //geom
