/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   //
   template < class, size_t >             struct quad_t;

   //
   typedef     quad_t< double, 2 >       quad_2;
   typedef     quad_t< float,  2 >       quad_2f;
   typedef     quad_t< int,    2 >       quad_2i;

   typedef     quad_t< double, 3 >       quad_3;
   typedef     quad_t< float,  3 >       quad_3f;
   typedef     quad_t< int,    3 >       quad_3i;
}
