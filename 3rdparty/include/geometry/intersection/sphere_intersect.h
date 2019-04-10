/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
   namespace details
   {
      // Square equation
      //   if a > 0 t = fewest root from [0, 1]
      //   if a < 0 t = greatest root from [0, 1] (not used, because a = dir^2)
      //   if a = 0 assertion
      template< class S >
         bool solve2Equation( S a, S b, S c, S &t, S* t2 )
      {
         S d = b * b - 4 * a * c;

         Assert(!eq_zero(a)) ;
         if (d < 0)
            return false;

         d = geom::sqrt(d);

         S t0 = (-b - d) / (2 * a);
         S t1 = (-b + d) / (2 * a);

         if (0 <= t0 && t0 <= 1)
         {
            t = t0;
            if (t2)
               *t2 = t1;
            return true;
         }
         else if (0 <= t1 && t1 <= 1)
         {
            t = t1;
            if (t2)
               *t2 = t0;
            return true;
         }

         return false;
      }

   }

   template< class S, size_t Dim >
      bool ray_intersection( point_t< S, Dim > const& p0, point_t< S, Dim > const& p1, sphere_t< S, Dim > const& sph, S & ratio, S * ratio_next = NULL )
   {
      typedef point_t< S, Dim > point_type;

      point_type dir  = p1 - p0;
      point_type move = p0 - sph.center;

      S a = geom::norm_sqr(dir);
      S c = geom::norm_sqr(move) - sph.radius * sph.radius;

      if(geom::eq_zero(a))
      {
         ratio = 0.;
         return geom::eq_zero(c);
      }

      S b = 2 * (dir * move);

      return details::solve2Equation(a, b, c, ratio, ratio_next);
   }

   template< class S, size_t Dim >
      point_3 CalcNormal( point_t< S, Dim > const& p, sphere_t< S, Dim > const& sph )
   {
      return geom::normalized_safe( p - sph.center ) ;
   }
}
