/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "point_fwd.h"
#include <common/no_deduce.h>
//#include <boost/static_assert.hpp>
#include <geometry/common.h>
#include "logger/assert.hpp"

namespace geom
{
   template < class Scalar, size_t Dim >     struct point_t;
   template < class Scalar, size_t Dim >     struct point_decomposition_t;

#define MAX_POINT(scalar_a, scalar_b, dim)   point_t< MAX_TYPE(scalar_a, scalar_b), dim >

   // --------------------------------------------------------------------------------------------------------- addition
   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator + ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator - ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   // --------------------------------------------------------------------------------------------------------- component-wise multiplication
   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator & ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator / ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator % ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   // --------------------------------------------------------------------------------------------------------- constant multiplication
   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator * ( point_t< ScalarT, Dim > const & a, ScalarU alpha );

   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator * ( ScalarT alpha, point_t< ScalarU, Dim > const & a );

   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator / ( point_t< ScalarT, Dim > const & a, ScalarU alpha );

   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_POINT(ScalarT, ScalarU, Dim) operator % ( point_t< ScalarT, Dim > const & a, ScalarU alpha );

   // --------------------------------------------------------------------------------------------------------- scalar product
   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_TYPE(ScalarT, ScalarU)     operator * ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   // --------------------------------------------------------------------------------------------------------- vector product
   template < class ScalarT, class ScalarU >
      MAX_TYPE(ScalarT, ScalarU)     operator ^ ( point_t< ScalarT, 2ul > const & a, point_t< ScalarU, 2ul > const & b );
   template < class ScalarT, class ScalarU >
      MAX_POINT(ScalarT, ScalarU, 3ul) operator ^ ( point_t< ScalarT, 3ul > const & a, point_t< ScalarU, 3ul > const & b );

   // --------------------------------------------------------------------------------------------------------- unary operations
   template < class Scalar, size_t Dim >     point_t< Scalar, Dim > operator - ( point_t< Scalar, Dim > a );

   // --------------------------------------------------------------------------------------------------------- norm and distance operations
   // returns squared norm of point
   template < class Scalar, size_t Dim >     Scalar norm_sqr( point_t< Scalar, Dim > const & a );

   // returns norm of point
   template < class Scalar, size_t Dim >     Scalar norm( point_t< Scalar, Dim > const & a );

   // returns distance between two points
   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_TYPE(ScalarT, ScalarU) distance_sqr( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   // returns distance between two points
   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_TYPE(ScalarT, ScalarU) distance( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b );

   // returns angle in radians between two vectors ( 0..2pi in 2D case, 0..pi otherwise )
   template < class ScalarT, class ScalarU, size_t Dim >
      MAX_TYPE(ScalarT, ScalarU) angle ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b ) ;

   template < class Scalar >
      point_t< Scalar, 2ul > normal ( point_t< Scalar, 2ul > const & a );

   template < class Scalar, size_t Dim >
      bool is_finite( point_t< Scalar, Dim > const & a );

   // --------------------------------------------------------------------------------------------------------- normalization operations
   // returns norm of point
   template < class Scalar, size_t Dim >     Scalar normalize( point_t< Scalar, Dim > & point );

   // returns normalized point
   template < class Scalar, size_t Dim >     point_t< Scalar, Dim > normalized( point_t< Scalar, Dim > point );

   // shifts
   template < class Scalar, size_t Dim >
      point_t< Scalar, Dim > operator >> ( point_t< Scalar, Dim > const & point, size_t bits );

   template < class Scalar, size_t Dim >
      point_t< Scalar, Dim > operator << ( point_t< Scalar, Dim > const & point, size_t bits );

   template < class Scalar, size_t Dim >
      point_t< Scalar, Dim > & operator >>= ( point_t< Scalar, Dim > & point, size_t bits );

   template < class Scalar, size_t Dim >
      point_t< Scalar, Dim > & operator <<= ( point_t< Scalar, Dim > & point, size_t bits );

   // returns direction and length: direction * length == point
   //template < class Scalar, size_t Dim >     point_decomposition_t< Scalar, Dim > decompose( point_t< Scalar, Dim > const & point, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) );

   // --------------------------------------------------------------------------------------------------------- comparison
   // fuzzy
   //template < class Scalar, size_t Dim >     bool eq( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) );
   //template < class Scalar, size_t Dim >     bool eq_zero( point_t< Scalar, Dim > const & a, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) );

   //// strong
   //template < class Scalar, size_t Dim >     bool operator == ( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b );
   //template < class Scalar, size_t Dim >     bool operator != ( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b );

