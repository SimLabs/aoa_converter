/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "../frustum.h"

#include "clip_data.h"
#include "clip_plane.h"
#include "clip_test.h"

namespace geom
{
   //
   // Frustum clipper declaration
   //

   template<typename scalar>
   struct frustum_clipper_t
   {
      typedef struct aabb_clip_data_t<scalar> aabb_clip_data_t;
      typedef struct obb_clip_data_t<scalar> obb_clip_data_t;
      typedef struct sphere_clip_data_t<scalar> sphere_clip_data_t;
      typedef struct cylinder_clip_data_t<scalar> cylinder_clip_data_t;
      typedef struct capsule_cylinder_clip_data_t<scalar> capsule_cylinder_clip_data_t;
      typedef struct ellipsoid_clip_data_t<scalar> ellipsoid_clip_data_t;

      typedef struct clip_plane_t<scalar> clip_plane_t;

   public:

       typedef point_t<scalar, 3>     point_3t;
       typedef point_t<scalar, 2>     point_2t;
       typedef point_t<scalar, 4>     point_4t;
       typedef matrix_t<scalar, 4>    matrix_4t;
       typedef transform_t<scalar, 4> transform_4t;
       typedef rectangle_t<scalar, 2> rectangle_2t;
       typedef rectangle_t<scalar, 3> rectangle_3t;
       typedef range_t<scalar>        range_1t;
       typedef segment_t<scalar, 3>   segment_3t;

   public:

      template< typename scalar2 >
      inline frustum_clipper_t( frustum_t<scalar2> const & frustum );

      // constructor from model-view-projection matrix
      inline frustum_clipper_t( matrix_4t const& mvp );

      // constructor from model-view-projection transform
      inline frustum_clipper_t( transform_4t const& mvp_tr );

      // constructor from conical light source parameters
      inline frustum_clipper_t( point_3t const& pos, point_3t const& dir, point_3t const& up, scalar half_angle, scalar near_clip, scalar far_clip );

      // constructor from oriented box
      inline frustum_clipper_t( point_3t const& pos, point_3t const& dir, point_3t const& up, point_3t const& side,
         scalar clip_left, scalar clip_right, scalar clip_bottom, scalar clip_top, scalar clip_near, scalar clip_far );

      // copy constructor
      template<typename other_scalar>
      inline frustum_clipper_t( frustum_clipper_t<other_scalar> const& other );

      // frustum point
      inline point_3t const& frustum_point( size_t idx ) const { return frustum_points_[idx]; }

      inline clip_plane_t const& frustum_clip_plane( size_t idx ) const { return clip_planes_[idx]; }


      // all possible clipping tests
      template<typename clip_data_t>
      inline bool is_visible( clip_data_t const& data ) const;
      template<typename clip_data_t>
      inline bool is_visible( clip_data_t const& data, coherency_info & coherency ) const;
      template<typename clip_data_t>
      inline bool is_visible( clip_data_t const& data, coherency_info & coherency, masking_info & masking ) const;

      template<typename clip_data_t>
      inline bool is_visible_no_mask_update( clip_data_t const& data,
         coherency_info & coherency, masking_info const &masking ) const;

      // segment frustum intersection, false - fully outside, true - t0 and t1 are start and end
      inline bool clip_segment( point_3t const& point0, point_3t const& point1, scalar &t0, scalar &t1 );

      // intersect horizontal plane with frustum
      inline bool horizontal_plane_intersection_bound( scalar plane_z, rectangle_2t &intersection_bound ) const;

      // intersect grid with frustum
      inline bool grid_intersection_bound( range_1t const& range, point_2t const& grid_center,
         point_2t const& grid_step, point_2t const& grid_inflate, rectangle_2i & intersection_bound ) const;

      // intersect grid with rectangle
      inline rectangle_2t rect_intersection_bound( rectangle_3t const& rect ) const;

   private:

      // make point frustum code
      inline unsigned _make_point_frustum_code( point_3t const& point );

   private:

      // frustum edge points
      point_3t frustum_points_[8];
      // clip planes
      clip_plane_t clip_planes_[6];

      template<typename> friend struct frustum_clipper_t;
   };


   //
   // Frustum clipper implementation
   //

