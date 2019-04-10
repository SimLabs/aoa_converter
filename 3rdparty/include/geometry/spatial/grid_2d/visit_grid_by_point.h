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
   struct visit_grid1l_by_point
   {
      typedef G grid_type;

      template <class Processor>
         static bool process( grid_type & grid, geom::point_2 const & point, Processor & processor )
      {
         const point_2i p = geom::rectangle_by_extents(grid.extents()).closest_point( floor( grid.world2local( point ) ) );

         if( processor( p, grid.at( p ) ) )
            return true;

         return false;
      }
   };

   template <class T, class Processor>
      inline bool visit(grid_2d<T> & grid, geom::point_2 const & point, Processor & processor)
   {
      return visit_grid1l_by_point< grid_2d<T> >::process( grid, point, processor );
   }

   template <class T, class Processor>
      inline bool visit(grid_2d<T> const & grid, geom::point_2 const & point, Processor & processor)
   {
      return visit_grid1l_by_point< grid_2d<T> const >::process( grid, point, processor );
   }
}
