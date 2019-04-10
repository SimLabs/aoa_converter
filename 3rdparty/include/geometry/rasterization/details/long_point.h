/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "long_number.h"
#include "geometry/primitives/point_fwd.h"

namespace geom {
namespace rasterization {
namespace details {

   struct long_point
   {
      long_number x, y;

      explicit long_point(point_2 const & pt)
         : x(pt.x)
         , y(pt.y)
      {}

      long_point(long_number const & x, long_number const & y)
         : x(x)
         , y(y)
      {}

      long_number const &  operator [] ( size_t i ) const { Assert( i < 2 ); return (&x)[i]; }
      long_number &        operator [] ( size_t i )       { Assert( i < 2 ); return (&x)[i]; }

      long_point & operator += (long_point const & other)
      {
         x += other.x;
         y += other.y;
         return *this;
      }

      long_point & operator *= (long_point const & other)
      {
         x *= other.x;
         y *= other.y;
         return *this;
      }

      long_point & operator *= (point_2i const & other)
      {
         x *= other.x;
         y *= other.y;
         return *this;
      }

      long_point & operator *= (long_number const & a)
      {
         x *= a;
         y *= a;
         return *this;
      }

      long_point & operator -= (long_point const & other)
      {
         x -= other.x;
         y -= other.y;
         return *this;
      }

      long_point & operator /= (long_point const & other)
      {
         x /= other.x;
         y /= other.y;
         return *this;
      }

      long_point & operator /= (point_2i const & other)
      {
         x /= other.x;
         y /= other.y;
         return *this;
      }

      point_2i truncate() const
      {
//          point_2i res(x.to_int(), y.to_int());
//          if (res.x < 0 && x.is_integer())
//             ++res.x;
//          if (res.y < 0 && y.is_integer())
//             ++res.y;
//
//          return res;
  
          return point_2i(int(x), int(y));
      }
   };

}
}
}