   template< typename scalar > template< typename scalar2 >
   inline frustum_clipper_t<scalar>::frustum_clipper_t( frustum_t<scalar2> const & frustum )
   {
      frustum_points_[0] = frustum.world_space_frustum_point(geom::FR_left_bottom_near);
      frustum_points_[1] = frustum.world_space_frustum_point(geom::FR_right_bottom_near);
      frustum_points_[2] = frustum.world_space_frustum_point(geom::FR_right_top_near);
      frustum_points_[3] = frustum.world_space_frustum_point(geom::FR_left_top_near);
      frustum_points_[4] = frustum.world_space_frustum_point(geom::FR_left_bottom_far);
      frustum_points_[5] = frustum.world_space_frustum_point(geom::FR_right_bottom_far);
      frustum_points_[6] = frustum.world_space_frustum_point(geom::FR_right_top_far);
      frustum_points_[7] = frustum.world_space_frustum_point(geom::FR_left_top_far);

      // frustum plane points
      point_3t const frustum_plane_points[6] =
      {
         frustum.camera_space_frustum_point(geom::FR_left_top_near),
         frustum.camera_space_frustum_point(geom::FR_left_top_far),
         frustum.camera_space_frustum_point(geom::FR_left_bottom_near),
         frustum.camera_space_frustum_point(geom::FR_right_bottom_near),
         frustum.camera_space_frustum_point(geom::FR_right_top_near),
         frustum.camera_space_frustum_point(geom::FR_left_top_near)
      };

      // frustum plane normals
      const point_3t frustum_plane_normals[6] =
      {
         frustum.camera_space_face_normal(FR_face_near),
         frustum.camera_space_face_normal(FR_face_far),
         frustum.camera_space_face_normal(FR_face_bottom),
         frustum.camera_space_face_normal(FR_face_right),
         frustum.camera_space_face_normal(FR_face_top),
         frustum.camera_space_face_normal(FR_face_left)
      };

      // convert points and normals to world-space and make 6 clip planes
      transform_4t const camera_to_world(frustum.camera().world_view_transform().inverted());
      for (size_t i = 0; i < 6; i++)
      {
         const point_3t p = camera_to_world * frustum_plane_points[i];
         const point_3t n = camera_to_world * as_normal(frustum_plane_normals[i]);
         clip_planes_[i] = clip_plane(n, p);
      }
   }

   // constructor from model-view-projection matrix
   template<typename scalar>
   inline frustum_clipper_t<scalar>::frustum_clipper_t( matrix_4t const& mvp )
   {
      // left clipping plane
      const point_4t left_p = -(mvp.get_row(3) + mvp.get_row(0));
      const clip_plane_t left_plane(point_3t(left_p), left_p.w);
      // right clipping plane
      const point_4t right_p = -(mvp.get_row(3) - mvp.get_row(0));
      const clip_plane_t right_plane(point_3t(right_p), right_p.w);
      // bottom clipping plane
      const point_4t bottom_p = -(mvp.get_row(3) + mvp.get_row(1));
      const clip_plane_t bottom_plane(point_3t(bottom_p), bottom_p.w);
      // top clipping plane
      const point_4t top_p = -(mvp.get_row(3) - mvp.get_row(1));
      const clip_plane_t top_plane(point_3t(top_p), top_p.w);
      // near clipping plane
      const point_4t near_p = -(mvp.get_row(3) + mvp.get_row(2));
      const clip_plane_t near_plane(point_3t(near_p), near_p.w);
      // far clipping plane
      const point_4t far_p = -(mvp.get_row(3) - mvp.get_row(2));
      const clip_plane_t far_plane(point_3t(far_p), far_p.w);

      clip_planes_[0] = near_plane;
      clip_planes_[1] = far_plane;
      clip_planes_[2] = bottom_plane;
      clip_planes_[3] = right_plane;
      clip_planes_[4] = top_plane;
      clip_planes_[5] = left_plane;

      matrix_4 mvp_inv;
      inverse(matrix_4(mvp), mvp_inv);
      static const point_4t frustum_points[8] =
      {
         point_4t(-1, -1, -1, 1),
         point_4t( 1, -1, -1, 1),
         point_4t( 1,  1, -1, 1),
         point_4t(-1,  1, -1, 1),
         point_4t(-1, -1,  1, 1),
         point_4t( 1, -1,  1, 1),
         point_4t( 1,  1,  1, 1),
         point_4t(-1,  1,  1, 1),
      };

      for (size_t i = 0; i < 8; i++)
      {
         const point_4t p = mvp_inv * frustum_points[i];
         frustum_points_[i] = point_3t(p) * (scalar(1) / p.w);
      }
   }

