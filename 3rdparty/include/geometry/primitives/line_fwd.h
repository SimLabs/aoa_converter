/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   // line definition
   template < class, size_t >             struct line_t;

   // point types
   typedef     line_t< double, 2 >       line_2;
   typedef     line_t< float,  2 >       line_2f;
   typedef     line_t< int,    2 >       line_2i;

   typedef     line_t< double, 3 >       line_3;
   typedef     line_t< float,  3 >       line_3f;
   typedef     line_t< int,    3 >       line_3i;
}

