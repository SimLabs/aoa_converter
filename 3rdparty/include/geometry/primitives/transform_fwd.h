/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   //
   template<typename scalar, size_t N> struct transform_t ;

   //
   template<typename S, size_t N> struct translation_t ;
   template<typename S, size_t N> struct vector_t      ;
   template<typename S, size_t N> struct normal_t      ;
   template<typename S, size_t N> struct scale_t       ;

   typedef transform_t<double,4> transform_4 ;
   typedef transform_t<double,3> transform_3 ;
   typedef transform_t<float,4> transform_4f ;
   typedef transform_t<float,3> transform_3f ;

   typedef scale_t<double,2> scale_2 ;
   typedef scale_t<double,3> scale_3 ;
   typedef scale_t<float,2>  scale_2f ;
   typedef scale_t<float,3>  scale_3f ;

   typedef normal_t<double,2> normal_2 ;
   typedef normal_t<double,3> normal_3 ;
   typedef normal_t<float,2>  normal_2f ;
   typedef normal_t<float,3>  normal_3f ;

   typedef translation_t<double,2> translation_2 ;
   typedef translation_t<double,3> translation_3 ;
   typedef translation_t<float,2>  translation_2f ;
   typedef translation_t<float,3>  translation_3f ;

   typedef vector_t<double,2> vector_2 ;
   typedef vector_t<double,3> vector_3 ;
   typedef vector_t<float,2>  vector_2f ;
   typedef vector_t<float,3>  vector_3f ;

}

