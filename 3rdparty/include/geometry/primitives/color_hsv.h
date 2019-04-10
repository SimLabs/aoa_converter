/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "color_fwd.h"
#include "geometry/common.h"

#pragma pack(push, 1)

namespace geom
{
   template<typename S>
   struct color_hsv_t
   {
      typedef color_scalar_unit<S> unit;

      S h, s, v; // hue, saturation, value

      color_hsv_t( S v = unit::get() ) ;
      color_hsv_t( S h, S s, S v ) ;

      template<typename _S> color_hsv_t( const color_hsv_t<_S> &c ) ;
      template<typename _S> color_hsv_t( const color_t<_S> &c ) ;

      color_hsv_t & operator += ( const color_hsv_t &c ) ;
      color_hsv_t & operator -= ( const color_hsv_t &c ) ;
      color_hsv_t & operator *= ( const color_hsv_t &c ) ;
      color_hsv_t & operator /= ( const color_hsv_t &c ) ;

      color_hsv_t & operator += ( S v ) ;
      color_hsv_t & operator -= ( S v ) ;
      color_hsv_t & operator *= ( S v ) ;
      color_hsv_t & operator /= ( S v ) ;
   };

   template <typename S> __forceinline color_hsv_t<S> operator + ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 ) ;
   template <typename S> __forceinline color_hsv_t<S> operator - ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 ) ;
   template <typename S> __forceinline color_hsv_t<S> operator * ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 ) ;
   template <typename S> __forceinline color_hsv_t<S> operator / ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 ) ;

   template <typename S> __forceinline color_hsv_t<S> operator + ( const color_hsv_t<S> &c, S v ) ;
   template <typename S> __forceinline color_hsv_t<S> operator - ( const color_hsv_t<S> &c, S v ) ;
   template <typename S> __forceinline color_hsv_t<S> operator * ( const color_hsv_t<S> &c, S v ) ;
   template <typename S> __forceinline color_hsv_t<S> operator / ( const color_hsv_t<S> &c, S v ) ;

   template <typename S> __forceinline color_hsv_t<S> operator + ( S v, const color_hsv_t<S> &c ) ;
   template <typename S> __forceinline color_hsv_t<S> operator * ( S v, const color_hsv_t<S> &c ) ;

   template <typename S> __forceinline bool eq     ( color_hsv_t<S> const & a, color_hsv_t<S> const & b, S eps = color_epsilon<S>() );
   template <typename S> __forceinline bool eq_zero( color_hsv_t<S> const & a, S eps = color_epsilon<S>() );

   template <typename S> __forceinline bool operator == ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 ) ;
   template <typename S> __forceinline bool operator != ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 ) ;

   template <typename S> __forceinline bool operator <  ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 ) ;

}

#pragma pack (pop)

// implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace geom
{
   //
   template<typename S>
      __forceinline color_hsv_t<S>::color_hsv_t ( S v )
         : h(0), s(0), v(v)
      {
      }

   template<typename S>
      __forceinline color_hsv_t<S>::color_hsv_t ( S h, S s, S v )
         : h(h), s(s), v(v)
      {
      }

   template<typename S> template<typename _S>
      __forceinline color_hsv_t<S>::color_hsv_t( const color_hsv_t<_S> &c )
         : h (color_cast<S>(c.h))
         , s (color_cast<S>(c.s))
         , v (color_cast<S>(c.v))
      {
      }

   template<typename S> template<typename _S>
      __forceinline color_hsv_t<S>::color_hsv_t( const color_t<_S> &c )
      {
         colorf source (c);
         colorf_hsv dest;

         float min = geom::min(source.r, source.g, source.b);
         float max = geom::max(source.r, source.g, source.b);

         if (max == 0)
         {
            dest.v = dest.s = dest.h = 0;
         }
         else
         {
            dest.v = max;
            float delta = max - min;

            dest.s = delta / max;

            if (delta == 0)
               dest.h = 0;
            else
            {
               if (source.r == max)
                  dest.h = (source.g - source.b) / delta; // between yellow & magenta
               else if (source.g == max)
                  dest.h = 2 + (source.b - source.r) / delta; // between cyan & yellow
               else
                  dest.h = 4 + (source.r - source.g) / delta; // between magenta & cyan
               if (dest.h < 0)
                  dest.h += 6;
               dest.h /= 6; // sectors
            }
         }

         *this = color_hsv_t<S>(dest);
      }

   //
   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator += ( const color_hsv_t &c )
      {
         h += c.h ;
         s += c.s ;
         v += c.v ;
         return *this ;
      }

   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator -= ( const color_hsv_t &c )
      {
         h -= c.h ;
         s -= c.s ;
         v -= c.v ;
         return *this ;
      }

   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator *= ( const color_hsv_t &c )
      {
         h *= c.h ;
         s *= c.s ;
         v *= c.v ;
         return *this ;
      }

   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator /= ( const color_hsv_t &c )
      {
         h /= c.h ;
         s /= c.s ;
         v /= c.v ;
         return *this ;
      }


   //
   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator += ( S x )
      {
         h += x ;
         s += x ;
         v += x ;
         return *this ;
      }

   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator -= ( S x )
      {
         h -= x ;
         s -= x ;
         v -= x ;
         return *this ;
      }

   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator *= ( S x )
      {
         h *= x ;
         s *= x ;
         v *= x ;
         return *this ;
      }

   template<typename S>
      __forceinline color_hsv_t<S> & color_hsv_t<S>::operator /= ( S x )
      {
         h /= x ;
         s /= x ;
         v /= x ;
         return *this ;
      }

   //
   template <typename S>
      __forceinline color_hsv_t<S> operator + ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 )
      {
         return color_hsv_t<S>(c1) += c2 ;
      }

   template <typename S>
      __forceinline color_hsv_t<S> operator - ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 )
      {
         return color_hsv_t<S>(c1) -= c2 ;
      }

   template <typename S>
      __forceinline color_hsv_t<S> operator * ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 )
      {
         return color_hsv_t<S>(c1) *= c2 ;
      }

   template <typename S>
      __forceinline color_hsv_t<S> operator / ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 )
      {
         return color_hsv_t<S>(c1) /= c2 ;
      }

   //
   template <typename S>
      __forceinline color_hsv_t<S> operator + ( const color_hsv_t<S> &c, S v )
      {
         return color_hsv_t<S>(c) += v ;
      }

   template <typename S>
      __forceinline color_hsv_t<S> operator - ( const color_hsv_t<S> &c, S v )
      {
         return color_hsv_t<S>(c) -= v ;
      }

   template <typename S>
      __forceinline color_hsv_t<S> operator * ( const color_hsv_t<S> &c, S v )
      {
         return color_hsv_t<S>(c) *= v ;
      }

   template <typename S>
      __forceinline color_hsv_t<S> operator / ( const color_hsv_t<S> &c, S v )
      {
         return color_hsv_t<S>(c) /= v ;
      }

   //
   template <typename S>
      __forceinline color_hsv_t<S> operator + ( S v, const color_hsv_t<S> &c )
      {
         return color_hsv_t<S>(c) += v ;
      }

   template <typename S>
      __forceinline color_hsv_t<S> operator * ( S v, const color_hsv_t<S> &c )
      {
         return color_hsv_t<S>(c) *= v ;
      }

   //
   template <typename S>
      __forceinline bool operator == ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( color_hsv_t<S> ) ) == 0 ;
      }

   template <typename S>
      __forceinline bool operator != ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( color_hsv_t<S> ) ) != 0 ;
      }

   template <typename S>
      __forceinline bool operator < ( const color_hsv_t<S> &c1, const color_hsv_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( color_hsv_t<S> ) ) < 0 ;
      }

   //
   template <typename S>
      __forceinline bool eq( color_hsv_t<S> const & a, color_hsv_t<S> const & b, S eps )
      {
         return (abs(a.h - b.h) <= eps && abs(a.s - b.s) <= eps && abs(a.v - b.v) <= eps);
      }

   template <typename S>
      __forceinline bool eq_zero( color_hsv_t<S> const & a, S eps )
      {
         return abs(a.v) <= eps;
      }

   //
   inline colorf_hsv color_hsv_random ( float brightness = 1.0f, float saturation = 1.0f )
   {
      return colorf_hsv (rand(1.f), saturation, brightness);
   }
}
