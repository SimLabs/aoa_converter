/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   template<typename scalar, int N> struct polar_point_t ;

   typedef polar_point_t<double,2> polar_point_2 ;
   typedef polar_point_t<double,3> polar_point_3 ;

   typedef polar_point_t<float,2> polar_point_2f ;
   typedef polar_point_t<float,3> polar_point_3f ;
}

