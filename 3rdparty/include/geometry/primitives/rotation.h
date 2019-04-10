/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <geometry/common.h>

#include "point.h"
#include "turn.h"
#include "matrix.h"
#include "quaternion_fwd.h"

#include "rotation_fwd.h"

namespace geom
{
   template<typename scalar, int N> struct rotation_t ;

   // operations
   //rotation operator * ( rotation const& r1, rotation const& r2 ) ;
   //rotation operator ! ( rotation const& r );
   //point    operator * ( rotation const& r, point const& p ) ;
   //point    operator * ( point const& p, rotation const& r ) ;
   //matrix   operator * ( matrix const& m, rotation const& r ) ;
   //matrix   operator * ( rotation const& r, matrix const& m ) ;
}

namespace geom
{
#pragma pack(push,1)
   //
   template<typename scalar>
      struct rotation_t<scalar, 3>
   {
      typedef point_t<scalar, 3>          point_t ;
      typedef cpr_t<scalar>               cpr_t ;
      typedef rot_axis_t<scalar>          rot_axis_t ;
      typedef quaternion_t<scalar>        quaternion_t ;
      typedef matrix_t<scalar, 3>         matrix_t ;

      rotation_t () ;
      rotation_t ( cpr_t const& orient ) ;
      rotation_t ( rot_axis_t const& axis ) ;
      rotation_t ( point_t const& i, const point_t& j, const point_t& k ) ;
      rotation_t ( matrix_t const& matr ) ;

      template<typename _scalar>
         rotation_t ( rotation_t<_scalar,3> const& other ) ;

      matrix_t const &  matrix()       const ;
      cpr_t             cpr()          const ;
      quaternion_t      quaternion()   const ;
      rot_axis_t        rot_axis()     const ;

      rotation_t& turn_heading ( scalar angle ) ;
      rotation_t& turn_pitch   ( scalar angle ) ;
      rotation_t& turn_roll    ( scalar angle ) ;
      rotation_t& turn_axis    ( rot_axis_t const& axis ) ;

   private :
      matrix_t m_ ;
   };

   //
   template<typename scalar>
      struct rotation_t<scalar, 2>
   {
      typedef matrix_t<scalar, 2>         matrix_t ;
      typedef point_t<scalar, 2>          point_t ;

      rotation_t () ;
      rotation_t ( point_t const& i, const point_t& j ) ;
      rotation_t ( scalar angle ) ;
      rotation_t ( matrix_t const& m ) ;

      template<typename _scalar>
         rotation_t ( rotation_t<_scalar,2> const& other ) ;

      matrix_t const & matrix() const ;

      rotation_t& turn ( scalar angle ) ;
      scalar      angle() const ;

   private :
      matrix_t m_ ;
   };
#pragma pack(pop)
}

////////////////////////////////////////////////////////////////////////////////////////////
// implementation
namespace geom
{
   //
   template<class S> __forceinline rotation_t<S,3>::rotation_t ()
      : m_ ( 1 )
   {
   }

   template<class S> __forceinline rotation_t<S,3>::rotation_t ( matrix_t const& m )
      : m_ ( m )
   {
   }

   template<class S> __forceinline rotation_t<S,3>::rotation_t ( rot_axis_t const& axis )
   {
      S ca  = cos ( - grad2rad(axis.angle) ) ;
      S sa  = sin ( - grad2rad(axis.angle) ) ;

      point_t const& a = axis.axis ;
      m_(0,0) = ca + a.x * a.x * (1 - ca);
      m_(0,1) = a.x * a.y * (1 - ca) + a.z * sa;
      m_(0,2) = a.x * a.z * (1 - ca) - a.y * sa;
      m_(1,0) = a.x * a.y * (1 - ca) - a.z * sa;
      m_(1,1) = ca + a.y * a.y * (1 - ca);
      m_(1,2) = a.y * a.z * (1 - ca) + a.x * sa;
      m_(2,0) = a.x * a.z * (1 - ca) + a.y * sa;
      m_(2,1) = a.y * a.z * (1 - ca) - a.x * sa;
      m_(2,2) = ca + a.z * a.z * (1 - ca);
   }

   template<class S> __forceinline rotation_t<S,3>::rotation_t ( const point_t& i, const point_t& j, const point_t& k )
   {
      m_.put_col(0, i);
      m_.put_col(1, j);
      m_.put_col(2, k);
   }