   // constructor from model-view-projection transform
   template<typename scalar>
   inline frustum_clipper_t<scalar>::frustum_clipper_t( transform_4t const& mvp_tr )
   {
      const matrix_4t &mvp = mvp_tr.direct_matrix();
      // left clipping plane
      const point_4t left_p = -(mvp.get_row(3) + mvp.get_row(0));
      const clip_plane_t left_plane(point_3t(left_p), left_p.w);
      // right clipping plane
      const point_4t right_p = -(mvp.get_row(3) - mvp.get_row(0));
      const clip_plane_t right_plane(point_3t(right_p), right_p.w);
      // bottom clipping plane
      const point_4t bottom_p = -(mvp.get_row(3) + mvp.get_row(1));
      const clip_plane_t bottom_plane(point_3t(bottom_p), bottom_p.w);
      // top clipping plane
      const point_4t top_p = -(mvp.get_row(3) - mvp.get_row(1));
      const clip_plane_t top_plane(point_3t(top_p), top_p.w);
      // near clipping plane
      const point_4t near_p = -(mvp.get_row(3) + mvp.get_row(2));
      const clip_plane_t near_plane(point_3t(near_p), near_p.w);
      // far clipping plane
      const point_4t far_p = -(mvp.get_row(3) - mvp.get_row(2));
      const clip_plane_t far_plane(point_3t(far_p), far_p.w);

      clip_planes_[0] = near_plane;
      clip_planes_[1] = far_plane;
      clip_planes_[2] = bottom_plane;
      clip_planes_[3] = right_plane;
      clip_planes_[4] = top_plane;
      clip_planes_[5] = left_plane;

      const matrix_4t &mvp_inv = mvp_tr.inverse_matrix();
      static const point_4t frustum_points[8] =
      {
         point_4t(-1, -1, -1, 1),
         point_4t( 1, -1, -1, 1),
         point_4t( 1,  1, -1, 1),
         point_4t(-1,  1, -1, 1),
         point_4t(-1, -1,  1, 1),
         point_4t( 1, -1,  1, 1),
         point_4t( 1,  1,  1, 1),
         point_4t(-1,  1,  1, 1),
      };

      for (size_t i = 0; i < 8; i++)
      {
         const point_4t p = mvp_inv * frustum_points[i];
         frustum_points_[i] = point_3t(p) * (scalar(1) / p.w);
      }
   }

   // constructor from conical light source parameters
   template<typename scalar>
   inline frustum_clipper_t<scalar>::frustum_clipper_t( point_3t const& pos, point_3t const& dir, point_3t const& up, scalar half_angle, scalar near_clip, scalar far_clip )
   {
      // get tangent and frustum dimensions
      Assert(half_angle < 90);
      const scalar half_angle_tan = tan(grad2rad(half_angle));
      const scalar nearXY = near_clip * half_angle_tan;
      const scalar farXY = far_clip * half_angle_tan;

      // build frustum matrix
      const transform_4t camera_to_world(camera_matrix_inversed(pos, -dir, up, dir ^ up), geom::ss_unscaled);

      // edge frustum points
      const point_3t frustum_points[8] =
      {
         point_3t(-nearXY, -nearXY, -near_clip),
         point_3t(+nearXY, -nearXY, -near_clip),
         point_3t(+nearXY, +nearXY, -near_clip),
         point_3t(-nearXY, +nearXY, -near_clip),
         point_3t( -farXY,  -farXY,  -far_clip),
         point_3t( +farXY,  -farXY,  -far_clip),
         point_3t( +farXY,  +farXY,  -far_clip),
         point_3t( -farXY,  +farXY,  -far_clip),
      };

      // convert points to world-space
      for (size_t i = 0; i < 8; i++)
         frustum_points_[i] = camera_to_world * frustum_points[i];

      // frustum plane points
      const point_3t frustum_plane_points[6] =
      {
         point_3t(      0,       0, -near_clip),
         point_3t(      0,       0,  -far_clip),
         point_3t(-nearXY, -nearXY, -near_clip),
         point_3t(+nearXY, -nearXY, -near_clip),
         point_3t(+nearXY, +nearXY, -near_clip),
         point_3t(-nearXY, +nearXY, -near_clip),
      };

      // frustum plane normals
      const point_3t frustum_plane_normals[6] =
      {
         point_3t( 0, 0, +1),                               // near
         point_3t( 0, 0, -1),                               // far
         frustum_plane_points[2] ^ frustum_plane_points[3], // bottom
         frustum_plane_points[3] ^ frustum_plane_points[4], // right
         frustum_plane_points[4] ^ frustum_plane_points[5], // top
         frustum_plane_points[5] ^ frustum_plane_points[2], // left
      };

      // convert points and normals to world-space and make 6 clip planes
      for (size_t i = 0; i < 6; i++)
      {
         const point_3t p = camera_to_world * frustum_plane_points[i];
         const point_3t n = camera_to_world * as_normal(frustum_plane_normals[i]);
         clip_planes_[i] = clip_plane(n, p);
      }
   }

