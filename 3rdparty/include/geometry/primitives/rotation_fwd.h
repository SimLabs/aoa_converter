/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   template<typename scalar, int N> struct rotation_t ;

   typedef rotation_t<double,2> rotation_2 ;
   typedef rotation_t<float,2>  rotation_2f ;
   typedef rotation_t<double,3> rotation_3 ;
   typedef rotation_t<float,3>  rotation_3f ;
}
