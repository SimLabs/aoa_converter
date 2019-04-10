/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/


#pragma once

#include "common/static_transform.h"

#include "clip_data.h"
#include "clip_plane.h"


namespace geom
{
   namespace details
   {
      __forceinline unsigned _get_sign_mask( float const& v1, float const& v2, float const& v3 )
      {
         return
            ((*(unsigned *)&v1 & 0x80000000) >> 31) |
            ((*(unsigned *)&v2 & 0x80000000) >> 30) |
            ((*(unsigned *)&v3 & 0x80000000) >> 29);
      }

      __forceinline unsigned _get_sign_mask( double const& v1, double const& v2, double const& v3 ) // not tested yet
      {
         return
            ((*((unsigned *)&v1 + 1) & 0x80000000) >> 31) |
            ((*((unsigned *)&v2 + 1) & 0x80000000) >> 30) |
            ((*((unsigned *)&v3 + 1) & 0x80000000) >> 29);
      }

      template<typename scalar, size_t size, template<typename,size_t> class predicate>
      struct static_norm_impl_t
      {
         static __forceinline void apply( scalar * data )
         {
            predicate<scalar, size - 1>::apply(data);
            static_norm_impl_t<scalar, size - 1, predicate>::apply(data);
         }
      };

      template<typename scalar, template<typename,size_t> class predicate>
      struct static_norm_impl_t<scalar, 0, predicate>
      {
         static __forceinline void apply( scalar * data )
         {
         }
      };

   }



   // AABB test for fully outside
   template<typename scalar>
   inline bool outside_negative_test( clip_plane_t<scalar> const& plane, aabb_clip_data_t<scalar> const& aabb )
   {
      return plane(aabb.corner(plane.aabb_neg_vert_idx())) > 0;
   }

   // AABB test for fully inside
   template<typename scalar>
   inline bool inside_positive_test( clip_plane_t<scalar> const& plane, aabb_clip_data_t<scalar> const& aabb )
   {
      return plane(aabb.corner(plane.aabb_pos_vert_idx())) < 0;
   }

   // OBB test for fully outside
   template<typename scalar>
   inline bool outside_negative_test( clip_plane_t<scalar> const& plane, obb_clip_data_t<scalar> const& obb )
   {
      scalar const
         res1 = obb.dir1() * plane.n(),
         res2 = obb.dir2() * plane.n(),
         res3 = obb.dir3() * plane.n();

      const unsigned negative_idx = details::_get_sign_mask(res1, res2, res3);

      plane.obb_last_vert_idx() = negative_idx;

      return plane(obb.corner(negative_idx)) > 0;
   }

   // OBB test for fully inside (must be after OutsideNegativeTest)
   template<typename scalar>
   inline bool inside_positive_test( clip_plane_t<scalar> const& plane, obb_clip_data_t<scalar> const& obb )
   {
      return plane(obb.corner(plane.obb_last_vert_idx() ^ 0x7)) < 0;
   }

   // sphere outside test
   template<typename scalar>
   inline bool outside_negative_test( clip_plane_t<scalar> const& plane, sphere_clip_data_t<scalar> const& sphere )
   {
      return plane(sphere.center()) > sphere.radius();
   }

   // sphere inside test
   template<typename scalar>
   inline bool inside_positive_test( clip_plane_t<scalar> const& plane, sphere_clip_data_t<scalar> const& sphere )
   {
      return plane(sphere.center()) < -sphere.radius();
   }

   // cylinder outside test
   template<typename scalar>
   inline bool outside_negative_test( clip_plane_t<scalar> const& plane, cylinder_clip_data_t<scalar> const& cylinder )
   {
      scalar const
         dd = cylinder.dir() * plane.n(),
         dp1 = cylinder.pos1() * plane.n(),
         dp2 = cylinder.pos2() * plane.n();
      scalar
         r_eff_d = 1 - dd * dd;

      util::scalar_clamp_zero(r_eff_d);
      r_eff_d = cylinder.radius() * sqrt(r_eff_d) - plane.d();

      return dp1 > r_eff_d && dp2 > r_eff_d;
   }