   //// ----------------------------------------------------------------------------------------------------------
   //template < class Scalar, size_t Dim >     point_t< int,    Dim > round( point_t< Scalar, Dim > const & );
   //template < class Scalar, size_t Dim >     point_t< Scalar, Dim > floor( point_t< Scalar, Dim > const & );
   //template < class Scalar, size_t Dim >     point_t< Scalar, Dim > ceil ( point_t< Scalar, Dim > const & );

   //// ---------------------------------------------------------------------------------------------------------- comparison operators
   //template < class Scalar, size_t Dim >     bool operator < ( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b ) ;
   //template < class Scalar, size_t Dim >     bool operator > ( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b ) ;
}

namespace geom
{
#pragma pack( push, 1 )

   // ----------------------------------------------------------------------------------------------------- point_2_t
   template < class Scalar >
      struct point_t< Scalar, 2ul >
   {
      typedef  Scalar   scalar_type;
      enum  { dimension = 2ul };

      // -------------------------------------------------------------- ctor
      point_t( );

      point_t( Scalar x, Scalar y );

      template < class _Scalar >
      point_t( point_t< _Scalar, 2ul > const & point );

      // -------------------------------------------------------------- arithmetic
      // vector operations
      point_t & operator += ( point_t const & );
      point_t & operator -= ( point_t const & );

      point_t & operator *= ( Scalar );
      point_t & operator /= ( Scalar );
      point_t & operator %= ( Scalar );

      // component-wise multiplication
      point_t & operator &= ( point_t const & );

      // component-wise division
      point_t & operator /= ( point_t const & );

      // component-wise remainder
      point_t & operator %= ( point_t const & );

      // -------------------------------------------------------------- data
      Scalar   x;
      Scalar   y;

      // -------------------------------------------------------------- data accessors
      Scalar         & operator [] ( size_t i )       { Assert( i < 2ul ); return (&x)[i]; }
      Scalar const   & operator [] ( size_t i ) const { Assert( i < 2ul ); return (&x)[i]; }

      // -------------------------------------------------------------- access for derived
      point_t const& as_2d () const { return *this ; }
      point_t &      as_2d ()       { return *this ; }

      // scalar explicit fabric
      template < typename Scalar2 >
      static point_t from_scalar(Scalar2 val) { return point_t(Scalar(val), Scalar(val)); }
   };

   // ----------------------------------------------------------------------------------------------------- point_3_t
   template < class Scalar >
      struct point_t< Scalar, 3ul >
         : point_t< Scalar, 2ul >
   {
      typedef  Scalar   scalar_type;
      enum  { dimension = 3ul };

      // -------------------------------------------------------------- ctor
      point_t( );

      point_t( Scalar x, Scalar y, Scalar z );

      point_t( point_t< Scalar, 2ul > const & point, Scalar h );

      explicit point_t( point_t< Scalar, 2ul > const & point );

      template < class _Scalar >
      point_t( point_t< _Scalar, 3ul > const & point );

      // -------------------------------------------------------------- arithmetic
      // vector operations
      point_t & operator += ( point_t const & );
      point_t & operator -= ( point_t const & );

      point_t & operator *= ( Scalar );
      point_t & operator /= ( Scalar );
      point_t & operator %= ( Scalar );

      // component-wise multiplication
      point_t & operator &= ( point_t const & );

      // component-wise division
      point_t & operator /= ( point_t const & );

      // component-wise remainder
      point_t & operator %= ( point_t const & );

      // -------------------------------------------------------------- data
      using point_t< Scalar, 2ul>::x;
      using point_t< Scalar, 2ul>::y;
      Scalar   z;

      // -------------------------------------------------------------- data accessors
      Scalar         & operator [] ( size_t i )       { Assert( i < 3ul ); return (&(this->x))[i]; }
      Scalar const   & operator [] ( size_t i ) const { Assert( i < 3ul ); return (&(this->x))[i]; }

      // -------------------------------------------------------------- access for derived
      point_t const& as_3d () const { return *this ; }
      point_t &      as_3d ()       { return *this ; }

      // scalar explicit fabric
      template < typename Scalar2 >
      static point_t from_scalar(Scalar2 val) { return point_t(Scalar(val), Scalar(val), Scalar(val)); }
   };


