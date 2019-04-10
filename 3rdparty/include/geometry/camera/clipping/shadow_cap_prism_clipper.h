/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/polygon/convex_hull.h"

namespace geom
{
   template< typename S >
   class shadow_cap_prism_clipper_t
   {
   public:

       typedef point_t      <S, 2> point_2t;
       typedef point_t      <S, 3> point_3t;

       typedef struct clip_plane_t<S> clip_plane_t;

   public:
      shadow_cap_prism_clipper_t();

      // light_dir points from light source
      shadow_cap_prism_clipper_t( std::vector<point_3t> const & points, point_3t const & light_dir );

      template< typename _S >
      shadow_cap_prism_clipper_t( frustum_t<_S> const & frustum, point_3t const & light_dir );

      size_t plane_masking() const;

      template< typename clip_data_t >
      bool is_visible( clip_data_t const & data ) const;

      template< typename clip_data_t >
      bool is_visible( clip_data_t const & data, size_t & masking ) const;

      template< typename clip_data_t >
      bool is_visible_no_mask_update( const clip_data_t & data, size_t masking ) const;

      shadow_cap_prism_clipper_t & offset( point_3t const & offset );
   private:
      void reset( std::vector<point_3t> const & points, point_3t const & light_dir );
   private:
      // Clipping planes (including caps)
      std::vector<clip_plane_t> clip_planes_;
      size_t                    plane_masking_;
   };

   template< typename S >
   shadow_cap_prism_clipper_t<S>::shadow_cap_prism_clipper_t()
   {
   }

   template< typename S >
   shadow_cap_prism_clipper_t<S>::shadow_cap_prism_clipper_t( std::vector<point_3t> const & points, point_3t const & light_dir )
   {
      reset(points, light_dir);
   }

   template< typename S > template< typename _S >
   shadow_cap_prism_clipper_t<S>::shadow_cap_prism_clipper_t( frustum_t<_S> const & frustum,
      typename shadow_cap_prism_clipper_t<S>::point_3t const & light_dir )
   {
      std::vector<point_3t> points(8);
      for (size_t i = 0; i < 8; i++)
         points[i] = frustum.world_space_frustum_point((geom::frustum_point_id)i);

      reset(points, light_dir);
   }

   template< typename S >
   size_t shadow_cap_prism_clipper_t<S>::plane_masking() const
   {
      return plane_masking_ ;
   }

   template< typename S > template< typename clip_data_t >
   bool shadow_cap_prism_clipper_t<S>::is_visible( clip_data_t const & data ) const
   {
      for (size_t plane_idx = 0; plane_idx < clip_planes_.size(); ++plane_idx)
         if (outside_negative_test(clip_planes_[plane_idx], data))
            return false;
      return true;
   }

   template< typename S > template< typename clip_data_t >
   bool shadow_cap_prism_clipper_t<S>::is_visible( clip_data_t const & data, size_t & masking ) const
   {
      if (masking & plane_masking_)
      {
         for (size_t plane_idx = 0, plane_mask = 1; plane_idx < clip_planes_.size(); plane_idx++, plane_mask <<= 1)
            if (plane_mask & masking)
            {
               if ( outside_negative_test(clip_planes_[plane_idx], data))
                  return false;
               else if (inside_positive_test(clip_planes_[plane_idx], data))
                  masking ^= plane_mask;
            }
      }
      return true;
   }

   template< typename S > template< typename clip_data_t >
   bool shadow_cap_prism_clipper_t<S>::is_visible_no_mask_update( clip_data_t const & data,
                                                                  size_t masking ) const
   {
      if (masking & plane_masking_)
      {
         for (size_t plane_idx = 0, plane_mask = 1; plane_idx < clip_planes_.size(); plane_idx++, plane_mask <<= 1)
            if (plane_mask & masking)
            {
               if (outside_negative_test(clip_planes_[plane_idx], data))
                  return false;
            }
      }
      return true;
   }

   template< typename S >
   shadow_cap_prism_clipper_t<S> & shadow_cap_prism_clipper_t<S>::offset( point_3t const & offset )
   {
      for (auto it = clip_planes_.begin(); it != clip_planes_.end(); ++it)
         it->offset(offset);

      return *this;
   }

   template< typename S >
   void shadow_cap_prism_clipper_t<S>::reset( std::vector<point_3t> const & points,
                                              point_3t const & light_dir )
   {
      clip_planes_.clear();
      // lightDir points from the light source

      // Get far point
      S max_dot = std::numeric_limits<S>::min();
      size_t farIdx = -1;
      for (size_t i = 0; i < points.size(); i++)
      {
         float const dot = light_dir * points[i];
         if (dot > max_dot)
         {
            max_dot = dot;
         }
      }

      // Cap clipping plane
      clip_planes_.push_back(clip_plane_t(light_dir, -max_dot, true));

      // Build light space up and right vectors
      point_3t fake_up, up, right;
      if (!eq(light_dir, point_3t(0.0, 0.0,  1.0)) &&
          !eq(light_dir, point_3t(0.0, 0.0, -1.0)))
      {
         fake_up = point_3t(0.0, 0.0, 1.0);
      }
      else
      {
         fake_up = point_3t(1.0, 0.0, 0.0);
      }

      right = normalized_safe(light_dir ^ fake_up);
      up = right ^ light_dir;

      // Project points on right up plane
      std::vector<point_2t> plane_points(points.size());
      for (size_t i = 0; i < points.size(); i++)
      {
         plane_points[i].x = right * points[i];
         plane_points[i].y = up * points[i];
      }

      // Build border planes
      std::vector<int> convex_hull_idx;
      build_convex_hull(plane_points.begin(), plane_points.end(), std::back_inserter(convex_hull_idx));
      Assert(convex_hull_idx.size() >= 3);

      for (size_t i = 0; i < convex_hull_idx.size(); ++i)
      {
         size_t const next_idx = (i == convex_hull_idx.size() - 1) ? 0 : i + 1;

         point_2t const & curP  = plane_points[convex_hull_idx[i]];
         point_2t const & nextP = plane_points[convex_hull_idx[next_idx]];
         point_2t const diff = nextP - curP;
         point_3t const n = geom::normalized_safe(-right * diff.y + up * diff.x);

         clip_planes_.push_back(clip_plane_t(n, curP.x * right + curP.y * up, true));
      }

      plane_masking_ = 0;
      for (size_t plane_idx = 0, plane_mask = 1; plane_idx < clip_planes_.size(); plane_idx++, plane_mask <<= 1)
         plane_masking_ |= plane_mask;
   }
}
