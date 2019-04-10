/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   // point definition
   template < class, size_t >             struct point_t;
   template < class Struct, size_t Dim >  struct point_decomposition_t;

   // point types
   typedef     point_t< double,           2 >       point_2;
   typedef     point_t< float,            2 >       point_2f;
   typedef     point_t< char,             2 >       point_2c;
   typedef     point_t< short,            2 >       point_2s;
   typedef     point_t< int,              2 >       point_2i;
   typedef     point_t< long,             2 >       point_2l;
   typedef     point_t< unsigned char ,   2 >       point_2b;
   typedef     point_t< unsigned char ,   2 >       point_2uc;
   typedef     point_t< unsigned short,   2 >       point_2us;
   typedef     point_t< unsigned int,     2 >       point_2ui;
   typedef     point_t< unsigned long,    2 >       point_2ul;

   typedef     point_t< double,           3 >       point_3;
   typedef     point_t< float,            3 >       point_3f;
   typedef     point_t< char,             3 >       point_3c;
   typedef     point_t< short,            3 >       point_3s;
   typedef     point_t< int,              3 >       point_3i;
   typedef     point_t< long,             3 >       point_3l;
   typedef     point_t< unsigned char ,   3 >       point_3b;
   typedef     point_t< unsigned char ,   3 >       point_3uc;
   typedef     point_t< unsigned short,   3 >       point_3us;
   typedef     point_t< unsigned int,     3 >       point_3ui;
   typedef     point_t< unsigned long,    3 >       point_3ul;

   typedef     point_t< double,           4 >       point_4;
   typedef     point_t< float,            4 >       point_4f;
   typedef     point_t< char,             4 >       point_4c;
   typedef     point_t< short,            4 >       point_4s;
   typedef     point_t< int,              4 >       point_4i;
   typedef     point_t< long,             4 >       point_4l;
   typedef     point_t< unsigned char ,   4 >       point_4b;
   typedef     point_t< unsigned char ,   4 >       point_4uc;
   typedef     point_t< unsigned short,   4 >       point_4us;
   typedef     point_t< unsigned int,     4 >       point_4ui;
   typedef     point_t< unsigned long,    4 >       point_4ul;

   typedef     point_decomposition_t< double, 2 >       point_decomposition_2;
   typedef     point_decomposition_t< float,  2 >       point_decomposition_2f;

   typedef     point_decomposition_t< double, 3 >       point_decomposition_3;
   typedef     point_decomposition_t< float,  3 >       point_decomposition_3f;

   typedef     point_decomposition_t< double, 4 >       point_decomposition_4;
   typedef     point_decomposition_t< float,  4 >       point_decomposition_4f;
}