   // constructor from oriented box
   template<typename scalar>
   inline frustum_clipper_t<scalar>::frustum_clipper_t( point_3t const& pos, point_3t const& dir, point_3t const& up, point_3t const& side,
      scalar clip_left, scalar clip_right, scalar clip_bottom, scalar clip_top, scalar clip_near, scalar clip_far )
   {
      transform_4t camera_to_world(camera_matrix(pos, dir, up, side),geom::ss_unscaled);
      camera_to_world.invert();

      const point_3t frustum_points[8] =
      {
         point_3t( clip_left, clip_bottom, clip_near),
         point_3t(clip_right, clip_bottom, clip_near),
         point_3t(clip_right,    clip_top, clip_near),
         point_3t( clip_left,    clip_top, clip_near),
         point_3t( clip_left, clip_bottom,  clip_far),
         point_3t(clip_right, clip_bottom,  clip_far),
         point_3t(clip_right,    clip_top,  clip_far),
         point_3t( clip_left,    clip_top,  clip_far),
      };

      // convert points to world-space
      for (size_t i = 0; i < 8; i++)
         frustum_points_[i] = camera_to_world * frustum_points[i];

      static const point_3t frustum_normals[6] =
      {
         point_3t( 0,  0, -1), // near
         point_3t( 0,  0, +1), // far
         point_3t( 0, -1,  0), // bottom
         point_3t(+1,  0,  0), // right
         point_3t( 0, +1,  0), // top
         point_3t(-1,  0,  0), // left
      };

      static const size_t plane_to_point_idx_map[6] = {0, 4, 0, 1, 2, 3};

      // convert points and normals to world-space and make 6 clip planes
      for (size_t i = 0; i < 6; i++)
      {
         const point_3t p = camera_to_world * frustum_points[plane_to_point_idx_map[i]];
         const point_3t n = camera_to_world * as_normal(frustum_normals[i]);
         clip_planes_[i] = clip_plane(n, p);
      }
   }

   // copy constructor
   template<typename scalar>
   template<typename other_scalar>
   frustum_clipper_t<scalar>::frustum_clipper_t( frustum_clipper_t<other_scalar> const& other )
   {
      for (size_t i = 0; i < 8; i++)
         frustum_points_[i] = other.frustum_points_[i];
      for (size_t i = 0; i < 6; i++)
         clip_planes_[i] = other.clip_planes_[i];
   }

   // all possible standard clipping tests
   template<typename scalar>
   template<typename clip_data_t>
   bool frustum_clipper_t<scalar>::is_visible( clip_data_t const& data ) const
   {
      for (size_t plane_idx = 0; plane_idx < 6; plane_idx++)
         if (outside_negative_test(clip_planes_[plane_idx], data))
            return false;
      return true;
   }

   template<typename scalar>
   template<typename clip_data_t>
   bool frustum_clipper_t<scalar>::is_visible( clip_data_t const& data, coherency_info & coherency ) const
   {
      const size_t last_failed_plane_idx = coherency.last_failed_id;

      // explicitly test last failed plane
      if (outside_negative_test(clip_planes_[last_failed_plane_idx], data))
         return false;

      // test all other planes
      for (size_t plane_idx = 0; plane_idx < 6; plane_idx++)
      {
         if (plane_idx != last_failed_plane_idx && outside_negative_test(clip_planes_[plane_idx], data))
         {
            coherency.last_failed_id = plane_idx;
            return false;
         }
      }
      return true;
   }

