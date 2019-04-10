/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "matrix_helpers.h"
#include "primitives/matrix.h"

#include "geometry/camera/frustum.h"
#include "geometry/camera/clipping/clipping.h"
#include "geometry/polygon/convex_hull.h"

namespace geom
{
   struct lispsm_tuning_params
   {
      lispsm_tuning_params()
      :  intersection_error_correction(0.01f)
      ,  far_extrude(1.0f)
      ,  n_opt_offset(0.0f)
      ,  n_opt_scale(1.0f)
      {}
      float far_extrude;
      float intersection_error_correction;
      float n_opt_offset;
      float n_opt_scale;
   };

   // returns defining points of intersection between frustum and rectangle
   void frustum_rectangle_intersection(
      frustum_clipper const & clipper,
      rectangle_3f const & box,
      float error_correction,
      std::vector<point_3f> * intersection_points );

   // orthographic matrix
   boost::optional<matrix_4f> ortho_shadow_matrix(
       frustum_f const & frustum,
       rectangle_3f const & receiver_box,
       point_3f const & world_space_light_dir );

   // JLPT - Jackis Layer Projection Transform
   // used for defining good and stable projection matrix for vertical cast vector and horizontal infinite layer
   // used for water, lightmaps, decals, - whatever with vertical projection
   boost::optional<matrix_4f> jlpt_matrix(
       frustum_f const & frustum, float min_farplane,
       range_2f const & layer_z_range );

   // LiSPSM transform matrix
   // if caster_box is empty, receiver_box is used
   // otherwise, intersection of both is used
   boost::optional<matrix_4f> lispsm_matrix(
      frustum_f const & frustum,
      point_3f const & world_space_light_dir, // assumed to be normalized
      rectangle_3f const & receiver_box,
      rectangle_3f const & caster_box = rectangle_3f(),
      lispsm_tuning_params const & params = lispsm_tuning_params() );

   // LiSPSM transform matrix - easy case
   boost::optional<matrix_4f> lispsm_matrix(
       frustum_f const & frustum,
       point_3f const & world_space_light_dir, // assumed to be normalized
       lispsm_tuning_params const & params = lispsm_tuning_params() );

   // LiSPSM transform matrix
   // for infinite plane
   boost::optional<matrix_4f> lispsm_matrix(
      frustum_f const & frustum,
      point_3f const & world_space_light_dir, // assumed to be normalized
      geom::range_2f const & height_range,
      lispsm_tuning_params const & params = lispsm_tuning_params() );
}

using geom::lispsm_tuning_params;

