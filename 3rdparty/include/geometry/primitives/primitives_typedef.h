/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "boost/preprocessor.hpp"

#include "geometry/primitives_fwd.h"

#define PT_iterated_typedef(z, N, S)                                           \
   typedef rectangle_t  <S, N> BOOST_PP_CAT(rectangle_,   BOOST_PP_CAT(N, t)); \
   typedef transform_t  <S, N> BOOST_PP_CAT(transform_,   BOOST_PP_CAT(N, t)); \
   typedef matrix_t     <S, N> BOOST_PP_CAT(matrix_,      BOOST_PP_CAT(N, t)); \
   typedef rotation_t   <S, N> BOOST_PP_CAT(rotation_,    BOOST_PP_CAT(N, t)); \
   typedef point_t      <S, N> BOOST_PP_CAT(point_,       BOOST_PP_CAT(N, t)); \
   typedef polar_point_t<S, N> BOOST_PP_CAT(polar_point_, BOOST_PP_CAT(N, t)); \
   typedef line_t       <S, N> BOOST_PP_CAT(line_,        BOOST_PP_CAT(N, t)); \
   typedef segment_t    <S, N> BOOST_PP_CAT(segment_,     BOOST_PP_CAT(N, t)); \
   typedef triangle_t   <S, N> BOOST_PP_CAT(triangle_,    BOOST_PP_CAT(N, t)); \
   typedef sphere_t     <S, N> BOOST_PP_CAT(sphere_,      BOOST_PP_CAT(N, t));

namespace geom
{
   template< typename S >
   struct primitives_typedef
   {
      typedef typename range_t<S>      range_t;
      typedef typename cpr_t<S>        cpr_t;
      typedef typename plane_t<S>      plane_t;
      typedef typename quaternion_t<S> quaternion_t;
      typedef typename color_t<S>      color_t;

      #define  BOOST_PP_LOCAL_MACRO(n)  PT_iterated_typedef(~, n, S)
      #define  BOOST_PP_LOCAL_LIMITS    (2, 4)
      #include BOOST_PP_LOCAL_ITERATE()

      #undef BOOST_PP_LOCAL_LIMITS
      #undef BOOST_PP_LOCAL_MACRO
   };
}

#undef PT_iterated_typedef
