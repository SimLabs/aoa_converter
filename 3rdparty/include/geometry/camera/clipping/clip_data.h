/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/


#pragma once


namespace geom
{
   //
   // AABB data
   //

   template<typename scalar>
   struct aabb_clip_data_t
   {
      typedef point_t<scalar, 3> point_3t;
      typedef rectangle_t<scalar, 3> rectangle_3t;

      // default constructor
      inline aabb_clip_data_t( void )
      {
      }

      // copy constructor
      template<typename other_scalar>
      inline aabb_clip_data_t( aabb_clip_data_t<other_scalar> const& other ) :
         center_(other.center_),
         halfsize_(other.halfsize_)
      {
         for (size_t i = 0; i < 8; i++)
            corners_[i] = other.corners_[i];
      }

      //
      inline aabb_clip_data_t( point_3t const& center, point_3t const& halfsize )
      {
         center_ = center;
         halfsize_ = halfsize;
         for (size_t i = 0; i < 8; i++)
         {
            corners_[i] = center_;
            corners_[i].x += ((i & 0x1) ? 1 : -1) * halfsize_.x;
            corners_[i].y += ((i & 0x2) ? 1 : -1) * halfsize_.y;
            corners_[i].z += ((i & 0x4) ? 1 : -1) * halfsize_.z;
         }
      }

      // sleepy:
      // 1. optimized constructor is required
      // 2. is it so faster than simple rectangle_3?
      inline aabb_clip_data_t( rectangle_3t const& rect )
      {
         center_ = rect.center() ;
         halfsize_ = rect.size() / 2;

         for (size_t i = 0; i < 8; i++)
         {
            corners_[i] = center_;
            corners_[i].x += ((i & 0x1) ? 1 : -1) * halfsize_.x;
            corners_[i].y += ((i & 0x2) ? 1 : -1) * halfsize_.y;
            corners_[i].z += ((i & 0x4) ? 1 : -1) * halfsize_.z;
         }
      }

      // center
      inline point_3t const& center( void ) const { return center_; }

      // half size
      inline point_3t const& halfsize( void ) const { return halfsize_; }

      // corner
      inline point_3t const& corner( size_t idx ) const { return corners_[idx]; }

      inline rectangle_3t rect () const { return rectangle_3t(center_ - halfsize_, center_ + halfsize_ ) ;  }


   private:

      point_3t
         center_, // center
         halfsize_, // half size
         corners_[8]; // corners

      template<typename> friend struct aabb_clip_data_t;
   };


   //
   // OBB data
   //

   template<typename scalar>
   struct obb_clip_data_t
   {
      typedef point_t<scalar, 3> point_3t;
      typedef transform_t<scalar, 4> transform_4t;

      // default constructor
      inline obb_clip_data_t( void )
      {
      }

      // copy constructor
      template<typename other_scalar>
      inline obb_clip_data_t( obb_clip_data_t<other_scalar> const& other ) :
         center_(other.center_),
         dir1_(other.dir1_),
         dir2_(other.dir2_),
         dir3_(other.dir3_)
      {
         for (size_t i = 0; i < 8; i++)
            corners_[i] = other.corners_[i];
         // extra data
         if (extra_data_computed_ = other.extra_data_computed_)
         {
            bound_radius_ = (scalar)other.bound_radius_;
            half_sizes_ = other.half_sizes_;
            half_sizes_inv_ = other.half_sizes_inv_;
         }
      }

      //
      inline obb_clip_data_t( point_3t const& center, point_3t const& dir1, point_3t const& dir2, point_3t const& dir3 ) :
         center_(center), dir1_(dir1), dir2_(dir2), dir3_(dir3), extra_data_computed_(false)
      {
         for (size_t i = 0; i < 8; i++)
         {
            corners_[i] = center_;
            corners_[i] += dir1_ * ((i & 0x1) ? 1 : -1);
            corners_[i] += dir2_ * ((i & 0x2) ? 1 : -1);
            corners_[i] += dir3_ * ((i & 0x4) ? 1 : -1);
         }
      }

      //
      inline obb_clip_data_t( transform_4t const & tr, point_3t const & half_size )
          : center_(tr.translation())
          , dir1_(tr.direct_matrix().get_col(0) * half_size.x)
          , dir2_(tr.direct_matrix().get_col(1) * half_size.y)
          , dir3_(tr.direct_matrix().get_col(2) * half_size.z)
          , extra_data_computed_(false)
      {
         for (size_t i = 0; i < 8; i++)
         {
            corners_[i] = center_;
            corners_[i] += dir1_ * ((i & 0x1) ? 1 : -1);
            corners_[i] += dir2_ * ((i & 0x2) ? 1 : -1);
            corners_[i] += dir3_ * ((i & 0x4) ? 1 : -1);
         }
      }

