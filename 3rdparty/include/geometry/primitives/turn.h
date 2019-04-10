/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/common.h"

#include "point.h"

#include "turn_fwd.h"

namespace geom
{

   template<typename scalar> struct cpr_t ;
   template<typename scalar> struct dcpr_t ;

   template<typename scalar> struct rot_axis_t ;

   // TODO ::   sleepy
   template<typename scalar> rot_axis_t<scalar> dcpr2rot_axis ( cpr_t<scalar> const& cpr, dcpr_t<scalar>     const& dcpr ) ;
   template<typename scalar> dcpr_t<scalar>     rot_axis2dcpr ( cpr_t<scalar> const& cpr, rot_axis_t<scalar> const& axis ) ;

   template<typename scalar> bool eq( cpr_t<scalar> const & a,      cpr_t<scalar> const & b,      NO_DEDUCE(scalar) eps = epsilon<scalar>() );
   template<typename scalar> bool eq( dcpr_t<scalar> const & a,     dcpr_t<scalar> const & b,     NO_DEDUCE(scalar) eps = epsilon<scalar>() );
   template<typename scalar> bool eq( rot_axis_t<scalar> const & a, rot_axis_t<scalar> const & b, NO_DEDUCE(scalar) eps = epsilon<scalar>() );
}

namespace geom
{
   #pragma pack(push,1)
   //
   template<typename scalar>
      struct cpr_t
   {
      scalar course, pitch, roll ;

      cpr_t( scalar c = 0, scalar p = 0, scalar r = 0) ;

      template<typename _scalar>
         cpr_t( cpr_t<_scalar> const& other ) ;
   } ;

   //
   template<typename scalar>
      struct dcpr_t
   {
      scalar dcourse, dpitch, droll;

      dcpr_t( scalar dc = 0, scalar dp = 0, scalar dr = 0 ) ;

      template<typename _scalar>
         dcpr_t( dcpr_t<_scalar> const& other ) ;
   } ;

   //
   template<typename scalar>
      struct rot_axis_t
   {
      point_t<scalar, 3>   axis ;
      scalar               angle ;

      rot_axis_t( ) ;
      rot_axis_t( point_t<scalar, 3> const &axis, scalar angle ) ;

      explicit rot_axis_t( point_t<scalar, 3> const &omega ) ; // NOTE: |omega| = angle (in degrees); omega / |omega| = axis
      point_t<scalar, 3> omega() const ;

      template<typename _scalar>
         rot_axis_t( rot_axis_t<_scalar> const& other ) ;
   } ;
   #pragma pack(pop)
}

////////////////////////////////////////////////////////////////////////////////////////////
// implementation
namespace geom
{
   // cpr_t
   template<typename S> cpr_t<S>::cpr_t (S c, S p, S r)
      : course(c), pitch(p), roll(r)
   {}

   template<typename S> template<typename _S> cpr_t<S>::cpr_t ( cpr_t<_S> const& other)
      : course ( (S)other.course )
      , pitch  ( (S)other.pitch  )
      , roll   ( (S)other.roll   )
   {
   }

   // dcpr_t
   template<typename S> dcpr_t<S>::dcpr_t (S c, S p, S r)
      : dcourse(c), dpitch(p), droll(r)
   {}

   template<typename S> template<typename _S> dcpr_t<S>::dcpr_t ( dcpr_t<_S> const& other)
      : dcourse ( (S)other.dcourse )
      , dpitch  ( (S)other.dpitch  )
      , droll   ( (S)other.droll   )
   {
   }

   // rot_axis_t
   template<typename S> rot_axis_t<S>::rot_axis_t ( )
      : axis(1, 0, 0), angle(0)
   {}

   template<typename S> rot_axis_t<S>::rot_axis_t ( point_t<S,3> const& axis, S angle )
      : axis(axis), angle(angle)
   {
      //Assert(geom::eq(geom::norm(axis), (S)1, (S)1e-4));
   }

   template<typename S> rot_axis_t<S>::rot_axis_t ( point_t<S,3> const &omega )
      : angle(norm(omega))
   {
      if ( geom::eq_zero(angle) )
         axis = point_t<S,3>(1, 0, 0);
      else
         axis = omega / angle;
   }

   template<typename S> point_t<S,3> rot_axis_t<S>::omega () const
   {
      return axis * angle ;
   }

   template<typename S> template<typename _S> rot_axis_t<S>::rot_axis_t ( rot_axis_t<_S> const& other )
      : axis  (    other.axis  )
      , angle ( (S)other.angle )
   {
   }

   // dcpr <-> rot_axis conversion
   template<typename S>
      rot_axis_t<S> dcpr2rot_axis ( cpr_t<S> const& cpr, dcpr_t<S> const& dcpr )
   {
      typedef point_t<S, 3>   point_t;

      S sinCourse = sin(grad2rad(cpr.course));
      S cosCourse = cos(grad2rad(cpr.course));
      S cosPitch  = cos(grad2rad(cpr.pitch ));
      S sinPitch  = sin(grad2rad(cpr.pitch ));

      point_t omega(                + cosCourse * dcpr.dpitch  + sinCourse            * dcpr.droll,
                                    - sinCourse * dcpr.dpitch  + cosPitch * cosCourse * dcpr.droll,
                     -dcpr.dcourse                             + sinPitch * cosCourse * dcpr.droll );

      return rot_axis_t<S>(omega);
   }

   template<typename S>
      dcpr_t<S> rot_axis2dcpr ( cpr_t<S> const& cpr, rot_axis_t<S> const& axis )
   {
      point_t<S,3> omega = axis.omega();

      S sinCourse = sin(grad2rad(cpr.course));
      S cosCourse = cos(grad2rad(cpr.course));
      S cosPitch  = cos(grad2rad(cpr.pitch ));
      S sinPitch  = sin(grad2rad(cpr.pitch ));

      S det = sinCourse * sinCourse + cosCourse * cosCourse * cosPitch;

      if ( !eq_zero(det) )
      {
         S wx = omega.x / det;
         S wy = omega.y / det;

         return dcpr_t<S>( sinCourse * sinPitch * cosCourse * wx + cosCourse * cosCourse * sinPitch * wy - omega.z,
                           cosPitch * cosCourse             * wx - sinCourse                        * wy          ,
                           sinCourse                        * wx + cosCourse                        * wy          );
      }

      if ( !eq_zero(sinCourse) )
         return dcpr_t<S>( sinPitch * omega.x - omega.z, 0, omega.x / sinCourse );

      return dcpr_t<S>( -omega.z, omega.x / cosCourse, 0 );
   }
}

namespace geom
{
   template<typename scalar>
      bool eq( cpr_t<scalar> const & a, cpr_t<scalar> const & b, NO_DEDUCE(scalar) eps )
   {
      return eq( a.course, b.course, eps )
         &&  eq( a.pitch,  b.pitch,  eps )
         &&  eq( a.roll,   b.roll,   eps );
   }

   template<typename scalar>
      bool eq( dcpr_t<scalar> const & a, dcpr_t<scalar> const & b, NO_DEDUCE(scalar) eps )
   {
      return eq( a.dcourse, b.dcourse, eps )
         &&  eq( a.dpitch,  b.dpitch,  eps )
         &&  eq( a.droll,   b.droll,   eps );
   }

   template<typename scalar>
      bool eq( rot_axis_t<scalar> const & a, rot_axis_t<scalar> const & b, NO_DEDUCE(scalar) eps )
   {
      return eq( a.axis,  b.axis,  eps )
         &&  eq( a.angle, b.angle, eps );
   }
}