   template<typename scalar>
   template<typename clip_data_t>
   inline bool frustum_clipper_t<scalar>::is_visible_no_mask_update( clip_data_t const &data,
                                                                     coherency_info &coherency,
                                                                     masking_info const &masking ) const
   {
      const size_t
         last_failed_plane_idx = coherency.last_failed_id,
         last_failed_plane_mask = 1 << last_failed_plane_idx;

      // explicitly test last failed plane, if it is not in mask
      if (last_failed_plane_mask & masking.masked_ids)
      {
         clip_plane_t const& last_failed_clip_plane = clip_planes_[last_failed_plane_idx];
         if (outside_negative_test(last_failed_clip_plane, data))
            return false;
      }

      // test all other planes
      for (size_t plane_idx = 0, plane_mask = 1; plane_idx < 6; plane_idx++, plane_mask <<= 1)
      {
         if (plane_idx != last_failed_plane_idx && (plane_mask & masking.masked_ids))
         {
            clip_plane_t const& cur_clip_plane = clip_planes_[plane_idx];
            if (outside_negative_test(cur_clip_plane, data))
            {
               coherency.last_failed_id = plane_idx;
               return false;
            }
         }
      }
      return true;
   }

   template<typename scalar>
   template<typename clip_data_t>
   bool frustum_clipper_t<scalar>::is_visible( clip_data_t const& data, coherency_info & coherency, masking_info & masking ) const
   {
      const size_t
         last_failed_plane_idx = coherency.last_failed_id,
         last_failed_plane_mask = 1 << last_failed_plane_idx;

      // explicitly test last failed plane, if it is not in mask
      if (last_failed_plane_mask & masking.masked_ids)
      {
         clip_plane_t const& last_failed_clip_plane = clip_planes_[last_failed_plane_idx];
         if (outside_negative_test(last_failed_clip_plane, data))
            return false;
         if (inside_positive_test(last_failed_clip_plane, data))
            masking.masked_ids ^= last_failed_plane_mask;
      }

      // test all other planes
      for (size_t plane_idx = 0, plane_mask = 1; plane_idx < 6; plane_idx++, plane_mask <<= 1)
      {
         if (plane_idx != last_failed_plane_idx && (plane_mask & masking.masked_ids))
         {
            clip_plane_t const& cur_clip_plane = clip_planes_[plane_idx];
            if (outside_negative_test(cur_clip_plane, data))
            {
               coherency.last_failed_id = plane_idx;
               return false;
            }
            if (inside_positive_test(cur_clip_plane, data))
               masking.masked_ids ^= plane_mask;
         }
      }
      return true;
   }

   // segment frustum intersection, false - fully outside, true - t0 and t1 are start and end
   template<typename scalar>
   bool frustum_clipper_t<scalar>::clip_segment( point_3t const& point0, point_3t const& point1, scalar &t0, scalar &t1 )
   {
      // make point codes
      unsigned code0 = _make_point_frustum_code(point0);
      unsigned code1 = _make_point_frustum_code(point1);

      // trivial reject case
      if ((code0 & code1) != 0)
         return false;

      // set default coefs
      t0 = 0;
      t1 = 1;

      // trivial accept case
      if ((code0 | code1) == 0)
         return true;

      // handle all planes
      point_3t p0(point0), p1(point1);
      for (size_t plane_idx = 0, plane_mask = 1; plane_idx < 6; plane_idx++, plane_mask <<= 1)
      {
         // check, if need to transform
         if (((code0 ^ code1) & plane_mask) == 0)
            continue;

         // clip by this plane
         segment_3t s(p0, p1);
         scalar t;
         has_intersection(clip_planes_[plane_idx], s, &t);
         point_3t p = s(t);
         if (code0 & plane_mask)
         {
            // get new segment coef
            t0 = t0 + (t1 - t0) * t;
            // set new point
            p0 = p;
            // recalc point code
            code0 = 0;
            for (size_t other_plane_idx = plane_idx + 1; other_plane_idx < 6; other_plane_idx++)
               code0 |= outside_negative_test(clip_planes_[other_plane_idx], p0) << other_plane_idx;
         }
         else
         {
            // get new segment coef
            has_intersection(clip_planes_[plane_idx], s, &t);
            t1 = t0 + (t1 - t0) * t;
            // set new point
            p1 = p;
            // recalc point code
            code1 = 0;
            for (size_t other_plane_idx = plane_idx + 1; other_plane_idx < 6; other_plane_idx++)
               code1 |= outside_negative_test(clip_planes_[other_plane_idx], p1) << other_plane_idx;
         }
         // trivial reject case
         if ((code0 & code1) != 0)
            return false;
         // trivial accept case
         if ((code0 | code1) == 0)
            return true;
      }
      // it is impossible to be here
      Assert(false);
      return false;
   }

