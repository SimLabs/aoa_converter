/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <cmath>
#include <math.h>
#include <stdlib.h>
#include <limits>
#include <common/meta.h>

#include "scalar_traits.h"
#include "geometry/primitives/point_fwd.h"

#undef min
#undef max

#if !defined(_MSC_VER)
    #define __forceinline inline
#endif

#pragma pack ( push, 1 )

namespace geom
{
   template <class type>   type    epsilon() { return type(); }
   template <> inline      double  epsilon<double>() { return 1e-10; }
   template <> inline      float   epsilon<float >() { return 1e-6f; }

   const double pi  = 3.14159265358979323846;
   const float  pif = 3.14159265f;

   const double pi_2  = 0.5 * pi;
   const float  pi_2f = float(pi_2);

   template <typename Scalar> inline Scalar mod ( Scalar x, Scalar y ) { return x % y ; }

   inline double mod ( double x, double y ) { return fmod( x, y ); }
   inline float  mod ( float  x, float  y ) { return fmodf( x, y ); }

   inline double sqrt( double x )  { return ::sqrt( x ); }
   inline float  sqrt( float x )   { return ::sqrt( x ); }

   inline int round(double x) { return x > 0 ? int(x + .5) : int(x - .5); }
   inline int round(float  x) { return x > 0 ? int(x + .5f) : int(x - .5f); }

   inline int floor(double f) { return (int)::floor( f ); }
   inline int floor(float f)  { return (int)::floor( f ); }

   inline int ceil(double f) { return (int)::ceil( f ); }
   inline int ceil(float f)  { return (int)::ceil( f ); }

   template <typename Scalar> inline Scalar abs ( Scalar x ) { return x >= 0 ? x : -x; }

   inline double round(double x, double step)
   {
      return step * round(x / step);
   }


   inline double grad2rad(double grad) { return grad * pi / 180.0; }
   inline float  grad2rad(float  grad) { return grad * float( pi / 180.0f ); }
   inline double grad2rad(int    grad) { return grad * pi / 180.0; }
   inline double grad2rad()            { return pi / 180.0; }

   inline double rad2grad(double rad)  { return rad * 180.0 / pi; }
   inline float  rad2grad(float  rad)  { return rad * float( 180.0 / pi ); }
   inline double rad2grad(int    rad)  { return rad * 180.0 / pi; }
   inline double rad2grad()            { return 180.0 / pi; }

   inline double feet2meter()      { return 0.3048; }
   inline double meter2feet()      { return 1. / feet2meter(); }
   inline double miles2meter()     { return 1609.344; }                 // statute mile
   inline double meter2miles()     { return 1. / miles2meter(); }
   inline double nmiles2meter()    { return 1852.; }                    // nautical mile
   inline double meter2nmiles()    { return 1. / nmiles2meter(); }
   inline double kt2mps()          { return 0.514444444444444; }
   inline double mps2kt()          { return 1. / kt2mps(); }
   inline double mps2kmph()        { return 3.6; }
   inline double kmph2mps()        { return 1. / mps2kmph(); }
   inline double kmph2kt()         { return kmph2mps() * mps2kt(); }
   inline double kt2kmph()         { return kt2mps() * mps2kmph(); }
   inline double ftm2mps()         { return feet2meter() / 60. ; }
   inline double mps2ftm()         { return 1. / ftm2mps() ; }

   double const abs_zero  = 273.15 ;
   float  const abs_zerof = 273.15f;

   inline double kelv2grad(double kelv) { return kelv - abs_zero ; }
   inline float  kelv2grad(float  kelv) { return kelv - abs_zerof; }
   inline double grad2kelv(double celc) { return celc + abs_zero ; }
   inline float  grad2kelv(float  celc) { return celc + abs_zerof; }

   inline double fahr2kelv(double fahr) { return (fahr  + 459.67) * 5. / 9.; }
   inline float  fahr2kelv(float  fahr) { return (fahr  + 459.67f) * 5.f / 9.f; }

