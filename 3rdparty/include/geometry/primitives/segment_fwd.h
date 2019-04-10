/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   //
   template < class, size_t >             struct segment_t;

   //
   typedef     segment_t< double, 2 >       segment_2;
   typedef     segment_t< float,  2 >       segment_2f;
   typedef     segment_t< int,    2 >       segment_2i;

   typedef     segment_t< double, 3 >       segment_3;
   typedef     segment_t< float,  3 >       segment_3f;
   typedef     segment_t< int,    3 >       segment_3i;
}
