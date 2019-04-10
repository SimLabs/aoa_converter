/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "glb_base_fwd.h"

#include "glb_point.h"

#include "point.h"
#include "rectangle.h"

namespace geom
{
   struct glb_base_2 : glb_point_2
   {
      glb_base_2 () {}
      glb_base_2 ( glb_point_2 const& base ) : glb_point_2(base) {}

      glb_point_2 operator () ( const point_2f   & pos ) const { return glb_point_2 ( pos.x + X, pos.y + Y ) ; }
      glb_point_2 operator () ( const point_2    & pos ) const { return glb_point_2 ( pos.x + X, pos.y + Y ) ; }
      point_2     operator () ( const glb_point_2& pos ) const { return     point_2 ( pos.X - X, pos.Y - Y ) ; }

      glb_point_3 operator () ( const point_3f   & pos ) const { return glb_point_3 ( pos.x + X, pos.y + Y, pos.z ) ; }
      glb_point_3 operator () ( const point_3    & pos ) const { return glb_point_3 ( pos.x + X, pos.y + Y, pos.z ) ; }
      point_3     operator () ( const glb_point_3& pos ) const { return     point_3 ( pos.X - X, pos.Y - Y, pos.Z ) ; }

      glb_rect_2  operator () ( const rectangle_2f& rect ) const { return glb_rect_2 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      glb_rect_2  operator () ( const rectangle_2 & rect ) const { return glb_rect_2 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      rectangle_2 operator () ( const glb_rect_2  & rect ) const { return rectangle_2( (*this)(rect.beg),  (*this)(rect.end) ) ; }

      glb_rect_3  operator () ( const rectangle_3f& rect ) const { return glb_rect_3 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      glb_rect_3  operator () ( const rectangle_3 & rect ) const { return glb_rect_3 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      rectangle_3 operator () ( const glb_rect_3  & rect ) const { return rectangle_3( (*this)(rect.beg),  (*this)(rect.end) ) ; }

      glb_base_2& operator += ( point_2 const pos ) { X += pos.x ; Y += pos.y ; return *this ; }
   } ;

   struct glb_base_3 : glb_point_3
   {
      glb_base_3 () {}
      glb_base_3 ( glb_point_3 const& base ) : glb_point_3(base) {}
      glb_base_3 ( glb_base_2 const& base, double height ) : glb_point_3(base, height) {}

      glb_point_2 operator () ( const point_2f   & pos ) const { return glb_point_2 ( pos.x + X, pos.y + Y ) ; }
      glb_point_2 operator () ( const point_2    & pos ) const { return glb_point_2 ( pos.x + X, pos.y + Y ) ; }
      point_2     operator () ( const glb_point_2& pos ) const { return     point_2 ( pos.X - X, pos.Y - Y ) ; }

      glb_point_3 operator () ( const point_3f   & pos ) const { return glb_point_3 ( pos.x + X, pos.y + Y, pos.z + Z); }
      glb_point_3 operator () ( const point_3    & pos ) const { return glb_point_3 ( pos.x + X, pos.y + Y, pos.z + Z); }
      point_3     operator () ( const glb_point_3& pos ) const { return     point_3 ( pos.X - X, pos.Y - Y, pos.Z - Z); }

      glb_rect_2  operator () ( const rectangle_2f& rect ) const { return glb_rect_2 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      glb_rect_2  operator () ( const rectangle_2 & rect ) const { return glb_rect_2 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      rectangle_2 operator () ( const glb_rect_2  & rect ) const { return rectangle_2( (*this)(rect.beg),  (*this)(rect.end) ) ; }

      glb_rect_3  operator () ( const rectangle_3f& rect ) const { return glb_rect_3 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      glb_rect_3  operator () ( const rectangle_3 & rect ) const { return glb_rect_3 ( (*this)(rect.lo()), (*this)(rect.hi()) ) ; }
      rectangle_3 operator () ( const glb_rect_3  & rect ) const { return rectangle_3( (*this)(rect.beg),  (*this)(rect.end) ) ; }

      glb_base_3& operator += ( point_3 const pos ) { X += pos.x ; Y += pos.y ; Z += pos.z ; return *this ; }
   } ;

}