   template<class S> __forceinline rotation_t<S,3>::rotation_t ( cpr_t const & orient )
   {
      S course = grad2rad(orient.course);
      S pitch  = grad2rad(orient.pitch );
      S roll   = grad2rad(orient.roll  );

      S ccrs   = cos ( course );
      S scrs   = sin ( course );
      S cpitch = cos ( pitch  );
      S spitch = sin ( pitch  );
      S croll  = cos ( roll   );
      S sroll  = sin ( roll   );

      m_(0,0) = ccrs*croll + spitch*scrs*sroll;
      m_(0,1) = cpitch*scrs;
      m_(0,2) = ccrs*sroll - spitch*scrs*croll;
      m_(1,0) = -scrs*croll + spitch*ccrs*sroll;
      m_(1,1) = cpitch*ccrs;
      m_(1,2) = -scrs*sroll - spitch*ccrs*croll;
      m_(2,0) = -cpitch*sroll;
      m_(2,1) = spitch;
      m_(2,2) = cpitch*croll;
   }

   template<typename S> template<typename _S>
   __forceinline rotation_t<S,3>::rotation_t ( rotation_t<_S,3> const& other )
      : m_ ( other.matrix () )
   {
   }

   template<class S> __forceinline typename rotation_t<S,3>::cpr_t rotation_t<S,3>::cpr () const
   {
      S course = atan2 ( m_(0,1), m_(1,1) ) ;
      S pitch = asin ( geom::bound<S>(m_(2,1), -1, 1) ) ;

      S sh = sin ( course ) ;
      S ch = cos ( course ) ;
      S roll = - atan2 ( (m_(0,1)*m_(2,0) - m_(2,1)*m_(0,0)) * sh + (m_(1,1)*m_(2,0) - m_(2,1)*m_(1,0)) * ch, m_(0,0)*ch - m_(1,0) * sh ) ;

      return cpr_t(rad2grad(course), rad2grad(pitch), rad2grad(roll));
   }

   template<class S> __forceinline typename rotation_t<S,3>::quaternion_t rotation_t<S,3>::quaternion () const
   {
      S tr = m_(0, 0) + m_(1, 1) + m_(2, 2);

      if ( geom::le(tr, 0) )
      {
         int i = (m_(1, 1) > m_(0, 0)) ? 1 : 0;
         if ( m_(2, 2) > m_(i, i) )
            i = 2;

         int j = (i + 1) % 3;
         int k = (j + 1) % 3;

         S s = geom::sqrt((m_(i, i) - (m_(j, j) + m_(k, k))) + 1);
         S si = !eq_zero(s) ? (S(0.5) / s) : 0;

         S q[4];
         q[i] = s * S(0.5);
         q[3] = (m_(j, k) - m_(k, j)) * si;
         q[j] = (m_(i, j) + m_(j, i)) * si;
         q[k] = (m_(i, k) + m_(k, i)) * si;

         quaternion_t quat(q[3], -point_t(q[0], q[1], q[2]));
         quat.normalize();
         return quat;
      }

      S s = geom::sqrt(tr + S(1.0));
      S si = S(0.5) / s;

      S x = (m_(1, 2) - m_(2, 1)) * si;
      S y = (m_(2, 0) - m_(0, 2)) * si;
      S z = (m_(0, 1) - m_(1, 0)) * si;

      quaternion_t quat(s * S(0.5), -point_t(x, y, z));
      quat.normalize();
      return quat;
   }

   template<class S> __forceinline typename rotation_t<S,3>::rot_axis_t rotation_t<S,3>::rot_axis () const
   {
      S s = geom::sqrt(sqr(m_(2, 1) - m_(1, 2))+sqr(m_(0, 2) - m_(2, 0))+sqr(m_(1, 0) - m_(0, 1)));

      // TODO: handle eq_zero(s)

      return rot_axis_t(
         point_3((m_(2, 1) - m_(1, 2))/s, (m_(0, 2) - m_(2, 0))/s, (m_(1, 0) - m_(0, 1))/s),
         rad2grad(acos((m_(0, 0) + m_(1, 1) + m_(2, 2) - 1) / 2))
         );
   }

   template<class S> __forceinline typename rotation_t<S,3>::matrix_t const& rotation_t<S,3>::matrix () const
   {
      return m_ ;
   }

   template<class S> __forceinline rotation_t<S,3>& rotation_t<S,3>::turn_heading ( S angle )
   {
      S sa = sin ( grad2rad(angle) ) ;
      S ca = cos ( grad2rad(angle) ) ;
      S temp;

      temp = m_(0,0) * ca - m_(0,1) * sa; // New m_(0,0)
      m_(0,1)  = m_(0,0) * sa + m_(0,1) * ca;
      m_(0,0)  = temp;
      temp = m_(1,0) * ca - m_(1,1) * sa; // New m_(1,0)
      m_(1,1)  = m_(1,0) * sa + m_(1,1) * ca;
      m_(1,0)  = temp;
      temp = m_(2,0) * ca - m_(2,1) * sa; // New m_(2,0)
      m_(2,1)  = m_(2,0) * sa + m_(2,1) * ca;
      m_(2,0)  = temp;

      return *this ;
   }

