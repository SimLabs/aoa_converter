/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "clip_data.h"
#include "geometry/primitives/plane.h"


namespace geom
{
   //
   // Clip plane
   //

   template<typename S>
   struct clip_plane_t : plane_t<S>
   {
      // default constructor
      inline clip_plane_t( void )
         : plane_t<S>()
         , aabb_pos_vert_idx_(4)
         , aabb_neg_vert_idx_(3)
         , obb_last_vert_idx_(0)
      {
      }

      // copy constructor
      template<typename S1>
         inline clip_plane_t( clip_plane_t<S1> const& p )
         : plane_t<S>(p)
         , aabb_pos_vert_idx_(p.aabb_pos_vert_idx_)
         , aabb_neg_vert_idx_(p.aabb_neg_vert_idx_)
         , obb_last_vert_idx_(0)
      {
      }

      //
      inline clip_plane_t( point_t<S,3> const& n, S d, bool normalized = false )
         : plane_t<S>(normalized ? n : normalized_safe(n), d / norm(n))
         , obb_last_vert_idx_(0)
      {
         _update_aabb_positive_negative_vertex_indices();
      }

      //
      inline clip_plane_t( point_t<S,3> const& n, point_t<S,3> const& p, bool normalized = false )
         : plane_t<S>(normalized ? n : normalized_safe(n), p)
         , obb_last_vert_idx_(0)
      {
         _update_aabb_positive_negative_vertex_indices();
      }

      // AABB tests caching

      // positive vertex index
      inline size_t const& aabb_pos_vert_idx( void ) const { return aabb_pos_vert_idx_; }

      // negative vertex index
      inline size_t const& aabb_neg_vert_idx( void ) const { return aabb_neg_vert_idx_; }


      // OBB test caching

      // last vertex idx
      inline size_t & obb_last_vert_idx( void ) const { return obb_last_vert_idx_; }


   private:

      // calculate positive-negative vertices
      inline void _update_aabb_positive_negative_vertex_indices( void )
      {
         const point_t<S,3> &n(this->n());
         // positive vertex index
         aabb_pos_vert_idx_ = ((n.x >= 0) << 0) | ((n.y >= 0) << 1) | ((n.z >= 0) << 2);
         // negative vertex index
         aabb_neg_vert_idx_ = 0x7 ^ aabb_pos_vert_idx_;
      }

   private:

      // AABB test caching
      size_t
         aabb_pos_vert_idx_, // near point index
         aabb_neg_vert_idx_; // far point index

      // OBB test caching
      mutable size_t
         obb_last_vert_idx_; // last tested vertex index

      template<typename> friend struct clip_plane_t;
   };
}
