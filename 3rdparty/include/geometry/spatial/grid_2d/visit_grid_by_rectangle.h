/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "grid.h"

namespace geom
{

   template <class G>
   struct visit_grid1l_by_rectangle
   {
      typedef G                       grid_type;
      typedef point_2i                smallidx_type;

      template <class Processor>
         static bool process( grid_type & grid, geom::rectangle_2 const & rect, Processor & processor )
      {
         if (!geom::has_intersection(rect, bounding(grid)))
             return false;

         geom::rectangle_2i ind_rect( point_2i( 0, 0 ), grid.extents( ) - point_2i( 1, 1 ) );

         const point_2i beg = ind_rect.closest_point( floor( grid.world2local( rect.xy( ) ) ) );
         const point_2i end = ind_rect.closest_point( floor( grid.world2local( rect.XY( ) ) ) );

         for( point_2i p = beg; p.x <= end.x; ++p.x ){
         for( p.y = beg.y; p.y <= end.y; ++p.y )
         {
            if( processor( p, grid.at( p ) ) )
               return true;
         }}
         return false;
      }
   };

   template <class T, class Processor>
      inline bool visit(grid_2d<T> & grid, geom::rectangle_2 const & rect, Processor & processor)
   {
      return visit_grid1l_by_rectangle< grid_2d<T> >::process( grid, rect, processor );
   }

   template <class T, class Processor>
      inline bool visit(grid_2d<T> const & grid, geom::rectangle_2 const & rect, Processor & processor)
   {
      return visit_grid1l_by_rectangle< grid_2d<T> const >::process( grid, rect, processor );
   }
}
