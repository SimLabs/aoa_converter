/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   //
   template < class, size_t >             struct sphere_t;

   //
   typedef     sphere_t< double, 2 >       sphere_2;
   typedef     sphere_t< float,  2 >       sphere_2f;
   typedef     sphere_t< int,    2 >       sphere_2i;

   typedef     sphere_t< double, 3 >       sphere_3;
   typedef     sphere_t< float,  3 >       sphere_3f;
   typedef     sphere_t< int,    3 >       sphere_3i;
}