   inline double kelv2fahr(double kelv) { return kelv * 9. / 5. - 459.67; }
   inline float  kelv2fahr(float  kelv) { return kelv * 9.f / 5.f - 459.67f; }

   inline double Pa2mb() { return 0.01; }
   inline double Pa2mmHg() { return 0.00750061683; }
   inline double Pa2inHg() { return 0.0002961339710085; }


   template <class T> inline T sqr(T x) { return x * x; }

   inline double rand(double max) { return max * ::rand() / (double)RAND_MAX; }
   inline float  rand(float max)  { return max * ::rand() / (float) RAND_MAX; }
   inline int    rand(int max)    { return ::rand() % max; } 

   inline double symmetric_rand(double max) { return rand(max * 2.0)  - max; }
   inline float  symmetric_rand(float max)  { return rand(max * 2.0f) - max; }
   inline int    symmetric_rand(int max)    { return rand(max * 2)    - max; }

   inline bool is_inf(double x) { return !( -1e30 < x && x < 1e30 ); }

   inline bool between01(double x) { return 0 <= x && x <= 1; }

   inline bool between01so(double x) { return 0 <= x && x < 1; }

   inline bool between01eps(double x, double eps=epsilon< double >( )) { return -eps <= x && x - 1 <= eps; }

   template <class T> T blend(T const & a, T const & b, double t)
   {
      return (1-t)*a + t*b;
   }

   template <class T> T blend(T const & a, T const & b, float t)
   {
      return (1-t)*a + t*b;
   }

   // OK
   template <class T> T lerp01( T const & a, T const & b, double t )
   {
      return a * (1 - t) + b * t;
   }
   template <class T> T lerp01( T const & a, T const & b, float t )
   {
      return a * (1.0f - t) + b * t;
   }

   inline int sign (int    x)  { return x < 0 ? -1 : x > 0 ? 1 : 0; }
   inline int sign (float  x)  { return x < 0 ? -1 : x > 0 ? 1 : 0; }
   inline int sign (double x)  { return x < 0 ? -1 : x > 0 ? 1 : 0; }

   inline bool different_signs(double x, double y)
   {   return x * y < 0; }


   // fuzzy greater equal
   inline bool ge (float a,  float b,  float  eps = epsilon< float >( ) ) { return a - b >= - eps; }
   inline bool ge (double a, double b, double eps = epsilon< double >( )) { return a - b >= - eps; }

   // fuzzy greater than
   inline bool gt (float a,  float b,  float  eps = epsilon< float >( ) ) { return a - b >= + eps; }
   inline bool gt (double a, double b, double eps = epsilon< double >( )) { return a - b >= + eps; }

   // fuzzy less equal
   inline bool le (float a,  float b,  float  eps = epsilon< float >( ) ) { return a - b <= + eps; }
   inline bool le (double a, double b, double eps = epsilon< double >( )) { return a - b <= + eps; }

   // fuzzy less than
   inline bool lt (float a,  float b,  float  eps = epsilon< float >( ) ) { return a - b <= - eps; }
   inline bool lt (double a, double b, double eps = epsilon< double >( )) { return a - b <= - eps; }

   // fuzzy equal
   inline bool eq(int a, int b) { return a == b; }
   inline bool eq (float a,  float b,  float  eps = epsilon< float >( )) { return abs(a - b) <= eps; }
   //inline bool eq (float a,  int   b,  float  eps = epsilon< float >( )) { return abs(a - b) <= eps; }
   inline bool eq (double a,  double b,  double  eps = epsilon< double >( )) { return abs(a - b) <= eps; }
   //inline bool eq (double a,  int    b,  double  eps = epsilon< double >( )) { return abs(a - b) <= eps; }
   //inline bool eq (float  a,  int    b,  double  eps = epsilon< double >( )) { return abs((double)a - b) <= eps; }