   // cylinder inside test
   template<typename scalar>
   inline bool inside_positive_test( clip_plane_t<scalar> const& plane, cylinder_clip_data_t<scalar> const& cylinder )
   {
      scalar const
         dd = cylinder.dir() * plane.n(),
         dp1 = cylinder.pos1() * plane.n(),
         dp2 = cylinder.pos2() * plane.n();
      scalar
         r_eff_d = 1 - dd * dd;

      util::scalar_clamp_zero(r_eff_d);
      r_eff_d = -cylinder.radius() * sqrt(r_eff_d) - plane.d();

      return dp1 < r_eff_d && dp2 < r_eff_d;
   }

   // capsule cylinder outside test
   template<typename scalar>
   inline bool outside_negative_test( clip_plane_t<scalar> const& plane, capsule_cylinder_clip_data_t<scalar> const& capsule_cylinder )
   {
      scalar const
         dp1 = plane(capsule_cylinder.pos1()),
         dp2 = plane(capsule_cylinder.pos2());
      return dp1 > capsule_cylinder.radius() && dp2 > capsule_cylinder.radius();
   }

   // capsule cylinder inside test
   template<typename scalar>
   inline bool inside_positive_test( clip_plane_t<scalar> const& plane, capsule_cylinder_clip_data_t<scalar> const& capsule_cylinder )
   {
      scalar const
         dp1 = plane(capsule_cylinder.pos1()),
         dp2 = plane(capsule_cylinder.pos2());
      return dp1 < -capsule_cylinder.radius() && dp2 < -capsule_cylinder.radius();
   }

   // ellipsoid outside test
   template<typename scalar>
   inline bool outside_negative_test( clip_plane_t<scalar> const& plane, ellipsoid_clip_data_t<scalar> const& ellipsoid )
   {
      scalar const
         dd1 = ellipsoid.dir1() * plane.n(),
         dd2 = ellipsoid.dir2() * plane.n(),
         dd3 = ellipsoid.dir3() * plane.n(),
         r_eff = sqrt(dd1 * dd1 + dd2 * dd2 + dd3 * dd3),
         d_c = plane(ellipsoid.center());
      return d_c > r_eff;
   }

   // capsule cylinder inside test
   template<typename scalar>
   inline bool inside_positive_test( clip_plane_t<scalar> const& plane, ellipsoid_clip_data_t<scalar> const& ellipsoid )
   {
      scalar const
         dd1 = ellipsoid.dir1() * plane.n(),
         dd2 = ellipsoid.dir2() * plane.n(),
         dd3 = ellipsoid.dir3() * plane.n(),
         r_eff = sqrt(dd1 * dd1 + dd2 * dd2 + dd3 * dd3),
         d_c = plane(ellipsoid.center());
      return d_c < -r_eff;
   }

   // point outside test
   template<typename scalar>
   inline bool outside_negative_test( clip_plane_t<scalar> const& plane, point_t<scalar,3> const& point )
   {
      return plane(point) > 0;
   }

   // point inside test
   template<typename scalar>
   inline bool inside_positive_test( clip_plane_t<scalar> const& plane, point_t<scalar,3> const& point )
   {
      return plane(point) < 0;
   }


   // make point frustum code
   template<typename scalar, int N>
   inline unsigned _make_point_planes_code( const clip_plane_t<scalar> planes[N], point_t<scalar,3> const& point )
   {
      unsigned code = 0;
      for (size_t plane_idx = 0; plane_idx < N; plane_idx++)
         code |= outside_negative_test(planes[plane_idx], point) << plane_idx;
      return code;
   }


   //
   // distances
   //

   // get minimal squared distance from point to AABB
   template<typename scalar>
   inline scalar distance_sqr( point_t<scalar,3> const& point, aabb_clip_data_t<scalar> const& aabb )
   {
      point_t<scalar,3> const& halfsize = aabb.halfsize();
      point_t<scalar,3> diff = point - aabb.center();

      // calculate distance
      util::vector_reset_sign_bit(diff);
      diff -= halfsize;
      util::vector_clamp_zero(diff);

      return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
   }