inline
void geom::frustum_rectangle_intersection(
   frustum_clipper const & clipper,
   rectangle_3f const & box,
   float error_correction,
   std::vector<point_3f> * intersection_points )
{
   // Adding points of frustum that lie inside box
   rectangle_3f const inflated_box = rectangle_3f(box).inflate(error_correction);
   for (size_t point_idx = 0; point_idx < 8; ++point_idx)
   {
      point_3f const & p = clipper.frustum_point(point_idx);
      if (inflated_box.contains(p))
         intersection_points->push_back(p);
   }

   // Adding points of box that lie inside frustum
   point_3f const box_corners[8] =
   {
      box.xyz(), box.Xyz(), box.XYz(), box.xYz(),
      box.xyZ(), box.XyZ(), box.XYZ(), box.xYZ(),
   };

   static size_t const box_edges[12][2] =
   {
      {0, 1}, {1, 2}, {2, 3}, {3, 0}, // down plane
      {4, 5}, {5, 6}, {6, 7}, {7, 4}, // up plane
      {0, 4}, {1, 5}, {2, 6}, {3, 7}, // down-to-up
   };

   for (size_t point_idx = 0; point_idx < 8; ++point_idx)
   {
      point_3f const & p = box_corners[point_idx];
      if (clipper.is_visible(p))
         intersection_points->push_back(p);
   }

   // finding intersection points of all edges of frustum and all planes of box
   // testing it by epsilon-inflated box
   clip_plane box_planes[6] =
   {
      clip_plane(point_3f(-1.0f,  0.0f,  0.0f), point_3f(box.x.lo(), box.y.center(), box.z.center()), true),
      clip_plane(point_3f(+1.0f,  0.0f,  0.0f), point_3f(box.x.hi(), box.y.center(), box.z.center()), true),
      clip_plane(point_3f( 0.0f, -1.0f,  0.0f), point_3f(box.x.center(), box.y.lo(), box.z.center()), true),
      clip_plane(point_3f( 0.0f, +1.0f,  0.0f), point_3f(box.x.center(), box.y.hi(), box.z.center()), true),
      clip_plane(point_3f( 0.0f,  0.0f, -1.0f), point_3f(box.x.center(), box.y.center(), box.z.lo()), true),
      clip_plane(point_3f( 0.0f,  0.0f, +1.0f), point_3f(box.x.center(), box.y.center(), box.z.hi()), true),
   };

   static size_t const view_frustum_edges[12][2] =
   {
      {0, 1}, {1, 2}, {2, 3}, {3, 0}, // near plane
      {4, 5}, {5, 6}, {6, 7}, {7, 4}, // far plane
      {0, 4}, {1, 5}, {2, 6}, {3, 7}, // near-to-far
   };

   clip_plane inflated_box_planes[6] =
   {
      clip_plane(point_3f(-1.0f,  0.0f,  0.0f), point_3f(inflated_box.x.lo(), inflated_box.y.center(), inflated_box.z.center()), true),
      clip_plane(point_3f(+1.0f,  0.0f,  0.0f), point_3f(inflated_box.x.hi(), inflated_box.y.center(), inflated_box.z.center()), true),
      clip_plane(point_3f( 0.0f, -1.0f,  0.0f), point_3f(inflated_box.x.center(), inflated_box.y.lo(), inflated_box.z.center()), true),
      clip_plane(point_3f( 0.0f, +1.0f,  0.0f), point_3f(inflated_box.x.center(), inflated_box.y.hi(), inflated_box.z.center()), true),
      clip_plane(point_3f( 0.0f,  0.0f, -1.0f), point_3f(inflated_box.x.center(), inflated_box.y.center(), inflated_box.z.lo()), true),
      clip_plane(point_3f( 0.0f,  0.0f, +1.0f), point_3f(inflated_box.x.center(), inflated_box.y.center(), inflated_box.z.hi()), true),
   };

   frustum_clipper const inflated_clipper = frustum_clipper(clipper);
   for (size_t i = 0; i < 6; i++)
   {
      clip_plane & plane = const_cast<clip_plane &>(inflated_clipper.frustum_clip_plane(i));
      float & d = const_cast<float &>(plane.d());
      d -= error_correction;
   }

   for (size_t pass_idx = 0; pass_idx < 2; pass_idx++)
   {
      clip_plane const (*planes_ptr)[6] = NULL;
      clip_plane const (*inflated_planes_ptr)[6] = NULL;
      size_t const (*edges_ptr)[12][2] = NULL;
      point_3f const (*points_ptr)[8] = NULL;

      if (pass_idx == 0)
      {
         planes_ptr = &box_planes;
         inflated_planes_ptr = &inflated_box_planes;
         edges_ptr = &view_frustum_edges;
         points_ptr = (point_3f const (*)[8])&clipper.frustum_point(0);
      }
      else if (pass_idx == 1)
      {
         planes_ptr = (clip_plane const (*)[6])&clipper.frustum_clip_plane(0);
         inflated_planes_ptr = (clip_plane const (*)[6])&inflated_clipper.frustum_clip_plane(0);
         edges_ptr = &box_edges;
         points_ptr = &box_corners;
      }

      for (size_t edge_idx = 0; edge_idx < 12; edge_idx++)
      {
         // intersecting frustum edge with box planes
         point_3f const
            &p0 = (*points_ptr)[(*edges_ptr)[edge_idx][0]],
            &p1 = (*points_ptr)[(*edges_ptr)[edge_idx][1]];

         unsigned const
            code0 = _make_point_planes_code<float, 6>(*inflated_planes_ptr, p0),
            code1 = _make_point_planes_code<float, 6>(*inflated_planes_ptr, p1);

         // trivial reject case
         if ((code0 & code1) != 0) // fully outside
            continue;
         // trivial accept case
         if ((code0 | code1) == 0) // fully inside (this points have already)
            continue;

         for (size_t plane_idx = 0, plane_mask = 1; plane_idx < 6; plane_idx++, plane_mask <<= 1)
         {
            // check, if need
            if (((code0 ^ code1) & plane_mask) == 0)
               continue;

            clip_plane const &plane = (*planes_ptr)[plane_idx];

            point_3f p;
            if (!has_intersection(plane, segment_3f(p0, p1), &p))
               continue;

            if (pass_idx == 0 && inflated_box.contains(p))
               intersection_points->push_back(p);
            else if (pass_idx == 1 && inflated_clipper.is_visible(p))
               intersection_points->push_back(p);
         }
      }
   }
}

