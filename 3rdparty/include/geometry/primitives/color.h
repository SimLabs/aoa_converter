/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "color_fwd.h"

#include "color_hsv.h"
#include "geometry/common.h"
#include "point_fwd.h"

// implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace geom
{
   template<> struct color_scalar_unit<unsigned char>
   {
      static __forceinline unsigned char get()     { return 255 ; }
      static __forceinline unsigned char epsilon() { return 0   ; }
   };
   template<> struct color_scalar_unit<float>
   {
      static __forceinline float         get()     { return 1.0f   ; }
      static __forceinline float         epsilon() { return 0.0035f; }
   };

   template<typename T> T color_epsilon() { return color_scalar_unit<T>::epsilon(); }

   template<typename S1, typename S2>
      __forceinline S1 color_cast ( S2 value )
      {
         return static_cast<S1>( ((double)value/color_scalar_unit<S2>::get())*color_scalar_unit<S1>::get() ) ;
      }

   //
   template<typename S>
      __forceinline color_t<S>::color_t ( S v )
         : r(v), g(v), b(v)
      {
      }

   template<typename S>
      __forceinline color_t<S>::color_t ( S r, S g, S b )
         : r(r), g(g), b(b)
      {
      }

   template<typename S> template<typename _S>
      __forceinline color_t<S>::color_t( const color_t<_S> &c )
         : r (color_cast<S>(c.r))
         , g (color_cast<S>(c.g))
         , b (color_cast<S>(c.b))
      {
      }

   template<typename S> template<typename _S>
      __forceinline color_t<S>::color_t( const color_hsv_t<_S> &c )
      {
         colorf_hsv source (c);
         colorf dest;

         source.h *= 360 ;

         if ( source.s == 0 )
            // If s is 0, all colors are the same (this is some flavor of gray)
            dest.r = dest.g = dest.b = source.v;
         else
         {
            // The color wheel consists of 6 sectors. Figure out which sector you're in.
            float sectorPos = source.h / 60;
            int sectorNumber = geom::floor(sectorPos);

            // Get the fractional part of the sector. That is, how many degrees into the sector are you?
            float fractionalSector = sectorPos - sectorNumber;

            // Calculate values for the three axes of the color.
            float p = source.v * (1 - source.s);
            float q = source.v * (1 - (source.s * fractionalSector));
            float t = source.v * (1 - (source.s * (1 - fractionalSector)));

            // Assign the fractional colors to r, g, and b based on the sector the angle is in.
            switch (sectorNumber)
            {
            case 0:
                dest.r = source.v;
                dest.g = t;
                dest.b = p;
                break;
            case 1:
                dest.r = q;
                dest.g = source.v;
                dest.b = p;
                break;
            case 2:
                dest.r = p;
                dest.g = source.v;
                dest.b = t;
                break;
            case 3:
                dest.r = p;
                dest.g = q;
                dest.b = source.v;
                break;
            case 4:
                dest.r = t;
                dest.g = p;
                dest.b = source.v;
                break;
            case 5:
                dest.r = source.v;
                dest.g = p;
                dest.b = q;
                break;
            }
         }

         *this = color_t<S>(dest);
      }

   //
   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator += ( const color_t &c )
      {
         r += c.r ;
         g += c.g ;
         b += c.b ;
         return *this ;
      }

   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator -= ( const color_t &c )
      {
         r -= c.r ;
         g -= c.g ;
         b -= c.b ;
         return *this ;
      }

   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator *= ( const color_t &c )
      {
         r *= c.r ;
         g *= c.g ;
         b *= c.b ;
         return *this ;
      }

   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator /= ( const color_t &c )
      {
         r /= c.r ;
         g /= c.g ;
         b /= c.b ;
         return *this ;
      }


   //
   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator += ( S v )
      {
         r += v ;
         g += v ;
         b += v ;
         return *this ;
      }

   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator -= ( S v )
      {
         r -= v ;
         g -= v ;
         b -= v ;
         return *this ;
      }

   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator *= ( S v )
      {
         r *= v ;
         g *= v ;
         b *= v;
         return *this ;
      }

   template<typename S>
      __forceinline color_t<S> & color_t<S>::operator /= ( S v )
      {
         r /= v ;
         g /= v ;
         b /= v ;
         return *this ;
      }

   //
   template <typename S>
      __forceinline color_t<S> operator + ( const color_t<S> &c1, const color_t<S> &c2 )
      {
         return color_t<S>(c1) += c2 ;
      }

   template <typename S>
      __forceinline color_t<S> operator - ( const color_t<S> &c1, const color_t<S> &c2 )
      {
         return color_t<S>(c1) -= c2 ;
      }

   template <typename S>
      __forceinline color_t<S> operator * ( const color_t<S> &c1, const color_t<S> &c2 )
      {
         return color_t<S>(c1) *= c2 ;
      }

   template <typename S>
      __forceinline color_t<S> operator / ( const color_t<S> &c1, const color_t<S> &c2 )
      {
         return color_t<S>(c1) /= c2 ;
      }

   //
   template <typename S>
      __forceinline color_t<S> operator + ( const color_t<S> &c, S v )
      {
         return color_t<S>(c) += v ;
      }

   template <typename S>
      __forceinline color_t<S> operator - ( const color_t<S> &c, S v )
      {
         return color_t<S>(c) -= v ;
      }

   template <typename S>
      __forceinline color_t<S> operator * ( const color_t<S> &c, S v )
      {
         return color_t<S>(c) *= v ;
      }

   template <typename S>
      __forceinline color_t<S> operator / ( const color_t<S> &c, S v )
      {
         return color_t<S>(c) /= v ;
      }

   //
   template <typename S>
      __forceinline color_t<S> operator + ( S v, const color_t<S> &c )
      {
         return color_t<S>(c) += v ;
      }

   template <typename S>
      __forceinline color_t<S> operator * ( S v, const color_t<S> &c )
      {
         return color_t<S>(c) *= v ;
      }

   //
   template <typename S>
      __forceinline bool eq( color_t<S> const & a, color_t<S> const & b, S eps )
      {
         return (abs(a.r - b.r) <= eps && abs(a.g - b.g) <= eps && abs(a.b - b.b) <= eps);
      }

   template <typename S>
      __forceinline bool eq_zero( color_t<S> const & a, S eps )
      {
         return (abs(a.r) <= eps && abs(a.g) <= eps && abs(a.b) <= eps);
      }

   //
   template <typename S>
      __forceinline bool operator == ( const color_t<S> &c1, const color_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( color_t<S> ) ) == 0 ;
      }

   template <typename S>
      __forceinline bool operator != ( const color_t<S> &c1, const color_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( color_t<S> ) ) != 0 ;
      }

   template <typename S>
      __forceinline bool operator < ( const color_t<S> &c1, const color_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( color_t<S> ) ) < 0 ;
      }

   //
   template<typename S>
      __forceinline colora_t<S>::colora_t ()
         : a(unit::get())
      {
      }

   template<typename S>
      __forceinline colora_t<S>::colora_t ( S r, S g, S b, S a)
         : color_t<S>(r, g, b)
         , a(a)
      {
      }

   template<typename S>
      __forceinline colora_t<S>::colora_t( const color_t<S>& c, S a )
         : color_t<S>(c)
         , a(a)
      {
      }

   template<typename S> template<typename _S>
      __forceinline colora_t<S>::colora_t( const colora_t<_S> &c )
         : color_t<S>(c)
         , a (color_cast<S>(c.a))
      {
      }

   //
   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator += ( const color_t<S> &c )
      {
         r += c.r ;
         g += c.g ;
         b += c.b ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator -= ( const color_t<S> &c )
      {
         r -= c.r ;
         g -= c.g ;
         b -= c.b ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator *= ( const color_t<S> &c )
      {
         r *= c.r ;
         g *= c.g ;
         b *= c.b ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator /= ( const color_t<S> &c )
      {
         r /= c.r ;
         g /= c.g ;
         b /= c.b ;
         return *this ;
      }

      //
   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator += ( const colora_t<S> &c )
      {
         r += c.r ;
         g += c.g ;
         b += c.b ;
         a += c.a ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator -= ( const colora_t<S> &c )
      {
         r -= c.r ;
         g -= c.g ;
         b -= c.b ;
         a -= c.a ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator *= ( const colora_t<S> &c )
      {
         r *= c.r ;
         g *= c.g ;
         b *= c.b ;
         a *= c.a ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator /= ( const colora_t<S> &c )
      {
         r /= c.r ;
         g /= c.g ;
         b /= c.b ;
         a /= c.a ;
         return *this ;
      }

   //
   template <typename S>
      __forceinline colora_t<S> operator + ( const colora_t<S> &c1, const color_t<S> &c2 )
      {
         return colora_t<S>(c1) += c2 ;
      }

   template <typename S>
      __forceinline colora_t<S> operator - ( const colora_t<S> &c1, const color_t<S> &c2 )
      {
         return colora_t<S>(c1) -= c2 ;
      }

   template <typename S>
      __forceinline colora_t<S> operator * ( const colora_t<S> &c1, const color_t<S> &c2 )
      {
         return colora_t<S>(c1) *= c2 ;
      }

   template <typename S>
      __forceinline colora_t<S> operator / ( const colora_t<S> &c1, const color_t<S> &c2 )
      {
         return colora_t<S>(c1) /= c2 ;
      }

   template <typename S>
      __forceinline colora_t<S> operator + ( const colora_t<S> &c1, const colora_t<S> &c2 )
      {
         return colora_t<S>(c1) += c2 ;
      }

   template <typename S>
      __forceinline colora_t<S> operator - ( const colora_t<S> &c1, const colora_t<S> &c2 )
      {
         return colora_t<S>(c1) -= c2 ;
      }

   template <typename S>
      __forceinline colora_t<S> operator * ( const colora_t<S> &c1, const colora_t<S> &c2 )
      {
         return colora_t<S>(c1) *= c2 ;
      }

   template <typename S>
      __forceinline colora_t<S> operator / ( const colora_t<S> &c1, const colora_t<S> &c2 )
      {
         return colora_t<S>(c1) /= c2 ;
      }

      //
   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator += ( S v )
      {
         r += v ;
         g += v ;
         b += v ;
         a += v ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator -= ( S v )
      {
         r -= v ;
         g -= v ;
         b -= v ;
         a -= v ;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator *= ( S v )
      {
         r *= v ;
         g *= v ;
         b *= v;
         a *= v;
         return *this ;
      }

   template<typename S>
      __forceinline colora_t<S> & colora_t<S>::operator /= ( S v )
      {
         r /= v ;
         g /= v ;
         b /= v ;
         a /= v ;
         return *this ;
      }

      //
      template <typename S>
   __forceinline colora_t<S> operator + ( const colora_t<S> &c, S v )
      {
         return colora_t<S>(c) += v ;
      }

      template <typename S>
   __forceinline colora_t<S> operator - ( const colora_t<S> &c, S v )
      {
         return colora_t<S>(c) -= v ;
      }

      template <typename S>
   __forceinline colora_t<S> operator * ( const colora_t<S> &c, S v )
      {
         return colora_t<S>(c) *= v ;
      }

      template <typename S>
   __forceinline colora_t<S> operator / ( const colora_t<S> &c, S v )
      {
         return colora_t<S>(c) /= v ;
      }

      //
   template <typename S>
      __forceinline color_t<S> operator + ( S v, const colora_t<S> &c )
      {
         return colora_t<S>(c) += v ;
      }

   template <typename S>
      __forceinline colora_t<S> operator * ( S v, const colora_t<S> &c )
      {
         return colora_t<S>(c) *= v ;
      }

      //
   template <typename S>
      __forceinline bool eq( colora_t<S> const & a, colora_t<S> const & b, S eps )
      {
         return (abs(a.r - b.r) <= eps && abs(a.g - b.g) <= eps && abs(a.b - b.b) <= eps && abs(a.a - b.a) <= eps);
      }

   template <typename S>
      __forceinline bool eq_zero( colora_t<S> const & a, S eps )
      {
         return (abs(a.r) <= eps && abs(a.g) <= eps && abs(a.b) <= eps && abs(a.a) <= eps);
      }

   //
   template <typename S>
      __forceinline bool operator == ( const colora_t<S> &c1, const colora_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( colora_t<S> ) ) == 0 ;
      }

   template <typename S>
      __forceinline bool operator != ( const colora_t<S> &c1, const colora_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( colora_t<S> ) ) != 0 ;
      }

   template <typename S>
      __forceinline bool operator < ( const colora_t<S> &c1, const colora_t<S> &c2 )
      {
         return memcmp( &c1, &c2, sizeof( colora_t<S> ) ) < 0 ;
      }

   template< class Scalar >
      color_t<Scalar> as_color( point_t<Scalar, 3> const & pt )
   {
      return color_t<Scalar>(pt.x, pt.y, pt.z);
   }

   //
   inline colorab color_white  () { return colorab() ; }
   inline colorab color_black  () { return colorab( 0, 0, 0 ) ; }
   inline colorab color_red    () { return colorab( 255, 0, 0 ) ; }
   inline colorab color_green  () { return colorab( 0, 255, 0 ) ; }
   inline colorab color_blue   () { return colorab( 0, 0, 255 ) ; }
   inline colorab color_yellow () { return colorab( 255, 255, 0 ) ; }
   inline colorab color_magenta() { return colorab( 255, 0, 255 ) ; }
   inline colorab color_cyan   () { return colorab( 0, 255, 255 ) ; }

   inline colorab color_brown       () { return colorab( 190, 128, 0 ) ; }
   inline colorab color_darkbrown   () { return colorab( 128, 0, 40 ) ; }
   inline colorab color_orange      () { return colorab( 255, 128, 0 ) ; }
   inline colorab color_lightorange () { return colorab( 255, 200, 100 ) ; }
   inline colorab color_lightgray   () { return colorab( 210, 210, 210 ) ; }
   inline colorab color_gray        () { return colorab( 178, 178, 178 ) ; }
   inline colorab color_darkgray    () { return colorab( 128, 128, 128 ) ; }
   inline colorab color_darkdarkgray() { return colorab( 90, 90, 90 ) ; }
   inline colorab color_supergray   () { return colorab( 50, 50, 50 ) ; }
   inline colorab color_lightblue   () { return colorab( 128, 220, 255 ) ; }
   inline colorab color_lightyellow () { return colorab( 216, 204, 148 ) ; }
   inline colorab color_lightred    () { return colorab( 255, 66, 71 ) ; }
   inline colorab color_lightcyan   () { return colorab( 240, 255, 255 ) ; }

   inline colorab color_selected    () { return colorab( 255, 128, 0 ) ; }

   inline colorb color_random ()
   {
      return colorb( uint8_t(rand( 255.f )), uint8_t(rand( 255.f )), uint8_t(rand( 255.f )) );
   }

   // TODO: correct this (u ~ brightness, v ~ brightness, so they can't be taken randomly this way)
   //inline colorf color_random ( float brigthness )
   //{
   //   // Brightness in [0, 1] (zero brightness will produce dark but not always black color).

   //   // Converting from YUV color model with provided brightness.
   //   // See http://en.wikipedia.org/wiki/YUV for details.

   //   float const u = rand( 0.463f * 2 ) - 0.463f;
   //   float const v = rand( 0.615f * 2 ) - 0.615f;

   //   return colorf(
   //      brigthness + 1.13983f * v,
   //      brigthness - 0.39465f * u - 0.58060f * v,
   //      brigthness + 2.03211f * u );
   //}

   inline colorf color_random ( float brigthness )
   {
      return colorf (color_hsv_random(brigthness));
   }

   //
   template <typename S>
   __forceinline S luminance_crt( color_t<S> const & col )
   {
       return S(0.299f * col.r + 0.587f * col.g + 0.114f * col.b);
   }

   //
   template <typename S>
   __forceinline S luminance_lcd( color_t<S> const & col )
   {
       return S(0.2127f * col.r + 0.7152f * col.g + 0.0722f * col.b);
   }


}
