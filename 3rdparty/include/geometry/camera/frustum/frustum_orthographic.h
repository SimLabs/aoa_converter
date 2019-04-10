/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/primitives.h"
#include "geometry/matrix_helpers.h"

#include "frustum_base.h"

#pragma pack(push, 1)

namespace geom
{
   template< typename S >
   class frustum_orthographic_t : public frustum_t<S>
   {
       typedef frustum_t<S> base_t;

   public:

      frustum_orthographic_t( typename base_t::camera_3t const & cam, rectangle_2f const & field_of_view, range_2f const & near_far );
      frustum_orthographic_t( typename base_t::camera_3t const & cam, rectangle_3f const & frustum_volume );

      frustum_orthographic_t & operator=( frustum_orthographic_t const & other );

      template< typename _S >
      frustum_orthographic_t( frustum_orthographic_t<_S> const & other );

      template< typename _S >
      frustum_orthographic_t & operator=( frustum_orthographic_t<_S> const & other );

      virtual std::unique_ptr<frustum_t<S>> clone() const;
      virtual frustum_type                  type () const;

      virtual void         set_aspect_ratio  ( float aspect_ratio, aspect_ratio_stretch_type stretch_type );
      virtual void         set_near_clip_rect( const rectangle_2f & new_clip_rect );
      virtual rectangle_2f near_clip_rect    () const;

      virtual void set_field_of_view_hor_range( range_2f const & range, bool reset_aspect_ratio );
      virtual void set_field_of_view_ver_range( range_2f const & range, bool reset_aspect_ratio );

   private:

      virtual void reset_optional();

      virtual void validate_projection_matrix             () const;
      virtual void validate_camera_space_frustum_points   () const;
      virtual void validate_camera_space_face_normals     () const;

   private:

      template< typename > friend class frustum_orthographic_t;
   };

   template< typename S >
   frustum_orthographic_t<S>::frustum_orthographic_t( typename base_t::camera_3t const & cam, rectangle_2f const & area,
                                                      range_2f const & near_far )
      : frustum_t<S>(cam, rectangle_3f(area, near_far))
   {
   }

   template< typename S >
   frustum_orthographic_t<S>::frustum_orthographic_t( typename base_t::camera_3t const & cam, rectangle_3f const & frustum_volume )
      : frustum_t<S>(cam, frustum_volume)
   {
   }

   template< typename S >
   frustum_orthographic_t<S> & frustum_orthographic_t<S>::operator=( frustum_orthographic_t<S> const & other )
   {
      frustum_t<S>::operator=(other);

      return *this;
   }

   template< typename S > template< typename _S >
   frustum_orthographic_t<S>::frustum_orthographic_t( frustum_orthographic_t<_S> const & other )
      : frustum_t<S>(other)
   {
   }

   template< typename S > template< typename _S >
   frustum_orthographic_t<S> & frustum_orthographic_t<S>::operator=( frustum_orthographic_t<_S> const & other )
   {
      frustum_t<S>::operator=(other);

      return *this;
   }

   template< typename S >
   std::unique_ptr< frustum_t<S> > frustum_orthographic_t<S>::clone() const
   {
      return std::unique_ptr< frustum_t<S> >(new frustum_orthographic_t<S>(*this));
   }

   template< typename S >
   frustum_type frustum_orthographic_t<S>::type() const
   {
      return FR_orthographic;
   }

   template< typename S >
   void frustum_orthographic_t<S>::set_aspect_ratio( float aspect_ratio, aspect_ratio_stretch_type stretch_type )
   {
      if (stretch_type == FR_stretch_vertical)
      {
         float const vertical_size = base_t::frustum_volume_.size().x / aspect_ratio;
         base_t::frustum_volume_[1] = range_2f(base_t::frustum_volume_[1].center() - vertical_size / 2,
                                       base_t::frustum_volume_[1].center() + vertical_size / 2);
      }
      else if (stretch_type == FR_stretch_horizontal)
      {
         float const horizontal_size = base_t::frustum_volume_.size().y * aspect_ratio;
         base_t::frustum_volume_[0] = range_2f(base_t::frustum_volume_[0].center() - horizontal_size / 2,
                                       base_t::frustum_volume_[0].center() + horizontal_size / 2);
      }
      else
         Assert(false);

      reset_optional();
      base_t::aspect_ratio_ = boost::none;
      ++base_t::version_;
   }