// orthographic matrix
inline boost::optional<matrix_4f> geom::ortho_shadow_matrix(
    frustum_f const & frustum,
    rectangle_3f const & receiver_box,
    point_3f const & world_space_light_dir )
{
    Assert(eq(norm_sqr(world_space_light_dir), 1.0f));

    std::vector<point_3f> intersection_points;
    matrix_4f light_view, light_projection;

    point_3f const view_dir = frustum.camera().dir();
    point_3f const view_pos = frustum.camera().position();

    rectangle_3f box(receiver_box);
    if (box.empty())
    {
        rectangle_3f split_box;
        for (size_t i = 0; i < 8; ++i)
            box |= frustum.world_space_frustum_point((geom::frustum_point_id)i);
    }

    frustum_rectangle_intersection(frustum, box, 0.01f, &intersection_points);
    if (intersection_points.size() <= 2)
        return boost::none;

    point_3f const view_up = frustum.camera().up();

    point_3f const light_space_right(normalized(view_up ^ world_space_light_dir));
    point_3f const light_space_up   (world_space_light_dir ^ light_space_right);
    rectangle_3f light_space_receivers_box;
    for (size_t i = 0; i < intersection_points.size(); ++i)
    {
        point_3f const & p = intersection_points[i];
        point_3f const v(p * light_space_right,
            p * light_space_up,
            p * world_space_light_dir);
        light_space_receivers_box |= v;
    }

    range_2f const depth_range(light_space_receivers_box.z);
    light_view = matrix_camera(depth_range.hi() * world_space_light_dir,
        world_space_light_dir,
        light_space_up,
        light_space_right);

    light_projection = matrix_ortho(
        light_space_receivers_box.x.lo(), light_space_receivers_box.x.hi(),
        light_space_receivers_box.y.lo(), light_space_receivers_box.y.hi(),
        0.0f, depth_range.size());

    return light_projection * light_view;
}


