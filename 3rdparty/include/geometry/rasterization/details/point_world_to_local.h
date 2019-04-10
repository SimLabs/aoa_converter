/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/aa_transform.h"
#include "geometry/rasterization/lazy_transform.h"
#include "geometry/rasterization/details/long_point.h"

namespace geom {
namespace rasterization {
namespace details
{
   inline point_2i exact_world_to_local(aa_transform const & tform,
                                        point_2 const & p, long_point pt)
   {
      long_point origin(tform.origin()), unit(tform.unit());
      pt -= origin;
      pt /= unit;
      point_2i res = pt.truncate();
      if (p.x < tform.origin().x)
         --res.x;
      if (p.y < tform.origin().y)
         --res.y;
      return res;
   }

   inline point_2i exact_world_to_local(segment_rasterization::lazy_transform const & tform,
                                        point_2 const & p, long_point pt)
   {
      long_point origin(tform.grid_tform.origin()), unit(tform.grid_tform.unit());
      pt -= origin;
      long_point shift(unit);
      shift *= tform.cell_idx;
      pt -= shift;
      pt /= unit;
      pt *= tform.cell_extents;
      point_2i res = pt.truncate();
      if (p.x < tform.origin().x)
         --res.x;
      if (p.y < tform.origin().y)
         --res.y;
      return res;
   }

   template<class TForm>
   point_2i exact_world_to_local(TForm const & tform, point_2 const & p)
   {
      return exact_world_to_local(tform, p, long_point(p));
   }

   template<class TForm>
   inline point_2i world_to_local(TForm const & tform, point_2 const & p)
   {
      point_2 pt = tform.world2local(p);
      point_2 frac;

      for (int axis = 0; axis <= 1; ++axis)
      {
         const double EPS = 1e-6;
         frac[axis] = pt[axis] - cg::floor(pt[axis]);
         if ((frac[axis] < EPS) || (frac[axis] + EPS > 1.0))
            return exact_world_to_local(tform, p);
      }

      point_2i res = cg::floor(pt);
      for (int axis = 0; axis <= 1; ++axis)      
         if ((res[axis] < 0) && (frac[axis] == 0))
            ++res[axis];

      return res;
   }
}
}
}