   // fuzzy equality to 0
   inline bool eq_zero (float  a, float  eps = epsilon< float >( ) ) { return abs(a) <= eps; }
   inline bool eq_zero (double a, double eps = epsilon< double >( )) { return abs(a) <= eps; }
   inline bool eq_zero (int a, double eps = epsilon< int >( )) { return abs(double(a)) <= eps; }

   namespace details
   {
      template < class Scalar >
      inline bool eq_rel( Scalar a, Scalar b, Scalar max_relative_error, Scalar max_absolute_error )
      {
          return abs( a - b ) <= std::max( max_absolute_error, max_relative_error * std::max( abs( a ), abs( b ) ) );
      }
   }

   inline bool eq_rel(double a, double b, double max_relative_error = epsilon< double >(), double max_absolute_error = epsilon< double >())
   {
      return details::eq_rel( a, b, max_relative_error, max_absolute_error );
   }

   inline bool eq_rel(float a, float b, float max_relative_error = epsilon< float >(), float max_absolute_error = epsilon< float >())
   {
      return details::eq_rel( a, b, max_relative_error, max_absolute_error );
   }

   inline bool le_rel(double a, double b, double max_relative_error = epsilon< double >(), double max_absolute_error = epsilon< double >())
   {
      return a < b || eq_rel( a, b, max_relative_error, max_absolute_error );
   }

   inline bool ge_rel(float a, float b, float max_relative_error = epsilon< float >(), float max_absolute_error = epsilon< float >())
   {
      return a > b || eq_rel( a, b, max_relative_error, max_absolute_error );
   }

   template < class Scalar > inline Scalar adjust(Scalar x, Scalar e = epsilon< Scalar >( )) { return eq_zero(x, e) ? 0 : x; }

   inline bool is_closer(double x, double step, double dist = epsilon< double >( ))
   {
      return eq(x, round(x, step), dist);
   }

   template <class T> inline bool make_min(T &to_be_min, T x) { if (to_be_min > x) {to_be_min = x; return true; } return false; }
   template <class T> inline bool make_max(T &to_be_max, T x) { if (to_be_max < x) {to_be_max = x; return true; } return false; }

   template <typename T, typename D>
   void inline make_min ( T & to_be_min, T x, D & assign_if, D const & y )
   {
      if ( to_be_min > x )
      {
         to_be_min = x;
         assign_if = y;
      }
   }

   template <typename T, typename D>
   void inline make_max ( T & to_be_max, T x, D & assign_if, D const & y )
   {
      if ( to_be_max < x )
      {
         to_be_max = x;
         assign_if = y;
      }
   }

   template <class T> inline bool make_min_ret(T &to_be_min, T x) { if (to_be_min > x) { to_be_min = x; return true; } return false; }
   template <class T> inline bool make_max_ret(T &to_be_max, T x) { if (to_be_max < x) { to_be_max = x; return true; } return false; }

   inline int min(int a, int b) { return a < b ? a : b; }
   inline int max(int a, int b) { return a > b ? a : b; }

   inline unsigned min(unsigned a, unsigned b) { return a < b ? a : b; }
   inline unsigned max(unsigned a, unsigned b) { return a > b ? a : b; }

   inline unsigned long min(unsigned long a, unsigned long b) { return a < b ? a : b; }
   inline unsigned long max(unsigned long a, unsigned long b) { return a > b ? a : b; }

   inline long long min(long long a, long long b) { return a < b ? a : b; }
   inline long long max(long long a, long long b) { return a > b ? a : b; }

   inline unsigned long long min(unsigned long long a, unsigned long long b) { return a < b ? a : b; }
   inline unsigned long long max(unsigned long long a, unsigned long long b) { return a > b ? a : b; }

   inline double min(double a, double b) { return a < b ? a : b; }
   inline double max(double a, double b) { return a > b ? a : b; }

   inline float min(float a, float b) { return a < b ? a : b; }
   inline float max(float a, float b) { return a > b ? a : b; }