   // get maximal squared distance from point to AABB
   template<typename scalar>
      inline scalar max_distance_sqr( point_t<scalar,3> const& point, aabb_clip_data_t<scalar> const& aabb )
   {
      point_t<scalar,3> const& halfsize = aabb.halfsize();
      point_t<scalar,3> diff = point - aabb.center();

      // calculate distance
      util::vector_reset_sign_bit(diff);
      diff += halfsize;

      return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
   }

   // get minimal and maximal squared distance from point to AABB
   template<typename scalar>
      inline void min_max_distance_sqr( point_t<scalar,3> const& point, aabb_clip_data_t<scalar> const& aabb,
                                        scalar & min_dist_sqr, scalar & max_dist_sqr )
   {
      point_t<scalar,3> const& halfsize = aabb.halfsize();
      point_t<scalar,3> diff = point - aabb.center();

      // calculate distance
      util::vector_reset_sign_bit(diff);

      // calculate max distance
      point_t<scalar,3> diff_max = diff + halfsize;
      max_dist_sqr = diff_max.x * diff_max.x + diff_max.y * diff_max.y + diff_max.z * diff_max.z;

      // calculate min distance
      diff -= halfsize;
      util::vector_clamp_zero(diff);
      min_dist_sqr = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

      // exit
      return;
   }

   // get minimal, centered and maximal squared distance from point to AABB
   template<typename scalar>
      inline void min_center_max_distance_sqr( point_t<scalar,3> const& point, aabb_clip_data_t<scalar> const& aabb,
      scalar & min_dist_sqr, scalar & center_dist_sqr, scalar & max_dist_sqr )
   {
      point_t<scalar,3> const& halfsize = aabb.halfsize();
      point_t<scalar,3> diff = point - aabb.center();
      util::vector_reset_sign_bit(diff);

      // calculate centered distance
      center_dist_sqr = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

      // calculate max distance
      point_t<scalar,3> diff_max = diff + halfsize;
      max_dist_sqr = diff_max.x * diff_max.x + diff_max.y * diff_max.y + diff_max.z * diff_max.z;

      // calculate min distance
      diff -= halfsize;
      util::vector_clamp_zero(diff);
      min_dist_sqr = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

      // exit
      return;
   }

   // get minimal distance from point to AABB
   template<typename scalar>
   inline scalar distance( point_t<scalar,3> const& point, aabb_clip_data_t<scalar> const& aabb )
   {
      return sqrt(distance_sqr(point, aabb));
   }

   // get maximal distance from point to AABB
   template<typename scalar>
      inline scalar max_distance( point_t<scalar,3> const& point, aabb_clip_data_t<scalar> const& aabb )
   {
      return sqrt(max_distance_sqr(point, aabb));
   }


   // get minimal squared distance from point to OBB
   template<typename scalar>
   inline scalar distance_sqr( point_t<scalar,3> const& point, obb_clip_data_t<scalar> const& obb )
   {
      point_t<scalar,3> const&
         halfsize = obb.edges_half_lengths(),
         halfsize_inv = obb.edges_half_lengths_inv();
      point_t<scalar,3>
         diff = point - obb.center(),
         diff_local = point_t<scalar,3>(diff * obb.dir1(), diff * obb.dir2(), diff * obb.dir3()) & halfsize_inv;

      // calculate distance
      util::vector_reset_sign_bit(diff_local);
      diff_local -= halfsize;
      util::vector_clamp_zero(diff_local);

      return diff_local.x * diff_local.x + diff_local.y * diff_local.y + diff_local.z * diff_local.z;
   }

   // get minimal distance from point to OBB
   template<typename scalar>
   inline scalar distance( point_t<scalar,3> const& point, obb_clip_data_t<scalar> const& obb )
   {
      return sqrt(distance_sqr(point, obb));
   }


   // get minimal distance from point to sphere
   template<typename scalar>
   inline scalar distance( point_t<scalar,3> const& point, sphere_clip_data_t<scalar> const& sph )
   {
      return geom::max(scalar(0), geom::distance(point, sph.center()) - sph.radius());
   }

   // get minimal squared distance from point to sphere
   template<typename scalar>
   inline scalar distance_sqr( point_t<scalar,3> const& point, sphere_clip_data_t<scalar> const& sph )
   {
      return geom::sqr(distance(point, sph));
   }
}