   template< typename S >
   void frustum_orthographic_t<S>::set_near_clip_rect( const rectangle_2f & new_clip_rect )
   {
       base_t::frustum_volume_.x = new_clip_rect.x;
       base_t::frustum_volume_.y = new_clip_rect.y;

       base_t::aspect_ratio_ = boost::none;

       reset_optional();
       ++base_t::version_;
   }

   template< typename S >
   rectangle_2f frustum_orthographic_t<S>::near_clip_rect() const
   {
       return base_t::frustum_volume_;
   }

   template< typename S >
   void frustum_orthographic_t<S>::set_field_of_view_hor_range( range_2f const & hrange,
                                                                bool reset_aspect_ratio )
   {
      float const saved_aspect_ratio = base_t::aspect_ratio();

      base_t::frustum_volume_.x = hrange;

      if (!reset_aspect_ratio)
      {
         float const vsize = hrange.size() / saved_aspect_ratio;
         base_t::frustum_volume_.y = range_2f(base_t::frustum_volume_.y.center()).inflate(vsize / 2);
      }
      else
         base_t::aspect_ratio_ = boost::none;

      reset_optional();
      ++base_t::version_;
   }

   template< typename S >
   void frustum_orthographic_t<S>::set_field_of_view_ver_range( range_2f const & vrange,
                                                                bool reset_aspect_ratio )
   {
      float const saved_aspect_ratio = base_t::aspect_ratio();

      base_t::frustum_volume_.y = vrange;

      if (!reset_aspect_ratio)
      {
         float const hsize = vrange.size() * saved_aspect_ratio;
         base_t::frustum_volume_.x = range_2f(base_t::frustum_volume_.x.center()).inflate(hsize / 2);
      }
      else
         base_t::aspect_ratio_ = boost::none;

      reset_optional();
      ++base_t::version_;
   }

   template< typename S >
   void frustum_orthographic_t<S>::reset_optional()
   {
      base_t::reset_optional_base();
   }


   template< typename S >
   void frustum_orthographic_t<S>::validate_projection_matrix() const
   {
      base_t::projection_matrix_ = matrix_ortho(
         base_t::frustum_volume_.lo().x, base_t::frustum_volume_.hi().x,
         base_t::frustum_volume_.lo().y, base_t::frustum_volume_.hi().y,
         base_t::frustum_volume_.lo().z, base_t::frustum_volume_.hi().z);

      if (base_t::reversed_z_)
      {
         (*base_t::projection_matrix_)(2, 2) = 1.0f / base_t::frustum_volume_.z.size();
         (*base_t::projection_matrix_)(2, 3) = base_t::frustum_volume_.z.hi() / base_t::frustum_volume_.z.size();
      }
   }

   template< typename S >
   void frustum_orthographic_t<S>::validate_camera_space_frustum_points() const
   {
      rectangle_2f const & area = base_t::frustum_volume_;
      float        const   n    = base_t::frustum_volume_.lo().z;
      float        const   f    = base_t::frustum_volume_.hi().z;

      base_t::frustum_points_ = base_t::frustum_points_array();
      typename base_t::frustum_points_array & points = base_t::frustum_points_.get();
      points[FR_left_bottom_near]  = point_3f(area.lo().x, area.lo().y, -n);
      points[FR_right_bottom_near] = point_3f(area.hi().x, area.lo().y, -n);
      points[FR_left_top_near]     = point_3f(area.lo().x, area.hi().y, -n);
      points[FR_right_top_near]    = point_3f(area.hi().x, area.hi().y, -n);
      points[FR_left_bottom_far]   = point_3f(area.lo().x, area.lo().y, -f);
      points[FR_right_bottom_far]  = point_3f(area.hi().x, area.lo().y, -f);
      points[FR_left_top_far]      = point_3f(area.lo().x, area.hi().y, -f);
      points[FR_right_top_far]     = point_3f(area.hi().x, area.hi().y, -f);
   }

   template< typename S >
   void frustum_orthographic_t<S>::validate_camera_space_face_normals() const
   {
      base_t::face_normals_ = base_t::face_normals_array();
      typename base_t::face_normals_array & normals = base_t::face_normals_.get();
      normals[FR_face_left]   = point_3f(-1,  0,  0);
      normals[FR_face_right]  = point_3f( 1,  0,  0);
      normals[FR_face_bottom] = point_3f( 0, -1,  0);
      normals[FR_face_top]    = point_3f( 0,  1,  0);
      normals[FR_face_near]   = point_3f( 0,  0,  1);
      normals[FR_face_far]    = point_3f( 0,  0, -1);
   }

}

#pragma pack(pop)