   template <class T> inline T max(T a, T b, T c) { return max(a, max(b,c)); }
   template <class T> inline T min(T a, T b, T c) { return min(a, min(b,c)); }

   template <class T> inline T max(T a, T b, T c, T d) { return max(max(a,b),max(c,d)); }
   template <class T> inline T min(T a, T b, T c, T d) { return min(min(a,b),min(c,d)); }

   inline bool is_finite(double x)
   {
#if defined _WIN32 || defined __CYGWIN__
       return _finite(x) != 0;
#else
       return std::isfinite(x);
#endif
   }

   inline bool is_pow2( long x )
   {
      if ( x <= 0 )
         return false;
      return ( (x & (x - 1)) == 0 );
   }


   inline int log2i( long x )
   {
      if ( x <= 0 )
         return -1;
      int res = 0;
      while ( x >>= 1 )
         res++;
      return res;
   }

   template <class T, class D = T>
      struct Lerp
   {
      typedef
         typename meta::_if< meta::_is_integral< D >, T, D >::type
         tform_type;

      inline Lerp(T x0, T x1, D y0, D y1)
         : K_(geom::eq_zero(x1 - x0) ? tform_type() * T(0.) : ( y1 - y0 ) * ( T(1) / (x1 - x0) ))
         , D_( y0 - K_ * x0 )
      {}

      inline D operator() (T x) const {
         return ( D ) ( K_*x + D_ );
      }

      inline tform_type const& K() const {
         return K_ ;
      }

      inline tform_type const& B() const {
         return D_ ;
      }

   private:
      tform_type K_;
      tform_type D_;
   };

   template <class T> inline  Lerp<T> lerp(T x0, T x1, T y0, T y1)
   {
      return Lerp<T>(x0,x1,y0,y1);
   }

   template <class T, class D> inline  Lerp<T,D> lerp_d(T x0, T x1, D y0, D y1)
   {
      return Lerp<T,D>(x0,x1,y0,y1);
   }

   template <class V, class S> inline  V slerp(S x0, S x1, V y0, V y1, S x)
   {
      S t = (x - x0) / (x1 - x0);
      S s = (3 - 2 * t) * t * t;

      return y0 * (1 - s) + y1 * s;
   }

   // three-point lerp
   template <class T, class D = T>
      struct Lerp3
   {
      inline Lerp3(T x0, T x1, T x2, D y0, D y1, D y2)
         : K1_(geom::eq_zero(x1 - x0) ? D() * T(0.) : (y1 - y0) * ( T(1) / (x1 - x0) ) )
         , D1_(y0 - K1_*x0)
         , K2_(geom::eq_zero(x2 - x1) ? D() * T(0.) : (y2 - y1) * ( T(1) / (x2 - x1) ) )
         , D2_(y1 - K2_*x1)
         , xx_(x1)
      {}

      inline D operator() (T x) const {
         return x < xx_
            ?  x*K1_ + D1_
            :  x*K2_ + D2_;
      }

   private:
      D    K1_,D1_;
      D    K2_,D2_;
      T    xx_;
   };

   template <class T> inline  Lerp3<T> lerp3(T x0, T x1, T x2,  T y0, T y1, T y2)
   {
      return Lerp3<T>(x0,x1,x2, y0,y1,y2);
   }

   // out1 = min(in1,in2); out2 = max(in1,in2);
   template <class T>
      inline void sort2(T in1, T in2, T &out1, T &out2)
   {
      if (in1 < in2) { out1 = in1; out2 = in2; }
      else           { out1 = in2; out2 = in1; }
   }

   template <class T>
      inline void sort2(T & v1, T & v2)
   {
      if (v1 > v2)
         std::swap(v1, v2);
   }

   template <class T>
      inline void sort3(T & v1, T & v2, T & v3)
   {
      if (v1 > v2)
      {
         if (v1 > v3)
         {
            std::swap(v1, v3);
            if (v1 > v2)
               std::swap(v1, v2);
         }
         else
            std::swap(v1, v2);
      }
      else
      {
         if (v2 > v3)
         {
            std::swap(v2, v3);
            if (v1 > v2)
               std::swap(v1, v2);
         }
      }
   }

