/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom {
namespace segment_rasterization {

   struct lazy_transform
   {
      aa_transform const & grid_tform;
      point_2i const & cell_idx;
      const point_2i cell_extents;

      template<class Grid>
      lazy_transform(Grid const & grid, point_2i const & cell_idx)
         : grid_tform(grid.tform())
         , cell_idx(cell_idx)
         , cell_extents(grid[cell_idx].extents())
      {}

      point_2 origin() const 
      { 
         return grid_tform.local2world(cell_idx);
      }

      point_2 unit() const 
      {
         return grid_tform.unit() / cell_extents; 
      }

      point_2 world2local(point_2 const & pt) const
      {
         return (pt - origin()) / grid_tform.unit() & cell_extents;
      }
   };

}
}
