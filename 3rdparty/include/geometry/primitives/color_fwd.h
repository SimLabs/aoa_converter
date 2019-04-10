/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   template<typename scalar> struct color_scalar_unit;
   template<typename scalar1, typename scalar2> scalar1 color_cast ( scalar2 value ) ;

   //
   template<typename scalar> struct color_hsv_t ;

   //
   typedef color_hsv_t<unsigned char> colorb_hsv ;
   typedef color_hsv_t<float>         colorf_hsv ;

   template<typename T> T color_epsilon();

   /// color class
   #pragma pack(push, 1)
   template<typename S>
   struct color_t
   {
      typedef color_scalar_unit<S> unit;

      S r, g, b;

      color_t( S v = unit::get() ) ;
      color_t( S r, S g, S b ) ;

      template<typename _S> color_t( const color_t<_S> &c ) ;
      template<typename _S> color_t( const color_hsv_t<_S> &c ) ;

      color_t & operator += ( const color_t &c ) ;
      color_t & operator -= ( const color_t &c ) ;
      color_t & operator *= ( const color_t &c ) ;
      color_t & operator /= ( const color_t &c ) ;

      color_t & operator += ( S v ) ;
      color_t & operator -= ( S v ) ;
      color_t & operator *= ( S v ) ;
      color_t & operator /= ( S v ) ;
   };

   template <typename S> __forceinline color_t<S> operator + ( const color_t<S> &c1, const color_t<S> &c2 ) ;
   template <typename S> __forceinline color_t<S> operator - ( const color_t<S> &c1, const color_t<S> &c2 ) ;
   template <typename S> __forceinline color_t<S> operator * ( const color_t<S> &c1, const color_t<S> &c2 ) ;
   template <typename S> __forceinline color_t<S> operator / ( const color_t<S> &c1, const color_t<S> &c2 ) ;

   template <typename S> __forceinline color_t<S> operator + ( const color_t<S> &c, S v ) ;
   template <typename S> __forceinline color_t<S> operator - ( const color_t<S> &c, S v ) ;
   template <typename S> __forceinline color_t<S> operator * ( const color_t<S> &c, S v ) ;
   template <typename S> __forceinline color_t<S> operator / ( const color_t<S> &c, S v ) ;

   template <typename S> __forceinline color_t<S> operator + ( S v, const color_t<S> &c ) ;
   template <typename S> __forceinline color_t<S> operator * ( S v, const color_t<S> &c ) ;

   template <typename S> __forceinline bool eq     ( color_t<S> const & a, color_t<S> const & b, S eps = color_epsilon<S>() );
   template <typename S> __forceinline bool eq_zero( color_t<S> const & a, S eps = color_epsilon<S>() );

   template <typename S> __forceinline bool operator == ( const color_t<S> &c1, const color_t<S> &c2 ) ;
   template <typename S> __forceinline bool operator != ( const color_t<S> &c1, const color_t<S> &c2 ) ;

   template <typename S> __forceinline bool operator <  ( const color_t<S> &c1, const color_t<S> &c2 ) ;

   template <typename S> __forceinline S luminance_crt( color_t<S> const & col );
   template <typename S> __forceinline S luminance_lcd( color_t<S> const & col );

   template<typename S>
   struct colora_t : color_t<S>
   {
      typedef color_scalar_unit<S> unit;

      using color_t<S>::r;
      using color_t<S>::g;
      using color_t<S>::b;
      S a;

      colora_t() ;
      colora_t( S r, S g, S b, S a = unit::get()) ;
      colora_t( const color_t<S>& c, S a = unit::get() ) ;

      template<typename _S> colora_t( const colora_t<_S> &c ) ;

      colora_t & operator += ( const color_t<S> &c ) ;
      colora_t & operator -= ( const color_t<S> &c ) ;
      colora_t & operator *= ( const color_t<S> &c ) ;
      colora_t & operator /= ( const color_t<S> &c ) ;

      colora_t & operator += ( const colora_t<S> &c ) ;
      colora_t & operator -= ( const colora_t<S> &c ) ;
      colora_t & operator *= ( const colora_t<S> &c ) ;
      colora_t & operator /= ( const colora_t<S> &c ) ;

      colora_t & operator += ( S v ) ;
      colora_t & operator -= ( S v ) ;
      colora_t & operator *= ( S v ) ;
      colora_t & operator /= ( S v ) ;
   };

   #pragma pack (pop)

   template <typename S> __forceinline colora_t<S> operator + ( const colora_t<S> &c1, const color_t<S> &c2 ) ;
   template <typename S> __forceinline colora_t<S> operator - ( const colora_t<S> &c1, const color_t<S> &c2 ) ;
   template <typename S> __forceinline colora_t<S> operator * ( const colora_t<S> &c1, const color_t<S> &c2 ) ;
   template <typename S> __forceinline colora_t<S> operator / ( const colora_t<S> &c1, const color_t<S> &c2 ) ;

   template <typename S> __forceinline colora_t<S> operator + ( const colora_t<S> &c1, const colora_t<S> &c2 ) ;
   template <typename S> __forceinline colora_t<S> operator - ( const colora_t<S> &c1, const colora_t<S> &c2 ) ;
   template <typename S> __forceinline colora_t<S> operator * ( const colora_t<S> &c1, const colora_t<S> &c2 ) ;
   template <typename S> __forceinline colora_t<S> operator / ( const colora_t<S> &c1, const colora_t<S> &c2 ) ;

   template <typename S> __forceinline colora_t<S> operator + ( const colora_t<S> &c, S v ) ;
   template <typename S> __forceinline colora_t<S> operator - ( const colora_t<S> &c, S v ) ;
   template <typename S> __forceinline colora_t<S> operator * ( const colora_t<S> &c, S v ) ;
   template <typename S> __forceinline colora_t<S> operator / ( const colora_t<S> &c, S v ) ;

   template <typename S> __forceinline colora_t<S> operator + ( S v, const colora_t<S> &c ) ;
   template <typename S> __forceinline colora_t<S> operator * ( S v, const colora_t<S> &c ) ;

   template <typename S> __forceinline bool eq     ( colora_t<S> const & a, colora_t<S> const & b, S eps = color_epsilon<S>() );
   template <typename S> __forceinline bool eq_zero( colora_t<S> const & a, S eps = color_epsilon<S>() );

   template <typename S> __forceinline bool operator == ( const colora_t<S> &c1, const colora_t<S> &c2 ) ;
   template <typename S> __forceinline bool operator != ( const colora_t<S> &c1, const colora_t<S> &c2 ) ;

   template <typename S> __forceinline bool operator <  ( const colora_t<S> &c1, const colora_t<S> &c2 ) ;

   //
   typedef color_t<unsigned char> colorb ;
   typedef color_t<float>         colorf ;

   typedef colora_t<unsigned char> colorab ;
   typedef colora_t<float>         coloraf ;


   colorab color_white  () ;
   colorab color_black  () ;
   colorab color_red    () ;
   colorab color_green  () ;
   colorab color_blue   () ;
   colorab color_yellow () ;
   colorab color_magenta() ;
   colorab color_cyan   () ;

   colorb color_random () ;
}