   template <class T, class Comp>
   inline void sort3( T & v1, T & v2, T & v3, Comp const & comp )
   {
      if (comp(v2, v1))
      {
         if (comp(v3, v1))
         {
            std::swap(v1, v3);
            if (comp(v2, v1))
               std::swap(v1, v2);
         }
         else
            std::swap(v1, v2);
      }
      else
      {
         if (comp(v3, v2))
         {
            std::swap(v2, v3);
            if (comp(v2, v1))
               std::swap(v1, v2);
         }
      }
   }

   template <class T, class D = T>
      struct Clamp
   {
      inline Clamp(T x0, T x1, D y0, D y1)
         : x0(x0), x1(x1), y0(y0), y1(y1)
         , l_( x0, x1, y0, y1 )
      {}

      inline D operator () (T x) const {
         return
            x <= x0 ? y0 :
            x >= x1 ? y1 :
            l_(x);
      }

   private:
      T   x0, x1;
      D   y0, y1;
      Lerp<T, D> l_;
   };

   template <class T> inline  Clamp<T> clamp(T x0, T x1, T y0, T y1)
   {
      return Clamp<T>(x0, x1, y0, y1);
   }

   template <class T, class D> inline  Clamp<T,D> clamp_d(T x0, T x1, D y0, D y1)
   {
      return Clamp<T,D>(x0,x1,y0,y1);
   }

   template <class V, class S> inline  V sclamp(S x0, S x1, V y0, V y1, S x)
   {
      if (x <= x0) return y0;
      if (x >= x1) return y1;
      return slerp<V, S>(x0, x1, y0, y1, x);
   }

   // non-linear clamp (three-point clamp)
   template <class T, class D = T>
      struct Clamp3
   {
      inline Clamp3(T x0, T x1, T x2,  D y0, D y1, D y2)
         :   x0(x0), x2(x2),  y0(y0), y2(y2)
         ,   l_( x0, x1, x2, y0, y1, y2 )
      {}

      inline D operator () (T x) const {
         return
            x <= x0 ? y0 :
            x >= x2 ? y2 :
            l_(x);
      }

   private:
      T   x0, x2;
      D   y0, y2;
      Lerp3<T,D> l_;
   };

   template <class T> inline  Clamp3<T> clamp3(T x0, T x1, T x2,  T y0, T y1, T y2)
   {
      return Clamp3<T>(x0, x1, x2,  y0, y1, y2);
   }

   template < class T >
      struct Bound
   {
      Bound( T min_val, T max_val )
         : min_val_( min_val )
         , max_val_( max_val )
      {}

      T operator( ) ( T val ) const
      {
         return val > max_val_ ? max_val_ : val < min_val_ ? min_val_ : val ;
      }

   private:
      T min_val_;
      T max_val_;
   };

   template<class T> inline
      Bound<T> bound( T vmin, T vmax )
   {
      return Bound<T>( vmin, vmax );
   }

   template<class T> inline
      T bound(T x, T vmin, T vmax)
   {
      return x < vmin ? vmin : x > vmax ? vmax : x;
   }

   template<class scalar> inline
       point_t<scalar, 2> vbound( point_t<scalar, 2> v, point_t<scalar, 2> const & vmin, point_t<scalar, 2> const & vmax)
   {
       v.x = (v.x < vmin.x) ? vmin.x : (v.x > vmax.x) ? vmax.x : v.x;
       v.y = (v.y < vmin.y) ? vmin.y : (v.y > vmax.y) ? vmax.y : v.y;
       return v;
   }