   // Intersect horizontal plane with frustum
   template<typename scalar>
   bool frustum_clipper_t<scalar>::horizontal_plane_intersection_bound( scalar plane_z, rectangle_2t &intersection_bound ) const
   {
      // frustum edges and it's count
      static const size_t frustum_edges[12][2] =
      {
         {0, 4}, {1, 5}, {2, 6}, {3, 7},
         {4, 5}, {5, 6}, {6, 7}, {7, 4},
         {0, 1}, {1, 2}, {2, 3}, {3, 0},
      };
      static const size_t frustum_edges_count = sizeof(frustum_edges) / sizeof(frustum_edges[0]);

      // intersect with plane (z = plane_z)
      for (size_t i = 0; i < frustum_edges_count; i++)
      {
         const point_3t
            &p1 = frustum_points_[frustum_edges[i][0]],
            &p2 = frustum_points_[frustum_edges[i][1]];
         // check for intersection
         if ((p1.z - plane_z) * (p2.z - plane_z) <= 0 && fabs(p2.z - p1.z) >= 0.000001)
         {
            scalar coef = (plane_z - p1.z) / (p2.z - p1.z);
            point_2t intersection_point = point_2t(p1.x + (p2.x - p1.x) * coef, p1.y + (p2.y - p1.y) * coef);
            intersection_bound |= intersection_point;
         }
      }

      return !intersection_bound.empty();
   }

   template<typename scalar>
   rectangle_t<scalar, 2> frustum_clipper_t<scalar>::rect_intersection_bound( rectangle_3t const& rect ) const
   {
      rectangle_2t intersection_bound ;

      // lower plane intersection
      horizontal_plane_intersection_bound(rect.z.lo(), intersection_bound);

      if (rect.z.size() > 0.01)
      {
         // higher plane intersection
         horizontal_plane_intersection_bound(rect.z.hi(), intersection_bound);

         // handle all cases with points inside frustum
         for (size_t i = 0; i < 8; i++)
         {
            const point_3t &p = frustum_points_[i];
            // check for intersection
            if (rect.z.contains(p.z))
               intersection_bound |= point_2t(p);
         }
      }

      intersection_bound &= rect ;

      return intersection_bound ;
   }

   // Intersect grid with frustum
   template<typename scalar>
   bool frustum_clipper_t<scalar>::grid_intersection_bound(
      range_1t  const& range,
      point_2t const& grid_center,
      point_2t const& grid_step,
      point_2t const& grid_inflate,
      rectangle_2i & intersection_bound ) const
   {

      rectangle_2t planes_intersection_bound;

      // lower plane intersection
      horizontal_plane_intersection_bound(range.lo(), planes_intersection_bound);

      if (range.size() > 0.01)
      {
         // higher plane intersection
         horizontal_plane_intersection_bound(range.hi(), planes_intersection_bound);

         // handle all cases with points inside frustum
         for (size_t i = 0; i < 8; i++)
         {
            const point_3t &p = frustum_points_[i];
            // check for intersection
            if (range.contains(p.z))
               planes_intersection_bound |= point_2t(p);
         }
      }

      // check, if no intersections at all
      if (planes_intersection_bound.empty())
         return false;

      // make extrusion offset
      planes_intersection_bound.inflate(grid_inflate);

      // convert to grid indices offsets and exit with success
      planes_intersection_bound -= grid_center;
      planes_intersection_bound /= grid_step;
      intersection_bound = rectangle_2i(
         floor(planes_intersection_bound.lo()),
         floor(planes_intersection_bound.hi()));

      return true;
   }

   // make point frustum code
   template<typename scalar>
   unsigned frustum_clipper_t<scalar>::_make_point_frustum_code( point_3t const& point )
   {
      unsigned code = 0;
      for (size_t plane_idx = 0; plane_idx < 6; plane_idx++)
         code |= outside_negative_test(clip_planes_[plane_idx], point) << plane_idx;
      return code;
   }
}
