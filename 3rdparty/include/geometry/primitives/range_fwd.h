/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   // range definition
   template < class >             struct range_t;

   // point types
   typedef     range_t< double         > range_2;
   typedef     range_t< float          > range_2f;
   typedef     range_t< int            > range_2i;
   typedef     range_t< unsigned char  > range_2b;
   typedef     range_t< unsigned short > range_2us;
   typedef     range_t< short          > range_2s;
   typedef     range_t<unsigned int    > range_2ui;

}