inline boost::optional<matrix_4f> geom::jlpt_matrix(
    frustum_f const & frustum, float min_farplane,
    range_2f const & layer_z_range )
{
    // make big box around viewer planar pos
    rectangle_3f receiver_box(point_3f(frustum.camera().position().x, frustum.camera().position().y, layer_z_range.center()));
    float const far_fr_dist = geom::norm(frustum.camera_space_frustum_point(FR_right_top_far));
    receiver_box.inflate(point_3f(far_fr_dist, far_fr_dist, 0.5f * layer_z_range.size()));

    // get inter points
    std::vector<point_3f> fInterPoints;
    frustum_rectangle_intersection(frustum, receiver_box, 0.01f, &fInterPoints);
    if (fInterPoints.size() < 3)
        return boost::none;

    std::vector<point_3> aInterPoints;
    for (unsigned i = 0; i < fInterPoints.size(); ++i)
        aInterPoints.emplace_back(fInterPoints[i]);

/*
    // planarize points
    std::vector<point_2> aInterPointsPlanar;
    for (unsigned i = 0; i < fInterPoints.size(); ++i)
        aInterPointsPlanar.push_back(fInterPoints[i]);
    // get convex hull
    std::vector<geom::point_2> conPlanarHull;
    point_set_convex_hull_envelope(&aInterPointsPlanar.front(), aInterPointsPlanar.size(), std::back_inserter(conPlanarHull));

    // add points of interest as convex hull vertices on low and high water level
    std::vector<point_3> aInterPoints;
    for (unsigned i = 0; i < conPlanarHull.size(); ++i)
    {
        aInterPoints.emplace_back(geom::point_3f(conPlanarHull[i], layer_z_range.lo()));
        aInterPoints.emplace_back(geom::point_3f(conPlanarHull[i], layer_z_range.hi()));
    }
*/

    // vectors
    const point_3 vViewerPos = frustum.camera().position();
    const point_3 vViewerDir = frustum.camera().dir();
    const point_3 vViewerRight = frustum.camera().right();

    // cast direction is always upward for now
    const point_3 vCastDir(0, 0, 1);

    // get ideal case angular notification
    const double dCosGamma = geom::min(fabs(vViewerDir * vCastDir), 1.0);
    const double dSinGamma = sqrt(1.0 - dCosGamma * dCosGamma);

    geom::matrix_4 mViewMatrix, mProjMatrix;
    // special case - dueling frustums, so orthographic projection needed
    if (eq_zero(dSinGamma, 0.01))
    {
        // special case - orthographic transform
        const geom::point_3 vCastUp = normalized(vCastDir ^ vViewerRight);
        const geom::point_3 vCastSide = normalized(vCastUp ^ vCastDir);
        mViewMatrix = matrix_camera(vViewerPos, vCastDir, vCastUp, vCastSide);

        // get rectangle
        rectangle_3 rcLightViewReceiversBox;
        for (unsigned i = 0; i < aInterPoints.size(); ++i)
            rcLightViewReceiversBox |= mViewMatrix * geom::point_4(aInterPoints[i], 1.0);

        // calc matrices
        mProjMatrix = matrix_ortho(
            rcLightViewReceiversBox.x.lo(), rcLightViewReceiversBox.x.hi(),
            rcLightViewReceiversBox.y.lo(), rcLightViewReceiversBox.y.hi(),
            rcLightViewReceiversBox.z.lo(), rcLightViewReceiversBox.z.hi());
    }
    else
    {
        // usual case - LISPSM-like transform
        const point_3 vCastUp = normalized((vCastDir ^ vViewerDir) ^ vCastDir);
        const point_3 vCastSide = normalized(vCastUp ^ vCastDir);
        mViewMatrix = matrix_camera(vViewerPos, vCastDir, vCastUp, vCastSide);

        // transform points to this space
        rectangle_3 rcAABBLightView;
        for (unsigned i = 0; i < aInterPoints.size(); ++i)
            rcAABBLightView |= mViewMatrix * geom::point_4(aInterPoints[i], 1.0);

        // height over range
        range_2 waveRangeModified = layer_z_range;
        waveRangeModified.inflate(2.5 * layer_z_range.size());
        const double dMaxSignificantHeightOverPlane = geom::max(geom::max(fabs(vViewerPos.z - waveRangeModified.lo()), fabs(vViewerPos.z - waveRangeModified.hi())), waveRangeModified.size());

        // back-offset by depth to match rectangle
        const double m_dNearCam = frustum.clipping_near();
        const double dBackOffset = geom::max(m_dNearCam - rcAABBLightView.y.lo(), 0.0);
        range_2 rangeFwdCorrected = range_2(rcAABBLightView.y.lo() + dBackOffset, rcAABBLightView.y.hi() + dBackOffset);

        // calculate "optimal" projector position
        const double dNopt = dMaxSignificantHeightOverPlane/* * dCosGamma*/ / dSinGamma;

        // get projector position - center and step back from viewer along direction axis
        // add offset because last column is negative dot products
        mViewMatrix(1,3) += dBackOffset + dNopt;

        // calculate frustum rectangle
        rectangle_3 rcFrustum;
        for (unsigned i = 0; i < aInterPoints.size(); ++i)
        {
            const auto v = mViewMatrix * geom::point_4(aInterPoints[i], 1.0);
            Assert(v.y > 0);
            rcFrustum |= point_3(v.x / v.y, v.y, v.z / v.y);
        }

        // deepness into the range
        double dRangeDeepness = geom::clamp(0.0, 2.0 * layer_z_range.size(), 0.0, 1.0)(geom::distance(layer_z_range, vViewerPos.z));
        dRangeDeepness *= dRangeDeepness * (2.0 - dRangeDeepness);
        // change frustum far according to some heuristics
        rcFrustum.y = range_2(rcFrustum.y.lo(), rcFrustum.y.lo() + std::min(rcFrustum.y.size(), lerp01(double(min_farplane), rcFrustum.y.size(), dRangeDeepness)));

        // save projection matrix
        const double
            dNearPlane = geom::max(dNopt, rcFrustum.y.lo()),
            dFarPlane = rcFrustum.y.hi(),
            dRight = rcFrustum.x.hi() * dNearPlane,
            dLeft = rcFrustum.x.lo() * dNearPlane,
            dWidthInv = 1.0 / (dRight - dLeft),
            dTop = rcFrustum.z.hi() * dNearPlane,
            dBottom = rcFrustum.z.lo() * dNearPlane,
            dHeightInv = 1.0 / (dTop - dBottom),
            dFarNearInv = 1.0 / (dFarPlane - dNearPlane);

        double dvProjection[4][4] =
        {
            {dWidthInv * 2.0 * dNearPlane, -dWidthInv * (dLeft + dRight),          0.0,                            0.0},
            {0.0,                          dFarNearInv * (dNearPlane + dFarPlane), 0.0,                            -dFarNearInv * 2.0 * dNearPlane * dFarPlane},
            {0.0,                          dHeightInv * (dTop + dBottom),          -dHeightInv * 2.0 * dNearPlane, 0.0},
            {0.0,                          1.0,                                    0.0,                            0.0},
        };
        mProjMatrix = matrix_4((const double *)dvProjection);
    }

    return geom::matrix_4f(mProjMatrix * mViewMatrix);
}