   // ----------------------------------------------------------------------------------------------------- point_4_t
   template < class Scalar >
      struct point_t< Scalar, 4ul >
         : point_t< Scalar, 3ul >
   {
      typedef  Scalar   scalar_type;
      enum  { dimension = 4ul };

      // -------------------------------------------------------------- ctor
      point_t( );

      point_t( Scalar x, Scalar y, Scalar z, Scalar w );

      point_t( point_t< Scalar, 3ul > const & point, Scalar w );

      explicit point_t( point_t< Scalar, 3ul > const & point );

      template < class _Scalar >
      point_t( point_t< _Scalar, 4ul > const & point );

      // -------------------------------------------------------------- arithmetic
      // vector operations
      point_t & operator += ( point_t const & );
      point_t & operator -= ( point_t const & );

      point_t & operator *= ( Scalar );
      point_t & operator /= ( Scalar );
      point_t & operator %= ( Scalar );

      // component-wise multiplication
      point_t & operator &= ( point_t const & );

      // component-wise division
      point_t & operator /= ( point_t const & );

      // component-wise remainder
      point_t & operator %= ( point_t const & );

      // -------------------------------------------------------------- data
      using point_t< Scalar, 3ul >::x;
      using point_t< Scalar, 3ul >::y;
      using point_t< Scalar, 3ul >::z;
      Scalar   w;

      // -------------------------------------------------------------- data accessors
      Scalar         & operator [] ( size_t i )       { Assert( i < 4ul ); return (&(this->x))[i]; }
      Scalar const   & operator [] ( size_t i ) const { Assert( i < 4ul ); return (&(this->x))[i]; }

      // -------------------------------------------------------------- access for derived
      point_t const& as_4d () const { return *this ; }
      point_t &      as_4d ()       { return *this ; }

      // scalar explicit fabric
      template < typename Scalar2 >
      static point_t from_scalar(Scalar2 val) { return point_t(Scalar(val), Scalar(val), Scalar(val), Scalar(val)); }
   };

   template < class Scalar, size_t Dim >
      struct point_decomposition_t
   {
      // -------------------------------------------------------------- ctor
      point_decomposition_t( );
      point_decomposition_t( point_t< Scalar, Dim > const & direction, Scalar length );

      // -------------------------------------------------------------- data
      point_t< Scalar, Dim >     direction;
      Scalar                     length;
   };

#pragma pack( pop )
}

// implementation
namespace geom
{
   // ----------------------------------------------------------------------------------------------------- point_2_t
   // -------------------------------------------------------------- ctor
   template < class Scalar >
   __forceinline  point_t< Scalar, 2ul > :: point_t( )
      : x( 0 )
      , y( 0 )
   {}

   template < class Scalar >
   __forceinline  point_t< Scalar, 2ul > :: point_t( Scalar x, Scalar y )
      : x( x )
      , y( y )
   {}

   template < class Scalar >
   template < class _Scalar >
   __forceinline  point_t< Scalar, 2ul > :: point_t( point_t< _Scalar, 2ul > const & point )
      : x( ( Scalar ) point.x )
      , y( ( Scalar ) point.y )
   {}

   // ----------------------------------------------------------------------------------------------------- point_3_t
   // -------------------------------------------------------------- ctor
   template < class Scalar >
   __forceinline point_t< Scalar, 3ul > :: point_t( )
      : z( 0 )
   {}

   template < class Scalar >
   __forceinline point_t< Scalar, 3ul > :: point_t( Scalar x, Scalar y, Scalar z )
      : point_t< Scalar, 2ul >( x, y )
      , z( z )
   {}

   template < class Scalar >
   __forceinline point_t< Scalar, 3ul > :: point_t( point_t< Scalar, 2ul > const & point, Scalar h )
      : point_t< Scalar, 2ul >( point )
      , z( h )
   {}

   template < class Scalar >
   __forceinline point_t< Scalar, 3ul > :: point_t( point_t< Scalar, 2ul > const & point )
      : point_t< Scalar, 2ul >( point )
      , z( 0 )
   {}

   template < class Scalar >
   template < class _Scalar >
   __forceinline point_t< Scalar, 3ul > :: point_t( point_t< _Scalar, 3ul > const & point )
      : point_t< Scalar, 2ul >( point )
      , z( ( Scalar ) point.z )
   {}

   // ----------------------------------------------------------------------------------------------------- point_4_t
   // -------------------------------------------------------------- ctor
   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > :: point_t( )
      : w( 0 )
   {}

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > :: point_t( Scalar x, Scalar y, Scalar z, Scalar w )
      : point_t< Scalar, 3ul >( x, y, z )
      , w( w )
   {}

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > :: point_t( point_t< Scalar, 3ul > const & point, Scalar w )
      : point_t< Scalar, 3ul >( point )
      , w( w )
   {}

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > :: point_t( point_t< Scalar, 3ul > const & point )
      : point_t< Scalar, 3ul >( point )
      , w( 0 )
   {}

