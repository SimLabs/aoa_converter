/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#if defined(__GNUG__)
    #pragma GCC diagnostic push

    #pragma GCC diagnostic ignored "-fpermissive"
    #pragma GCC diagnostic ignored "-Wparentheses"
#endif

#include "triangle_fwd.h"

#include <boost/static_assert.hpp>

#include "common/util.h"

#define MAX_POINT(scalar_a, scalar_b, dim)   point_t< MAX_TYPE(scalar_a, scalar_b), dim >
#define MAX_TRIANGLE(scalar_a, scalar_b, dim) triangle_t< MAX_TYPE(scalar_a, scalar_b), dim >

namespace geom
{
   #pragma pack (push, 1)

   template < class S, size_t D>
      struct triangle_t
      {
         typedef S scalar_type;
         enum  { dimension = D };

         typedef point_t<S, D>      point_type ;
         typedef point_t<S, D>      point_t ;

         typedef point_type *       iterator;
         typedef point_type const * const_iterator;

         triangle_t(point_t const &p0, point_t const &p1, point_t const &p2) ;
         triangle_t() ;

         template <class Scalar>
            triangle_t( triangle_t<Scalar, D> const& t );

         explicit triangle_t ( triangle_t<S, D+1> const& t );

         bool contains ( point_t const &p ) const ;

         point_t const & operator [] (size_t index) const ;
         point_t       & operator [] (size_t index) ;

         iterator       begin()        { return v_; }
         iterator       end()          { return v_ + 3; }

         const_iterator begin()  const { return v_; }
         const_iterator end()    const { return v_ + 3; }

      private:
         point_t   v_[3];
      };

   #pragma pack (pop)

   //operations
   template <class S>
      point_t< S, 2> normal( triangle_t<S, 2> const &t);

   template <class S, size_t D>
      S square( triangle_t<S, D> const &t) ;

   template <class S, size_t D>
      bool singular ( triangle_t<S, D> const &t) ;

   template <class S, size_t D>
      point_t<S, D> describing_circle_center( triangle_t<S, D> const &t );

   template <class Stream, class S, size_t D>
      Stream & operator <<( Stream & os, triangle_t<S, D> const & t );
}

/////////////////////////////////////////////////////////////////////////////////////////
// implementation
namespace geom
{
   template <class S, size_t D>
      __forceinline triangle_t<S,D>::triangle_t(point_t const &p0, point_t const &p1, point_t const& p2)
      {
         v_[0] = p0 ;
         v_[1] = p1 ;
         v_[2] = p2 ;
      }

   template <class S, size_t D>
      __forceinline triangle_t<S,D>::triangle_t()
      {}

   template <class S, size_t D> template <class Scalar>
      __forceinline triangle_t<S,D>::triangle_t( triangle_t<Scalar, D> const& v )
      {
         v_[0] = v[0];
         v_[1] = v[1];
         v_[2] = v[2];
      }

   template <class S, size_t D>
      triangle_t<S,D>::triangle_t ( triangle_t<S, D+1> const& v )
      {
         v_[0] = v[0];
         v_[1] = v[1];
         v_[2] = v[2];
      }

   template< class T >
      __forceinline void min_max_3( T const & v1, T const & v2, T const & v3,
                                    T & min, T & max )
   {
      if (v1 > v2)
      {
         if (v1 > v3)
         {
            max = v1;
            min = std::min(v2, v3);
         }
         else
            min = v2, max = v3;
      }
      else
      {
         if (v2 > v3)
         {
            max = v2;
            min = std::min(v1, v3);
         }
         else
            min = v1, max = v3;
      }
   }

   template <class S, size_t D>
      bool triangle_t<S,D>::contains ( point_t const &p ) const
      {
         BOOST_STATIC_ASSERT(D == 2);

         int signum[] =
         {
            geom::sign(((v_[1] - v_[0]) ^ (p - v_[0]))),
            geom::sign(((v_[2] - v_[1]) ^ (p - v_[1]))),
            geom::sign(((v_[0] - v_[2]) ^ (p - v_[2])))
         };

         return (signum[0] < 1 && signum[1] < 1 && signum[2] < 1) ||
                (signum[0] > -1 && signum[1] > -1 && signum[2] > -1);
      }

   template <class S, size_t D>
      __forceinline point_t<S,D> const &triangle_t<S,D>::operator[](size_t index) const
      {
         Assert ( index < 3 ) ;
         return v_[index];
      }

   template <class S, size_t D>
      __forceinline point_t<S,D> &triangle_t<S,D>::operator[](size_t index)
      {
         Assert ( index < 3 ) ;
         return v_[index];
      }

   //operations
   template <class S >
      point_t<S, 3> normal( triangle_t<S, 3> const &t)
      {
         point_decomposition_t< S, 3 >  const d = decompose( (t[1] - t[0]) ^ (t[2] - t[0]) );

         // ????
         if ( d.length == 0. )
            return point_t< S, 3 >( 0, 0, 1 );

         return d.direction;
      }

   template <class S, size_t D>
      S square( triangle_t<S, D> const &t)
      {
         return norm((t[1]-t[0])^(t[2]-t[0])) / 2;
      }

   template <class S, size_t D>
      bool singular ( triangle_t<S, D> const &t)
      {
         return eq_zero( (t[1] - t[0]) ^ (t[2] - t[0]) );
      }

   template <class S>
      point_t<S, 2> describing_circle_center( triangle_t<S, 2> const &t )
      {
         point_t<S,2> a = t[1] - t[0];
         point_t<S,2> b = t[2] - t[0];

         point_t<S,2> b_n ( -b.y, b.x );

         S g = a * b_n ;

         Assert ( !eq_zero( g ) ) ;

         return t[0] + b / 2 + b_n * (a * (a - b)) / (2 * g);
      }

   template <class Stream, class S, size_t D>
      Stream & operator <<( Stream & os, triangle_t<S, D> const & t )
      {
         os << '[' << t[0] << ", " << t[1] << ", " << t[2] << ']';
         return os;
      }

   template <class S, size_t D>
      point_t< S, D > center ( triangle_t<S, D> const &t)
      {
         return ( t[0] + t[1] + t[2] ) / S( 3 );
      }
}

#undef MAX_POINT
#undef MAX_TRIANGLE

#if defined(__GNUG__)
    #pragma GCC diagnostic pop
#endif