   template<class S> __forceinline rotation_t<S,3>& rotation_t<S,3>::turn_pitch ( S angle )
   {
      S sa = sin ( grad2rad(angle) ) ;
      S ca = cos ( grad2rad(angle) ) ;
      S temp;

      temp = m_(0,1) * ca + m_(0,2) * sa; // New m_(0,1)
      m_(0,2)  =  m_(0,2) * ca - m_(0,1) * sa;
      m_(0,1)  = temp;
      temp = m_(1,1) * ca + m_(1,2) * sa; // New m_(1,1)
      m_(1,2)  =  m_(1,2) * ca - m_(1,1) * sa;
      m_(1,1)  = temp;
      temp = m_(2,1) * ca + m_(2,2) * sa; // New m_(2,1)
      m_(2,2)  =  m_(2,2) * ca - m_(2,1) * sa;
      m_(2,1)  = temp;

      return *this ;
   }

   template<class S> __forceinline rotation_t<S,3>& rotation_t<S,3>::turn_roll ( S angle )
   {
      S sa = sin ( grad2rad(angle) ) ;
      S ca = cos ( grad2rad(angle) ) ;
      S temp;

      temp = m_(0,0) * ca - m_(0,2) * sa; // New m_(0,0)
      m_(0,2)  =  m_(0,0) * sa + m_(0,2) * ca;
      m_(0,0)  = temp;
      temp = m_(1,0) * ca - m_(1,2) * sa; // New m_(1,0)
      m_(1,2)  =  m_(1,0) * sa + m_(1,2) * ca;
      m_(1,0)  = temp;
      temp = m_(2,0) * ca - m_(2,2) * sa; // New m_(2,0)
      m_(2,2)  =  m_(2,0) * sa + m_(2,2) * ca;
      m_(2,0)  = temp;

      return *this ;
   }

   template<class S> __forceinline rotation_t<S,3>& rotation_t<S,3>::turn_axis ( rot_axis_t const& axis )
   {
      m_ *= rotation_t<S,3>(axis).matrix() ;

      return *this ;
   }

   //
   template<class S> __forceinline rotation_t<S,2>::rotation_t ()
      : m_ ( 1 )
   {
   }

   template<class S> __forceinline rotation_t<S,2>::rotation_t ( matrix_t const& m )
      : m_ ( m )
   {
   }

   template<class S> __forceinline rotation_t<S,2>::rotation_t ( const point_t& i, const point_t& j )
   {
      m_.col[0] = i ;
      m_.col[1] = j ;
   }

   template<class S> __forceinline rotation_t<S,2>::rotation_t ( S angle )
   {
      angle = grad2rad(angle) ;

      m_(0,0) = cos(angle);
      m_(1,0) = -sin(angle);
      m_(0,1) = -m_(1,0);
      m_(1,1) = m_(0,0);
   }

   template<typename S> template<typename _S>
   __forceinline rotation_t<S,2>::rotation_t ( rotation_t<_S,2> const& other )
      : m_ ( other.matrix () )
   {
   }

   template<class S> __forceinline typename rotation_t<S,2>::matrix_t const& rotation_t<S,2>::matrix () const
   {
      return m_ ;
   }

   template<class S> __forceinline rotation_t<S,2>& rotation_t<S,2>::turn ( S angle )
   {
      m_ *= rotation_t<S,2>(angle).matrix() ;
      return *this ;
   }

   template<class S> __forceinline S rotation_t<S,2>::angle() const
   {
      return rad2grad( atan2( m_(0,1), m_(0,0) ) ) ;
   }

   // operations
   template<typename S, int N> __forceinline rotation_t<S,N> operator * ( rotation_t<S,N> const& r1, rotation_t<S,N> const& r2 )
   {
      return rotation_t<S,N> ( r1.matrix() * r2.matrix() );
   }

   template<typename S, int N> __forceinline rotation_t<S,N> operator ! ( rotation_t<S,N> const& r )
   {
      return rotation_t<S,N> ( !r.matrix() );
   }

   template<typename S, int N> __forceinline point_t<S,N> operator * ( rotation_t<S,N> const& r, typename rotation_t<S,N>::point_t const& p )
   {
      return r.matrix() * p ;
   }

   template<typename S, int N> __forceinline point_t<S,N> operator * ( typename rotation_t<S,N>::point_t const& p, rotation_t<S,N> const& r )
   {
      return p * r.matrix() ;
   }

   template<typename S, int N>  __forceinline typename rotation_t<S,N>::matrix_t operator * ( typename rotation_t<S,N>::matrix_t const& m, rotation_t<S,N> const& r )
   {
      return m * r.matrix();
   }

   template<typename S, int N> __forceinline typename rotation_t<S,N>::matrix_t operator * ( rotation_t<S,N> const& r, typename rotation_t<S,N>::matrix_t const& m )
   {
      return r.matrix() * m ;
   }
}
