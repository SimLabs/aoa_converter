/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   //
   template < class, size_t >             struct triangle_t;

   //
   typedef     triangle_t< double, 2 >       triangle_2;
   typedef     triangle_t< float,  2 >       triangle_2f;
   typedef     triangle_t< int,    2 >       triangle_2i;

   typedef     triangle_t< double, 3 >       triangle_3;
   typedef     triangle_t< float,  3 >       triangle_3f;
   typedef     triangle_t< int,    3 >       triangle_3i;
}
