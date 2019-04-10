/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   template<typename scalar, int N> struct matrix_t ;

   typedef matrix_t<double, 2> matrix_2 ;
   typedef matrix_t<double, 3> matrix_3 ;
   typedef matrix_t<double, 4> matrix_4 ;

   typedef matrix_t<float, 2> matrix_2f ;
   typedef matrix_t<float, 3> matrix_3f ;
   typedef matrix_t<float, 4> matrix_4f ;

   typedef matrix_t<int, 2> matrix_2i ;
   typedef matrix_t<int, 3> matrix_3i ;
   typedef matrix_t<int, 4> matrix_4i ;
}