   template < class Scalar >
      template < class _Scalar >
      __forceinline point_t< Scalar, 4ul > :: point_t( point_t< _Scalar, 4ul > const & point )
      : point_t< Scalar, 3ul >( point )
      , w( ( Scalar ) point.w )
   {}


   // ------------------------------------------------------------------------------------------------- point_decomposition_t
   template < class Scalar, size_t Dim >
   __forceinline point_decomposition_t< Scalar, Dim > :: point_decomposition_t( )
      : length( 0 )
   {}

   template < class Scalar, size_t Dim >
   __forceinline point_decomposition_t< Scalar, Dim > :: point_decomposition_t( point_t< Scalar, Dim > const & direction, Scalar length )
      : direction ( direction )
      , length    ( length    )
   {}

   // --------------------------------------------------------------------------------------------------------- addition
   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT( ScalarT, ScalarU, Dim ) operator + ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b )
   {
      return MAX_POINT( ScalarT, ScalarU, Dim )( a ) += b;
   }

   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT( ScalarT, ScalarU, Dim ) operator - ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b )
   {
      return MAX_POINT( ScalarT, ScalarU, Dim )( a ) -= b;
   }

   // --------------------------------------------------------------------------------------------------------- component-wise multiplication
   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT( ScalarT, ScalarU, Dim ) operator & ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b )
   {
      return MAX_POINT( ScalarT, ScalarU, Dim )( a ) &= b;
   }

   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT(ScalarT, ScalarU, Dim) operator % ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b )
   {
      return MAX_POINT(ScalarT, ScalarU, Dim) (a) %= b ;
   }

   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT( ScalarT, ScalarU, Dim ) operator / ( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b )
   {
      return MAX_POINT( ScalarT, ScalarU, Dim )( a ) /= b;
   }

   // --------------------------------------------------------------------------------------------------------- constant multiplication
   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT(ScalarT, ScalarU, Dim) operator * ( point_t< ScalarT, Dim > const & a, ScalarU alpha )
   {
      return MAX_POINT(ScalarT, ScalarU, Dim)( a ) *= static_cast< MAX_TYPE( ScalarT, ScalarU ) >( alpha );
   }

   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT(ScalarT, ScalarU, Dim) operator * ( ScalarT alpha, point_t< ScalarU, Dim > const & a )
   {
      return MAX_POINT(ScalarT, ScalarU, Dim)( a ) *= static_cast< MAX_TYPE( ScalarT, ScalarU ) >( alpha );
   }

   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT(ScalarT, ScalarU, Dim) operator / ( point_t< ScalarT, Dim > const & a, ScalarU alpha )
   {
      return MAX_POINT(ScalarT, ScalarU, Dim)( a ) /= static_cast< MAX_TYPE( ScalarT, ScalarU ) >( alpha );
   }

   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_POINT(ScalarT, ScalarU, Dim) operator % ( point_t< ScalarT, Dim > const & a, ScalarU alpha )
   {
      return MAX_POINT(ScalarT, ScalarU, Dim)( a ) %= static_cast< MAX_TYPE( ScalarT, ScalarU ) >( alpha );
   }

   // --------------------------------------------------------------------------------------------------------- scalar product
   template < class ScalarT, class ScalarU >
      __forceinline MAX_TYPE(ScalarT, ScalarU)     operator * ( point_t< ScalarT, 2ul > const & a, point_t< ScalarU, 2ul > const & b )
   {
      MAX_TYPE(ScalarT, ScalarU) res = 0;
      res += a.x * b.x;
      res += a.y * b.y;

      return res;
   }

   template < class ScalarT, class ScalarU >
      __forceinline MAX_TYPE(ScalarT, ScalarU)     operator * ( point_t< ScalarT, 3ul > const & a, point_t< ScalarU, 3ul > const & b )
   {
      MAX_TYPE(ScalarT, ScalarU) res = 0;
      res += a.x * b.x;
      res += a.y * b.y;
      res += a.z * b.z;

      return res;
   }

   template < class ScalarT, class ScalarU >
      __forceinline MAX_TYPE(ScalarT, ScalarU)     operator * ( point_t< ScalarT, 4ul > const & a, point_t< ScalarU, 4ul > const & b )
   {
      MAX_TYPE(ScalarT, ScalarU) res = 0;
      res += a.x * b.x;
      res += a.y * b.y;
      res += a.z * b.z;
      res += a.w * b.w;

      return res;
   }

   // --------------------------------------------------------------------------------------------------------- vector product
   template < class ScalarT, class ScalarU >
      __forceinline MAX_TYPE(ScalarT, ScalarU)     operator ^ ( point_t< ScalarT, 2ul > const & a, point_t< ScalarU, 2ul > const & b )
   {
      return a.x * b.y - a.y * b.x;
   }

   template < class ScalarT, class ScalarU >
      __forceinline MAX_POINT(ScalarT, ScalarU, 3ul) operator ^ ( point_t< ScalarT, 3ul > const & a, point_t< ScalarU, 3ul > const & b )
   {
      return MAX_POINT(ScalarT, ScalarU, 3ul) ( a.y * b.z - a.z * b.y,
                                              a.z * b.x - a.x * b.z,
                                              a.x * b.y - a.y * b.x );
   }

   // --------------------------------------------------------------------------------------------------------- unary operations
   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > operator - ( point_t< Scalar, 2ul > a )
   {
      a.x = -a.x;
      a.y = -a.y;

      return a;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > operator - ( point_t< Scalar, 3ul > a )
   {
      a.x = -a.x;
      a.y = -a.y;
      a.z = -a.z;

      return a;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > operator - ( point_t< Scalar, 4ul > a )
   {
      a.x = -a.x;
      a.y = -a.y;
      a.z = -a.z;
      a.w = -a.w;

      return a;
   }

   // --------------------------------------------------------------------------------------------------------- norm and distance operations
   // returns squared norm of point
   template < class Scalar, size_t Dim >
      __forceinline Scalar norm_sqr( point_t< Scalar, Dim > const & a )
   {
      return a * a;
   }

   // returns norm of point
   template < class Scalar, size_t Dim >
      __forceinline Scalar norm( point_t< Scalar, Dim > const & a )
   {
      BOOST_STATIC_ASSERT( is_floating_point_f<Scalar>::value ); // norm_for_integral_points_undefined
      return geom::sqrt( a * a );
   }

   // returns distance between two points
   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_TYPE(ScalarT, ScalarU) distance_sqr( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b )
   {
      return norm_sqr( b - a );
   }

   // returns distance between two points
   template < class ScalarT, class ScalarU, size_t Dim >
      __forceinline MAX_TYPE(ScalarT, ScalarU) distance( point_t< ScalarT, Dim > const & a, point_t< ScalarU, Dim > const & b )
   {
      return norm( b - a );
   }

   template < class ScalarT, class ScalarU>
      __forceinline MAX_TYPE(ScalarT, ScalarU) angle ( point_t< ScalarT, 2ul > const & a, point_t< ScalarU, 2ul > const & b )
   {
      typedef MAX_TYPE(ScalarT, ScalarU) S ;

      S ab = norm_sqr(a) * norm_sqr(b);

      S ret = !eq_zero(ab) ? acos( bound<S>( (a * b) / geom::sqrt(ab), -1, 1 ) ) : 0;

      return (a ^ b) < 0 ? 2ul * (S)pi - ret : ret;
   }

   template < class ScalarT, class ScalarU, size_t D>
      __forceinline MAX_TYPE(ScalarT, ScalarU) angle ( point_t< ScalarT, D > const & a, point_t< ScalarU, D > const & b )
   {
      typedef MAX_TYPE(ScalarT, ScalarU) S ;

      S ab = norm_sqr(a) * norm_sqr(b);

      return !eq_zero(ab) ? acos( bound<S>( (a * b) / geom::sqrt(ab), -1, 1 ) ) : 0;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > normal ( point_t< Scalar, 2ul > const & a )
   {
      return point_t< Scalar, 2ul > ( - a.y, a.x ) ;
   }

   template < class Scalar, size_t Dim >
      __forceinline bool is_finite( point_t< Scalar, Dim > const & a )
   {
      for ( size_t i = 0; i < Dim; ++i )
         if ( !geom::is_finite( a[i] ) )
            return false;
      return true;
   }

   // --------------------------------------------------------------------------------------------------------- normalization operations
   // returns norm of point
   template < class Scalar, size_t Dim >
      __forceinline Scalar normalize( point_t< Scalar, Dim > & point )
   {
      Scalar const point_norm = norm( point );
      point *= (1 / point_norm);

      return point_norm;
   }

   // returns normalized point
   template < class Scalar, size_t Dim >
      __forceinline point_t< Scalar, Dim > normalized( point_t< Scalar, Dim > point )
   {
      normalize( point );
      return point;
   }

   // returns normalized point
   template < class Scalar, size_t Dim >
      __forceinline point_t< Scalar, Dim > normalized_safe( point_t< Scalar, Dim > const & point )
   {
      point_decomposition_t<Scalar, Dim> dec = decompose( point );

      if ( dec.length != 0 )
         return dec.direction;

      point_t< Scalar, Dim > res;
      res[Dim - 1] = 1.;

      return res;
   }

   // shifts

   template < class Scalar, size_t Dim >
      __forceinline point_t< Scalar, Dim > operator >> ( point_t< Scalar, Dim > const & point, size_t bits )
   {
      BOOST_STATIC_ASSERT(boost::is_integral<Scalar>::value);

      point_t< Scalar, Dim > result;

      for (size_t i = 0; i < Dim; i++)
         result[i] = point[i] >> bits;

      return result;
   }

   template < class Scalar, size_t Dim >
      __forceinline point_t< Scalar, Dim > operator << ( point_t< Scalar, Dim > const & point, size_t bits )
   {
      BOOST_STATIC_ASSERT(boost::is_integral<Scalar>::value);

      point_t< Scalar, Dim > result;

      for (size_t i = 0; i < Dim; i++)
         result[i] = point[i] << bits;

      return result;
   }

   template < class Scalar, size_t Dim >
      __forceinline point_t< Scalar, Dim > & operator >>= ( point_t< Scalar, Dim > & point, size_t bits )
   {
      BOOST_STATIC_ASSERT(boost::is_integral<Scalar>::value);

      for (size_t i = 0; i < Dim; i++)
         point[i] >>= bits;

      return point;
   }

   template < class Scalar, size_t Dim >
      __forceinline point_t< Scalar, Dim > & operator <<= ( point_t< Scalar, Dim > & point, size_t bits )
   {
      BOOST_STATIC_ASSERT(boost::is_integral<Scalar>::value);

      for (size_t i = 0; i < Dim; i++)
         point[i] <<= bits;

      return point;
   }

   // returns direction and length: direction * length == point
   // if eq_zero( length, eps ) returns point( 0, 0, ... , 0 ), length == 0
   template < class Scalar, size_t Dim >
      __forceinline point_decomposition_t< Scalar, Dim > decompose( point_t< Scalar, Dim > const & point, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      Scalar point_norm = norm( point );
      if ( eq_zero( point_norm, eps ) )
         return point_decomposition_t< Scalar, Dim >( );

      return point_decomposition_t< Scalar, Dim >( point / point_norm, point_norm );
   }

   // --------------------------------------------------------------------------------------------------------- comparison
   // fuzzy
   template < class Scalar >
      __forceinline bool eq( point_t< Scalar, 2ul > const & a, point_t< Scalar, 2ul > const & b, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq( a.x, b.x, eps ) )
         return false;

      if ( !eq( a.y, b.y, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq( point_t< Scalar, 3ul > const & a, point_t< Scalar, 3ul > const & b, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq( a.x, b.x, eps ) )
         return false;

      if ( !eq( a.y, b.y, eps ) )
         return false;

      if ( !eq( a.z, b.z, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq( point_t< Scalar, 4ul > const & a, point_t< Scalar, 4ul > const & b, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq( a.x, b.x, eps ) )
         return false;

      if ( !eq( a.y, b.y, eps ) )
         return false;

      if ( !eq( a.z, b.z, eps ) )
         return false;

      if ( !eq( a.w, b.w, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq_rel( point_t< Scalar, 2ul > const & a, point_t< Scalar, 2ul > const & b, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq_rel( a.x, b.x, eps ) )
         return false;

      if ( !eq_rel( a.y, b.y, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq_rel( point_t< Scalar, 3ul > const & a, point_t< Scalar, 3ul > const & b, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq_rel( a.x, b.x, eps ) )
         return false;

      if ( !eq_rel( a.y, b.y, eps ) )
         return false;

      if ( !eq_rel( a.z, b.z, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq_rel( point_t< Scalar, 4ul > const & a, point_t< Scalar, 4ul > const & b, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq_rel( a.x, b.x, eps ) )
         return false;

      if ( !eq_rel( a.y, b.y, eps ) )
         return false;

      if ( !eq_rel( a.z, b.z, eps ) )
         return false;

      if ( !eq_rel( a.w, b.w, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq_zero( point_t< Scalar, 2ul > const & a, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq_zero( a.x, eps ) )
         return false;

      if ( !eq_zero( a.y, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq_zero( point_t< Scalar, 3ul > const & a, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq_zero( a.x, eps ) )
         return false;

      if ( !eq_zero( a.y, eps ) )
         return false;

      if ( !eq_zero( a.z, eps ) )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool eq_zero( point_t< Scalar, 4ul > const & a, NO_DEDUCE(Scalar) eps = epsilon< Scalar >( ) )
   {
      if ( !eq_zero( a.x, eps ) )
         return false;

      if ( !eq_zero( a.y, eps ) )
         return false;

      if ( !eq_zero( a.z, eps ) )
         return false;

      if ( !eq_zero( a.w, eps ) )
         return false;

      return true;
   }

   // strong
   template < class Scalar >
      __forceinline bool operator == ( point_t< Scalar, 2ul > const & a, point_t< Scalar, 2ul > const & b )
   {
      if ( a.x != b.x )
         return false;

      if ( a.y != b.y )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool operator == ( point_t< Scalar, 3ul > const & a, point_t< Scalar, 3ul > const & b )
   {
      if ( a.x != b.x )
         return false;

      if ( a.y != b.y )
         return false;

      if ( a.z != b.z )
         return false;

      return true;
   }

   template < class Scalar >
      __forceinline bool operator == ( point_t< Scalar, 4ul > const & a, point_t< Scalar, 4ul > const & b )
   {
      if ( a.x != b.x )
         return false;

      if ( a.y != b.y )
         return false;

      if ( a.z != b.z )
         return false;

      if ( a.w != b.w )
         return false;

      return true;
   }

   template < class Scalar, size_t Dim >
      __forceinline bool operator != ( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b )
   {
      return !( a == b );
   }

   // -----------------------------------------------------------------------------------------------------
   template < class Scalar >
      __forceinline point_t< int, 2ul > round( point_t< Scalar, 2ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< int, 2ul > ( round( point.x ),
                                 round( point.y ) );
   }

   template < class Scalar >
      __forceinline point_t< int, 3ul > round( point_t< Scalar, 3ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< int, 3ul > ( round( point.x ),
                                 round( point.y ),
                                 round( point.z ) );
   }

   template < class Scalar >
      __forceinline point_t< int, 4ul > round( point_t< Scalar, 4ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< int, 4ul > ( round( point.x ),
                                 round( point.y ),
                                 round( point.z ),
                                 round( point.w ) );
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > floor( point_t< Scalar, 2ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< Scalar, 2ul > ( ::floor( point.x ),
                                    ::floor( point.y ) );
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > floor( point_t< Scalar, 3ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< Scalar, 3ul > ( ::floor( point.x ),
                                    ::floor( point.y ),
                                    ::floor( point.z ) );
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > floor( point_t< Scalar, 4ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< Scalar, 4ul > ( ::floor( point.x ),
                                    ::floor( point.y ),
                                    ::floor( point.z ),
                                    ::floor( point.w ) );
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > ceil ( point_t< Scalar, 2ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< Scalar, 2ul > ( ::ceil( point.x ),
                                    ::ceil( point.y ) );
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > ceil ( point_t< Scalar, 3ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< Scalar, 3ul > ( ::ceil( point.x ),
                                    ::ceil( point.y ),
                                    ::ceil( point.z ) );
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > ceil ( point_t< Scalar, 4ul > const & point )
   {
      BOOST_STATIC_ASSERT_MSG( is_floating_point_f<Scalar>::value, "norm_for_integral_points_undefined" );

      return point_t< Scalar, 4ul > ( ::ceil( point.x ),
                                    ::ceil( point.y ),
                                    ::ceil( point.z ),
                                    ::ceil( point.w ) );
   }

   // ---------------------------------------------------------------------------------------------------------- operator less
   template < class Scalar >
      __forceinline bool operator < ( point_t< Scalar, 2ul > const & a, point_t< Scalar, 2ul > const & b )
   {
      if ( a.x != b.x )
         return a.x < b.x ;
      if ( a.y != b.y )
         return a.y < b.y ;

      return false ;
   }

   template < class Scalar >
      __forceinline bool operator < ( point_t< Scalar, 3ul > const & a, point_t< Scalar, 3ul > const & b )
   {
       if ( a.x != b.x )
           return a.x < b.x ;
       if ( a.y != b.y )
           return a.y < b.y ;
       if ( a.z != b.z )
           return a.z < b.z ;

      return false ;
   }

   template < class Scalar >
      __forceinline bool operator < ( point_t< Scalar, 4ul > const & a, point_t< Scalar, 4ul > const & b )
   {
       if ( a.x != b.x )
           return a.x < b.x ;
       if ( a.y != b.y )
           return a.y < b.y ;
       if ( a.z != b.z )
           return a.z < b.z ;
       if ( a.w != b.w )
           return a.w < b.w ;

      return false ;
   }

   // ---------------------------------------------------------------------------------------------------------- operator greater
   template < class Scalar, size_t Dim >
      __forceinline bool operator > ( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b )
   {
      return !( a < b || a == b );
   }

   template < class Scalar, size_t Dim >
      __forceinline bool operator <= ( point_t< Scalar, Dim > const & a, point_t< Scalar, Dim > const & b )
   {
      return a < b || a == b;
   }


   // ----------------------------------------------------------------------------------------------------- point_2_t

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator += ( point_t< Scalar, 2ul > const & point )
   {
      x += point.x;
      y += point.y;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator -= ( point_t< Scalar, 2ul > const & point )
   {
      x -= point.x;
      y -= point.y;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator *= ( Scalar alpha )
   {
      x *= alpha;
      y *= alpha;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator /= ( Scalar alpha )
   {
      x /= alpha;
      y /= alpha;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator %= ( Scalar alpha )
   {
      x = geom::mod(x,alpha);
      y = geom::mod(y,alpha);

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator &= ( point_t< Scalar, 2ul > const & point )
   {
      x *= point.x;
      y *= point.y;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator /= ( point_t< Scalar, 2ul > const & point )
   {
      x /= point.x;
      y /= point.y;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 2ul > & point_t< Scalar, 2ul > :: operator %= ( point_t< Scalar, 2ul > const & point )
   {
      x = geom::mod(x, point.x);
      y = geom::mod(y, point.y);

      return *this;
   }

   // ----------------------------------------------------------------------------------------------------- point_3_t

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator += ( point_t< Scalar, 3ul > const & point )
   {
      x += point.x;
      y += point.y;
      z += point.z;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator -= ( point_t< Scalar, 3ul > const & point )
   {
      x -= point.x;
      y -= point.y;
      z -= point.z;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator *= ( Scalar alpha )
   {
      x *= alpha;
      y *= alpha;
      z *= alpha;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator /= ( Scalar alpha )
   {
      x /= alpha;
      y /= alpha;
      z /= alpha;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator %= ( Scalar alpha )
   {
      x = geom::mod(x,alpha);
      y = geom::mod(y,alpha);
      z = geom::mod(z,alpha);

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator &= ( point_t< Scalar, 3ul > const & point )
   {
      x *= point.x;
      y *= point.y;
      z *= point.z;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator /= ( point_t< Scalar, 3ul > const & point )
   {
      x /= point.x;
      y /= point.y;
      z /= point.z;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 3ul > & point_t< Scalar, 3ul > :: operator %= ( point_t< Scalar, 3ul > const & point )
   {
      x = geom::mod(x, point.x);
      y = geom::mod(y, point.y);
      z = geom::mod(z, point.z);

      return *this;
   }

   // ----------------------------------------------------------------------------------------------------- point_4_t

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator += ( point_t< Scalar, 4ul > const & point )
   {
      x += point.x;
      y += point.y;
      z += point.z;
      w += point.w;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator -= ( point_t< Scalar, 4ul > const & point )
   {
      x -= point.x;
      y -= point.y;
      z -= point.z;
      w -= point.w;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator *= ( Scalar alpha )
   {
      x *= alpha;
      y *= alpha;
      z *= alpha;
      w *= alpha;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator /= ( Scalar alpha )
   {
      x /= alpha;
      y /= alpha;
      z /= alpha;
      w /= alpha;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator %= ( Scalar alpha )
   {
      x = geom::mod(x,alpha);
      y = geom::mod(y,alpha);
      z = geom::mod(z,alpha);
      w = geom::mod(w,alpha);

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator &= ( point_t< Scalar, 4ul > const & point )
   {
      x *= point.x;
      y *= point.y;
      z *= point.z;
      w *= point.w;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator /= ( point_t< Scalar, 4ul > const & point )
   {
      x /= point.x;
      y /= point.y;
      z /= point.z;
      w /= point.w;

      return *this;
   }

   template < class Scalar >
      __forceinline point_t< Scalar, 4ul > & point_t< Scalar, 4ul > :: operator %= ( point_t< Scalar, 4ul > const & point )
   {
      x = geom::mod(x, point.x);
      y = geom::mod(y, point.y);
      z = geom::mod(z, point.z);
      w = geom::mod(w, point.w);

      return *this;
   }

#undef MAX_POINT

}