inline boost::optional<matrix_4f> geom::lispsm_matrix(
   frustum_f const & frustum,
   point_3f const & world_space_light_dir,
   rectangle_3f const & receiver_box,
   rectangle_3f const & caster_box,
   lispsm_tuning_params const & params )
{
   Assert(eq(norm_sqr(world_space_light_dir), 1.0f));

   std::vector<point_3f> intersection_points;
   matrix_4f light_view, light_projection;

   point_3f const view_dir = frustum.camera().dir();
   point_3f const view_pos = frustum.camera().position();
   float const near_clip = frustum.clipping_near();

   rectangle_3f aabb;

   if (caster_box.empty())
      aabb = receiver_box;
   else
      aabb = receiver_box & caster_box;

   if (aabb.empty())
      return boost::none;

   frustum_rectangle_intersection(frustum, aabb, params.intersection_error_correction, &intersection_points);

   if (intersection_points.size() <= 2)
      return boost::none;

   float const dot_product = min(fabsf(view_dir * -world_space_light_dir), 1.f);
   float const sin_gamma = sqrtf(1.0f - dot_product * dot_product);

   // Special case - dueling frustums, so orthographic projection needed
   if (eq_zero(sin_gamma, 0.01f))
   {
      point_3f const view_up = frustum.camera().up();

      point_3f const light_space_right(normalized(view_up ^ world_space_light_dir));
      point_3f const light_space_up   (world_space_light_dir ^ light_space_right);
      rectangle_3f light_space_receivers_box;
      for (size_t i = 0; i < intersection_points.size(); ++i)
      {
         point_3f const & p = intersection_points[i];
         point_3f const v(p * light_space_right,
            p * light_space_up,
            p * world_space_light_dir);
         light_space_receivers_box |= v;
      }

      range_2f const depth_range(light_space_receivers_box.z);
      light_view = matrix_camera(depth_range.hi() * world_space_light_dir,
         world_space_light_dir,
         light_space_up,
         light_space_right);

      light_projection = matrix_ortho(
         light_space_receivers_box.x.lo(), light_space_receivers_box.x.hi(),
         light_space_receivers_box.y.lo(), light_space_receivers_box.y.hi(),
         0.0f, depth_range.size());

      return light_projection * light_view;
   }

   point_3f const light_side = normalized(view_dir ^ world_space_light_dir);
   point_3f const light_up   = world_space_light_dir ^ light_side;

   // transform points to this space
   transform_4f const light_world_view(matrix_camera(view_pos,
      world_space_light_dir,
      light_up,
      light_side));
   rectangle_3f aabb_light_view;
   for (size_t i = 0; i < intersection_points.size(); ++i)
      aabb_light_view |= light_world_view.treat_point(intersection_points[i]);

   // offset by depth to match rectangle
   float const near_offset = max(near_clip - aabb_light_view.y.lo(), 0.0f);
   aabb_light_view.y.offset(near_offset);

   // calculate "optimal" projector position
   float const near_plane = aabb_light_view.y.lo(), far_plane = aabb_light_view.y.hi() * params.far_extrude;
   float near_opt = near_plane + sqrt(near_plane * (near_plane + (far_plane - near_plane) * sin_gamma));
   near_opt = near_opt / sin_gamma;
   near_opt = near_opt * params.n_opt_scale + params.n_opt_offset; // linear transform for n_opt

   // get projector position
   point_3f const projector_pos = view_pos + light_up * ((near_plane - near_offset) - near_opt);

   // calculate frustum rectangle
   light_view = matrix_camera(projector_pos, world_space_light_dir, light_up, light_side);
   transform_4f const light_view_transform(light_view);
   rectangle_3f light_frustum;
   for (size_t i = 0; i < intersection_points.size(); i++)
   {
      point_3f const v = light_view_transform.treat_point(intersection_points[i]);
      if (v.y > 0.0f)
         light_frustum |= point_3f(v.x / v.y, v.y, v.z / v.y);
   }

   if (light_frustum.empty())
      light_frustum = rectangle_3f(point_3f(), point_3f(1, 1, 1));

   // save projection matrix
   float const
      f_near  = near_opt,
      f_far   = light_frustum.y.hi(),
      f_right = light_frustum.x.hi() * f_near,
      f_left  = light_frustum.x.lo() * f_near,
      width_inv    = 1.0f / (f_right - f_left),
      f_top        = light_frustum.z.hi() * f_near,
      f_bottom     = light_frustum.z.lo() * f_near,
      height_inv   = 1.0f / (f_top - f_bottom),
      far_near_inv = 1.0f / (f_far - f_near);

   float proj[4][4] =
   {
      {width_inv * 2.0f * f_near,    -width_inv    * (f_left + f_right),      0.0f,                            0.0f},
      {0.0f,                          far_near_inv * (f_near + f_far),        0.0f,                           -far_near_inv * 2.0f * f_near * f_far},
      {0.0f,                          height_inv   * (f_top + f_bottom),     -height_inv * 2.0f * f_near,      0.0f},
      {0.0f,                          1.0f,                                   0.0f,                            0.0f},
   };

   light_projection = matrix_4f((float *)proj);

   return light_projection * light_view;
}

inline boost::optional<matrix_4f> geom::lispsm_matrix(
    frustum_f const & frustum,
    point_3f const & world_space_light_dir, // assumed to be normalized
    lispsm_tuning_params const & params )
{
    rectangle_3f split_box;
    for (size_t i = 0; i < 8; ++i)
        split_box |= frustum.world_space_frustum_point((geom::frustum_point_id)i);

    return geom::lispsm_matrix(frustum, world_space_light_dir, split_box, rectangle_3f(), params);
}


inline boost::optional<matrix_4f> geom::lispsm_matrix(
   frustum_f const & frustum,
   point_3f const & world_space_light_dir, // assumed to be normalized
   geom::range_2f const & height_range,
   lispsm_tuning_params const & params )
{
   point_3f const cam_pos = frustum.camera().position();
   rectangle_3f receiver_box(point_3f(cam_pos.x, cam_pos.y, height_range.center()));
   float const far_fr_dist = geom::norm(frustum.camera_space_frustum_point(FR_right_top_far));
   receiver_box.inflate(point_3f(far_fr_dist, far_fr_dist, 0.5f * height_range.size()));

   return geom::lispsm_matrix(frustum, world_space_light_dir, receiver_box, rectangle_3f(), params);
}

