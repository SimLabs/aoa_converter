/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "primitives/point.h"

namespace geom
{
   template < class ScalarT, class ScalarU, class ScalarW >
      bool right_turn(point_t< ScalarT, 2 > const &a, point_t< ScalarU, 2 > const &b, point_t< ScalarW, 2 > const &c)
   {
      return ((b - a) ^ (c - a)) <= 0;
   }

   template < class ScalarT, class ScalarU, class ScalarW >
      bool left_turn(point_t< ScalarT, 2 > const &a, point_t< ScalarU, 2 > const &b, point_t< ScalarW, 2 > const &c)
   {
      return ((b - a) ^ (c - a)) >= 0;
   }

   template < class ScalarT, class ScalarU, class ScalarW >
      bool left_turn_strict(point_t< ScalarT, 2 > const &a, point_t< ScalarU, 2 > const &b, point_t< ScalarW, 2 > const &c)
   {
      return !right_turn(a,b,c);
   }

   template < class ScalarT, class ScalarU, class ScalarW >
      bool right_turn_strict(point_t< ScalarT, 2 > const &a, point_t< ScalarU, 2 > const &b, point_t< ScalarW, 2 > const &c)
   {
      return !left_turn(a,b,c);
   }

   template < class Scalar >
      bool right_turn_epsilon(point_t< Scalar, 2 > const &a, point_t< Scalar, 2 > const &b, point_t< Scalar, 2 > const &c,
                                    Scalar eps=epsilon< Scalar >())
   {
      return ((b - a) ^ (c - a)) <  eps;
   }

   template < class Scalar >
      bool left_turn_epsilon(point_t< Scalar, 2 > const &a, point_t< Scalar, 2 > const &b, point_t< Scalar, 2 > const &c,
                                    Scalar eps=epsilon<Scalar>())
   {
      return ((b - a) ^ (c - a)) > -eps;
   }

   template < class Scalar >
      point_t< Scalar, 2 > cutoff(point_t< Scalar, 2 > const &A, point_t< Scalar, 2 > const &B, Scalar L)
   {
      return A + normalized(B - A)* L;
   }

   template < class Scalar >
      point_t< Scalar, 2 > cutoff_ex(point_t< Scalar, 2 > const &A, point_t< Scalar, 2 > const &B, Scalar L)
   {
      point_t< Scalar, 2 >  C = B - A;
      Scalar d = norm(C);
      return d < L ? B : A + normalized(C)*L;
   }
}
