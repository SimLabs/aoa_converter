#pragma once

namespace geom
{
   //
   template < class, size_t >             struct arc_t;

   //
   typedef     arc_t< double, 2 >       arc_2;
   typedef     arc_t< float,  2 >       arc_2f;
   typedef     arc_t< int,    2 >       arc_2i;
}
