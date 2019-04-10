/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   template<typename scalar>   struct cpr_t ;
   template<typename scalar>   struct dcpr_t ;
   template<typename scalar>   struct rot_axis_t ;

   //
   typedef cpr_t<double> cpr ;
   typedef cpr_t<float>  cprf ;

   typedef dcpr_t<double> dcpr ;
   typedef dcpr_t<float>  dcprf ;

   typedef rot_axis_t<double> rot_axis ;
   typedef rot_axis_t<float > rot_axisf ;
}