      // center
      inline point_3t const& center( void ) const { return center_; }

      // 1st direction
      inline point_3t const& dir1( void ) const { return dir1_; }

      // 2nd direction
      inline point_3t const& dir2( void ) const { return dir2_; }

      // 3rd direction
      inline point_3t const& dir3( void ) const { return dir3_; }

      // corner
      inline point_3t const& corner( size_t idx ) const { return corners_[idx]; }


      // bound sphere radius
      inline scalar const& bound_radius( void ) const { _compute_extra_data(); return bound_radius_; }

      // distance edge vector
      inline point_3t const& edges_half_lengths( void ) const { _compute_extra_data(); return half_sizes_; }

      // inverse distance edge vector
      inline point_3t const& edges_half_lengths_inv( void ) const { _compute_extra_data(); return half_sizes_inv_; }


   private:

      inline void _compute_extra_data( void ) const
      {
         if (!extra_data_computed_)
         {
            bound_radius_ = geom::distance(center_, corners_[0]);
            half_sizes_.x = geom::norm(dir1_);
            half_sizes_.y = geom::norm(dir2_);
            half_sizes_.z = geom::norm(dir3_);
            half_sizes_inv_.x = (half_sizes_.x > std::numeric_limits<scalar>::epsilon()) ? (1 / half_sizes_.x) : 0;
            half_sizes_inv_.y = (half_sizes_.y > std::numeric_limits<scalar>::epsilon()) ? (1 / half_sizes_.y) : 0;
            half_sizes_inv_.z = (half_sizes_.z > std::numeric_limits<scalar>::epsilon()) ? (1 / half_sizes_.z) : 0;
            extra_data_computed_ = true;
         }
      }


   private:

      point_3t
         center_, // center
         dir1_, // 1st direction half-size length
         dir2_, // 2nd direction half-size length
         dir3_, // 3rd direction half-size length
         corners_[8]; // corner precomputed points


      // extra data

      mutable bool
         extra_data_computed_; // is extra data computed

      mutable scalar
         bound_radius_; // precomputed bound-sphere radius

      mutable point_3t
         half_sizes_, // precomputed edge half distances
         half_sizes_inv_; // precomputed inversed edge half distances

      template<typename> friend struct obb_clip_data_t;
   };


   //
   // Sphere data
   //

   template<typename scalar>
   struct sphere_clip_data_t
   {
      typedef point_t<scalar, 3> point_3t;

      // default constructor
      inline sphere_clip_data_t( void ) :
         radius_(1),
         radius_sqr_(1)
      {
      }

      // copy constructor
      template<typename other_scalar>
      inline sphere_clip_data_t( sphere_clip_data_t<other_scalar> const& other ) :
         center_(other.center_),
         radius_((scalar)other.radius_),
         radius_sqr_((scalar)other.radius_sqr_)
      {
      }

      //
      inline sphere_clip_data_t( point_3t const& center, scalar radius ) :
         center_(center),
         radius_(radius),
         radius_sqr_(radius_ * radius_)
      {
      }

      // center
      inline point_3t const& center( void ) const { return center_; }

      // radius
      inline scalar radius( void ) const { return radius_; }


   private:

      point_3t
         center_; // center

      scalar
         radius_, // radius
         radius_sqr_; // squared radius
   };


   //
   // Cylinder data
   //

   template<typename scalar>
   struct cylinder_clip_data_t
   {
      typedef point_t<scalar, 3> point_3t;

      // default constructor
      inline cylinder_clip_data_t( void ) :
         radius_(1),
         dir_(0, 0, 1)
      {
      }

      // copy constructor
      template<typename other_scalar>
      inline cylinder_clip_data_t( cylinder_clip_data_t<other_scalar> const& other ) :
         pos1_(other.pos1_),
         pos2_(other.pos2_),
         dir_(other.dir_),
         radius_((scalar)other.radius_)
      {
      }

      //
      inline cylinder_clip_data_t( point_3t const& pos1, point_3t const& pos2, scalar radius ) :
         pos1_(pos1),
         pos2_(pos2),
         dir_(normalized_safe(pos2_ - pos1_)),
         radius_(radius)
      {
      }

      // begin point
      inline point_3t const& pos1( void ) const { return pos1_; }

      // end point
      inline point_3t const& pos2( void ) const { return pos2_; }

