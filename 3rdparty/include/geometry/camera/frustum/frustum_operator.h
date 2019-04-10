/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "frustum_perspective.h"
#include "frustum_orthographic.h"

namespace geom
{
   template< class S >
   frustum_orthographic_t<S> change_frustum_type( geom::frustum_perspective_t<S> const & frp )
   {
      frustum_orthographic_t<S> result = frustum_orthographic_t<S>(frp.camera(), frp.near_clip_rect(), frp.clipping());
      result.set_reversed_z(frp.reversed_z());
      return result;
   }

   template< class S >
   frustum_perspective_t<S> change_frustum_type( geom::frustum_orthographic_t<S> const & fro )
   {
      typedef geom::frustum_orthographic_t<S> frustum_type;

      float const clipping_near = fro.clipping_near(),
                  clipping_far  = fro.clipping_far ();
      rectangle_2f const near_clip_rect = fro.near_clip_rect();
      point_2f const near_clip_center_pos(near_clip_rect.center());

      float const hleft   = geom::rad2grad(atanf(near_clip_rect.lo().x / clipping_near));
      float const hright  = geom::rad2grad(atanf(near_clip_rect.hi().x / clipping_near));
      float const vbottom = geom::rad2grad(atanf(near_clip_rect.lo().y / clipping_near));
      float const vtop    = geom::rad2grad(atanf(near_clip_rect.hi().y / clipping_near));

      float const hfov = hright - hleft;
      float const vfov = vbottom - vtop;
      float const hdef = (hright + hleft) / 2;

      float const vdef = (vbottom + vtop) / 2;

      frustum_perspective_t<S> result = geom::frustum_perspective_t<S>(fro.camera(), rectangle_3f(range_2f(hleft, hright), range_2f(vbottom, vtop), range_2f(clipping_near, clipping_far)));
      result.set_reversed_z(fro.reversed_z());

      return result;
   }

   template< class S >
   rectangle_2f field_of_view_by_near_clip_rect( frustum_t<S> const & fr, rectangle_2f const & rect )
   {
      if (fr.type() == FR_orthographic)
         return rect;

      Assert(fr.type() == geom::FR_perspective);

      float const near_clip = fr.clipping_near();

      point_2f const lo(geom::rad2grad(atan(rect.x.lo() / near_clip)), geom::rad2grad(atan(rect.y.lo() / near_clip)));
      point_2f const hi(geom::rad2grad(atan(rect.x.hi() / near_clip)), geom::rad2grad(atan(rect.y.hi() / near_clip)));

      return rectangle_2f(lo, hi);
   }

   template< class S >
   rectangle_2f near_clip_rect_by_field_of_view( frustum_t<S> const & fr, rectangle_2f const & rect )
   {
      if (fr.type() == FR_orthographic)
         return rect;

      Assert(fr.type() == geom::FR_perspective);

      float const near_clip = fr.clipping_near();

      point_2f const lo = point_2f(tan(geom::grad2rad(rect.x.lo())), tan(geom::grad2rad(rect.y.lo()))) * near_clip;
      point_2f const hi = point_2f(tan(geom::grad2rad(rect.x.hi())), tan(geom::grad2rad(rect.y.hi()))) * near_clip;

      return rectangle_2f(lo, hi);
   }

   template< class S >
   float frustum_z_by_pseudo_depth( frustum_t<S> const & fr, float pseudo_depth )
   {
      matrix_4f const & proj = fr.projection_matrix();

      return proj(2, 3) / (pseudo_depth * proj(3, 2) - proj(2, 2));
   }

   template< class S >
   void offset_frustum_planes( frustum_t<S> & frust, float offset )
   {
      if (frust.type() == geom::FR_perspective)
      {
         frustum_perspective_t<S> & fr = static_cast<frustum_perspective_t<S> &>(frust);

         point_3f const bisector_dir = normalized(
            normalized(fr.world_space_frustum_vector(geom::FR_left_bottom_near )) +
            normalized(fr.world_space_frustum_vector(geom::FR_right_bottom_near)) +
            normalized(fr.world_space_frustum_vector(geom::FR_right_top_near )) +
            normalized(fr.world_space_frustum_vector(geom::FR_left_top_near )));

         float const extrude_offset = offset / sinf(0.5f * geom::grad2rad(geom::min(fr.hfov(), fr.vfov())));

         fr.camera().set_position(fr.camera().position() - extrude_offset * bisector_dir);
         fr.set_clipping_far(fr.clipping_far() + extrude_offset + offset);
         fr.set_clipping_near(fr.clipping_near() + extrude_offset - offset);
      }
      else // if (frust.type == geom::FR_orthographic)
      {
         rectangle_2f field = frust.field_of_view();
         field.inflate(offset);
         frust.set_field_of_view(field);
         frust.set_clipping_near(frust.clipping_near() - offset);
         frust.set_clipping_far(frust.clipping_far() + offset);
      }
   }

}
