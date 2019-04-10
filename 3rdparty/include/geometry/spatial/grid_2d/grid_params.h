/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/aa_transform.h"

#pragma pack( push, 1 )

namespace geom
{
   struct grid_params_type
   {
      grid_params_type() {}

      grid_params_type(point_2 const & org, point_2 const & unt, point_2i const & ext)
         :   tform_  (org, unt)
         ,   extents_(ext)
      {}

      aa_transform const & tform   ()  const { return tform_;           }
      point_2 const &      origin  ()  const { return tform_.origin();  }
      point_2 const &      unit    ()  const { return tform_.unit();    }
      point_2i const &     extents ()  const { return extents_;         }

   private:
      aa_transform    tform_;
      point_2i        extents_;
   };

   inline grid_params_type calc_grid_params( geom::rectangle_2 const & aabb, double step )
   {
      geom::point_2 const unit( step, step );
      return grid_params_type( aabb.lo(), unit, geom::ceil( aabb.size() / unit ) );
   }
}

#pragma pack( pop )