   template<class scalar> inline
       point_t<scalar, 3> vbound( point_t<scalar, 3> v, point_t<scalar, 3> const & vmin, point_t<scalar, 3> const & vmax)
   {
       v.x = (v.x < vmin.x) ? vmin.x : (v.x > vmax.x) ? vmax.x : v.x;
       v.y = (v.y < vmin.y) ? vmin.y : (v.y > vmax.y) ? vmax.y : v.y;
       v.z = (v.z < vmin.z) ? vmin.z : (v.z > vmax.z) ? vmax.z : v.z;
       return v;
   }

   /* Next & previous indexes for closed (cyclic) arrays */
   int inline prev(int index, int size)
   {
      return (index == 0) ? size - 1 : index - 1;
   }

   int inline next(int index, int size)
   {
      return (index == size - 1) ? 0 : index + 1;
   }

   template<class T, bool integer> struct norm360_impl; 

   template<class T> struct norm360_impl<T, true>
   {
      static T f(T x)
      {
         if ( x > 0 )
            return x % 360;
         else if ( x < 0 )
            return (360 - (-x-1) % 360) - 1;
         return x ;
      }
   } ; 

   template<class T> struct norm360_impl<T, false>
   {
      static T f(T x)
      {
         return x - T(360) * floor(x / T(360));
      }
   } ; 

   template <class T> T norm360 ( T x ) 
   {
      return norm360_impl<T,is_integer_f<T>::value>::f(x);
   }

   // [-180, 180)
   template < class T >
      inline T norm180 ( T x )
   {
       static_assert(std::is_signed<T>::value, "Unsigned arg for norm180");
       x = norm360(x) ;
      if ( x >= 180 )
         x -= 360 ;
      return x ;
   }

   // [0, 2*Pi)
   inline double norm_2pi ( double x )
   {
      return x - 2*pi * floor(x / (2*pi));
   }

   // [-Pi, Pi)
   inline double norm_pi ( double x )
   {
      x = norm_2pi(x) ;
      if ( x >= pi )
         x -= 2*pi ;
      return x ;
   }

   inline double norm( double value )
   {
      return abs( value ) ;
   }

   inline float norm( float value )
   {
      return abs( value ) ;
   }

   inline double distance_sqr(double A, double B)
   {   return (A - B)*(A - B); }

   inline double distance(double A, double B)
   {   return abs(A - B); }

   // greatest common divisor
   template<class T>
      inline T gcd( T a, T b )
   {
      BOOST_STATIC_ASSERT_MSG(meta::_is_integral<T>::value, "T_must_be_integral");
      if ( b == 0 ) return a;
      return gcd( b, a % b );
   }

   inline bool is_power_of_two ( size_t value )
   {
      return (value & (value - 1)) == 0 ;
   }

    // next float
    inline float next_float( float f )
    {
        return reinterpret_cast<float &>(++reinterpret_cast<uint32_t &>(f));
    }

    // prev float
    inline float prev_float( float f )
    {
        return reinterpret_cast<float &>(--reinterpret_cast<uint32_t &>(f));
    }

    // TBD: fix under gcc and linux

    template<typename T>
    inline void fsincos( T angle, T * sine, T * cosine );

    template<>
    inline void fsincos( float angle, float * sine, float * cosine )
    {
    #if defined(_M_IX86)
        __asm
        {
            fld dword ptr [angle]
            fsincos
            mov eax, [cosine]
            fstp dword ptr [eax]
            mov eax, [sine]
            fstp dword ptr [eax]
        }
    #else // _M_IX86
        *sine = sin(angle);
        *cosine = cos(angle);
    #endif // _M_IX86
    }

    template<>
    inline void fsincos( double angle, double * sine, double * cosine )
    {
    #if defined(_M_IX86)
        __asm
        {
            fld qword ptr [angle]
            fsincos
            mov eax, [cosine]
            fstp qword ptr [eax]
            mov eax, [sine]
            fstp qword ptr [eax]
        }
    #else // _M_IX86
        *sine = sin(angle);
        *cosine = cos(angle);
    #endif // _M_IX86
    }

}

#pragma pack ( pop )