      // direction
      inline point_3t const& dir( void ) const { return dir_; }

      // radius
      inline scalar radius( void ) const { return radius_; }


   private:

      point_3t
         pos1_, // begin point
         pos2_, // end point
         dir_; // direction

      scalar
         radius_; // radius
   };


   //
   // Capsule cylinder data
   //

   template<typename scalar>
   struct capsule_cylinder_clip_data_t
   {
      typedef point_t<scalar, 3> point_3t;

      // default constructor
      inline capsule_cylinder_clip_data_t( void ) :
         radius_(1)
      {
      }

      // copy constructor
      template<typename other_scalar>
      inline capsule_cylinder_clip_data_t( capsule_cylinder_clip_data_t<other_scalar> const& other ) :
         pos1_(other.pos1_),
         pos2_(other.pos2_),
         radius_((scalar)other.radius_)
      {
      }

      //
      inline capsule_cylinder_clip_data_t( point_3t const& pos1, point_3t const& pos2, scalar radius ) :
         pos1_(pos1),
         pos2_(pos2),
         radius_(radius)
      {
      }

      // begin point
      inline point_3t const& pos1( void ) const { return pos1_; }

      // end point
      inline point_3t const& pos2( void ) const { return pos2_; }

      // radius
      inline scalar radius( void ) const { return radius_; }


   private:

      point_3t
         pos1_, // begin point
         pos2_; // end point

      scalar
         radius_; // radius
   };


   //
   // Ellipsoid data
   //

   template<typename scalar>
   struct ellipsoid_clip_data_t
   {
      typedef point_t<scalar, 3> point_3t;

      // default constructor
      inline ellipsoid_clip_data_t( void ) :
         dir1_(1, 0, 0),
         dir2_(0, 1, 0),
         dir3_(0, 0, 1)
      {
      }

      // copy constructor
      template<typename other_scalar>
      inline ellipsoid_clip_data_t( ellipsoid_clip_data_t<other_scalar> const& other ) :
         center_(other.center_),
         dir1_(other.dir1_),
         dir2_(other.dir2_),
         dir3_(other.dir3_)
      {
      }

      //
      inline ellipsoid_clip_data_t( point_3t const& center, point_3t const& dir1, point_3t const& dir2, point_3t const& dir3 ) :
         center_(center),
         dir1_(dir1),
         dir2_(dir2),
         dir3_(dir3)
      {
      }

      // center
      inline point_3t const& center( void ) const { return center_; }

      // 1st direction
      inline point_3t const& dir1( void ) const { return dir1_; }

      // 2nd direction
      inline point_3t const& dir2( void ) const { return dir2_; }

      // 3rd direction
      inline point_3t const& dir3( void ) const { return dir3_; }


   private:

      point_3t     // mutually perpendicular vectors,
         dir1_,   // parallel to the principal axes of bounded
         dir2_,   // object, those have magnitudes equal
         dir3_,   // to the semi-axis lengths of the ellipsoid
         center_; // center
   };


   //
   // frustum clipper coherency info
   //

   struct coherency_info
   {
      // default constructor
      inline coherency_info( void ) :
         last_failed_id(0)
      {
      }

      explicit coherency_info ( size_t id ) :
         last_failed_id(id)
      {
      }

      __forceinline size_t&        operator * ()       { return last_failed_id ; }
      __forceinline size_t const & operator * () const { return last_failed_id ; }
   private:

      size_t last_failed_id; // last plane failed id

      template<typename> friend struct frustum_clipper_t;
   };


   //
   // frustum clipper masking info
   //

   namespace masking
   {
      struct fully_inside_tag {};

      const fully_inside_tag fully_inside;
   }

   struct masking_info
   {
      // default constructor
      inline masking_info( void ) :
         masked_ids(0x3F)
      {
      }

      // specific constructor
      inline masking_info( masking::fully_inside_tag /*fit*/ ) :
         masked_ids(0x0)
      {
      }

      // reset test mask
      inline void reset_mask( void ) { masked_ids = 0x3F; }

      // ask, if fully inside
      inline bool fully_inside( void ) const { return !masked_ids; }

      inline void reset_mask ( unsigned ids ) { masked_ids |= ids ; }

      __forceinline unsigned &        operator * ()       { return masked_ids ; }
      __forceinline unsigned const & operator * () const { return masked_ids ; }

   private:

      unsigned masked_ids; // masked plane id's

      template<typename> friend struct frustum_clipper_t;
   };


   //
   // full clip data definitions
   //


