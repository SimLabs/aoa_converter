/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   // rectangle definition
   template < class, size_t >             struct rectangle_t;
   template < class, size_t >             struct rectangle_iterator_t ;

   // rectangle types

   typedef     rectangle_t< double,           2 >       rectangle_2;
   typedef     rectangle_t< float,            2 >       rectangle_2f;
   typedef     rectangle_t< short,            2 >       rectangle_2s;
   typedef     rectangle_t< int,              2 >       rectangle_2i;
   typedef     rectangle_t< long,             2 >       rectangle_2l;
   typedef     rectangle_t< unsigned char ,   2 >       rectangle_2b;
   typedef     rectangle_t< unsigned short,   2 >       rectangle_2us;
   typedef     rectangle_t< unsigned,         2 >       rectangle_2ui;
   typedef     rectangle_t< unsigned long,    2 >       rectangle_2ul;

   typedef     rectangle_t< double,           3 >       rectangle_3;
   typedef     rectangle_t< float,            3 >       rectangle_3f;
   typedef     rectangle_t< short,            3 >       rectangle_3s;
   typedef     rectangle_t< int,              3 >       rectangle_3i;
   typedef     rectangle_t< long,             3 >       rectangle_3l;
   typedef     rectangle_t< unsigned char ,   3 >       rectangle_3b;
   typedef     rectangle_t< unsigned short,   3 >       rectangle_3us;
   typedef     rectangle_t< unsigned,         3 >       rectangle_3ui;
   typedef     rectangle_t< unsigned long,    3 >       rectangle_3ul;

   typedef     rectangle_t< double,           4 >       rectangle_4;
   typedef     rectangle_t< float,            4 >       rectangle_4f;
   typedef     rectangle_t< short,            4 >       rectangle_4s;
   typedef     rectangle_t< int,              4 >       rectangle_4i;
   typedef     rectangle_t< long,             4 >       rectangle_4l;
   typedef     rectangle_t< unsigned char ,   4 >       rectangle_4b;
   typedef     rectangle_t< unsigned short,   4 >       rectangle_4us;
   typedef     rectangle_t< unsigned,         4 >       rectangle_4ui;
   typedef     rectangle_t< unsigned long,    4 >       rectangle_4ul;
}