   template<typename scalar>
   struct aabb_full_clip_data_t : aabb_clip_data_t<scalar>, coherency_info, masking_info
   {
      typedef aabb_clip_data_t<scalar> base_t;
      typedef point_t<scalar, 3> point_3t;

      inline aabb_full_clip_data_t() : base_t(), coherency_info(), masking_info() {}

      template<typename other_scalar>
      inline aabb_full_clip_data_t( aabb_full_clip_data_t<other_scalar> const& other )
          : base_t(other), coherency_info(), masking_info() {}

      inline aabb_full_clip_data_t( point_3t const& center, point_3t const& halfsize)
          : base_t(center, halfsize), coherency_info(), masking_info() {}
   };


   template<typename scalar>
   struct obb_full_clip_data_t : obb_clip_data_t<scalar>, coherency_info, masking_info
   {
      typedef obb_clip_data_t<scalar> base_t;
      typedef point_t<scalar, 3> point_3t;

      inline obb_full_clip_data_t() : base_t(), coherency_info(), masking_info() {}

      template<typename other_scalar>
      inline obb_full_clip_data_t( obb_full_clip_data_t<other_scalar> const& other )
          : base_t(other), coherency_info(), masking_info() {}

      inline obb_full_clip_data_t( point_3t const& center, point_3t const& dir1, point_3t const& dir2, point_3t const& dir3)
          : base_t(center, dir1, dir2, dir3), coherency_info(), masking_info() {}
   };


   template<typename scalar>
   struct sphere_full_clip_data_t : sphere_clip_data_t<scalar>, coherency_info, masking_info
   {
      typedef sphere_clip_data_t<scalar> base_t;
      typedef point_t<scalar, 3> point_3t;

      inline sphere_full_clip_data_t() : base_t(), coherency_info(), masking_info() {}

      template<typename other_scalar>
      inline sphere_full_clip_data_t( sphere_full_clip_data_t<other_scalar> const& other )
          : base_t(other), coherency_info(), masking_info() {}

      inline sphere_full_clip_data_t( point_3t const& center, scalar radius)
          : base_t(center, radius), coherency_info(), masking_info() {}
   };


   template<typename scalar>
   struct cylinder_full_clip_data_t : cylinder_clip_data_t<scalar>, coherency_info, masking_info
   {
      typedef cylinder_clip_data_t<scalar> base_t;
      typedef point_t<scalar, 3> point_3t;

      inline cylinder_full_clip_data_t() : base_t(), coherency_info(), masking_info() {}

      template<typename other_scalar>
      inline cylinder_full_clip_data_t( cylinder_full_clip_data_t<other_scalar> const& other )
          : base_t(other), coherency_info(), masking_info() {}

      inline cylinder_full_clip_data_t( point_3t const& pos1, point_3t const& pos2, scalar radius)
          : base_t(pos1, pos2, radius), coherency_info(), masking_info() {}
   };


   template<typename scalar>
   struct capsule_cylinder_full_clip_data_t : capsule_cylinder_clip_data_t<scalar>, coherency_info, masking_info
   {
      typedef capsule_cylinder_clip_data_t<scalar> base_t;
      typedef point_t<scalar, 3> point_3t;

      inline capsule_cylinder_full_clip_data_t() : base_t(), coherency_info(), masking_info() {}

      template<typename other_scalar>
      inline capsule_cylinder_full_clip_data_t( capsule_cylinder_full_clip_data_t<other_scalar> const& other )
          : base_t(other), coherency_info(), masking_info() {}

      inline capsule_cylinder_full_clip_data_t( point_3t const& pos1, point_3t const& pos2, scalar radius)
          : base_t(pos1, pos2, radius), coherency_info(), masking_info() {}
   };


   template<typename scalar>
   struct ellipsoid_full_clip_data_t : ellipsoid_clip_data_t<scalar>, coherency_info, masking_info
   {
      typedef ellipsoid_clip_data_t<scalar> base_t;
      typedef point_t<scalar, 3> point_3t;

      inline ellipsoid_full_clip_data_t() : base_t(), coherency_info(), masking_info() {}

      template<typename other_scalar>
      inline ellipsoid_full_clip_data_t( ellipsoid_full_clip_data_t<other_scalar> const& other )
          : base_t(other), coherency_info(), masking_info() {}

      inline ellipsoid_full_clip_data_t( point_3t const& center, point_3t const& dir1, point_3t const& dir2, point_3t const& dir3)
          : base_t(center, dir1, dir2, dir3), coherency_info(), masking_info() {}
   };

}
