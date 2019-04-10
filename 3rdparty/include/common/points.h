/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#ifndef CG_PRIMITIVES

#   include "common/primitives_ext.h"

#else 

#   if defined(__GNUG__)
#       pragma GCC diagnostic push
#
#       pragma GCC diagnostic ignored "-fpermissive"
#       pragma GCC diagnostic ignored "-Wparentheses"
#   endif

#   include "geometry/primitives.h"
#   include "geometry/camera/camera_fwd.h"
#   include "geometry/camera/frustum_fwd.h"
#   include "geometry/camera/clipping/clipping_fwd.h"

#   if defined(__GNUG__)
#       pragma GCC diagnostic pop
#   endif

//
using geom::point_2;
using geom::point_2f;
using geom::point_2c;
using geom::point_2s;
using geom::point_2i;
using geom::point_2l;
using geom::point_2b;
using geom::point_2uc;
using geom::point_2us;
using geom::point_2ui;
using geom::point_2ul;

using geom::point_3;
using geom::point_3f;
using geom::point_3c;
using geom::point_3s;
using geom::point_3i;
using geom::point_3l;
using geom::point_3b;
using geom::point_3uc;
using geom::point_3us;
using geom::point_3ui;
using geom::point_3ul;

using geom::point_4;
using geom::point_4f;
using geom::point_4c;
using geom::point_4s;
using geom::point_4i;
using geom::point_4l;
using geom::point_4b;
using geom::point_4uc;
using geom::point_4us;
using geom::point_4ui;
using geom::point_4ul;

//
using geom::polar_point_2 ;
using geom::polar_point_3 ;
using geom::polar_point_2f ;
using geom::polar_point_3f ;

//
using geom::cpr;
using geom::dcpr;
using geom::rot_axis;
using geom::cprf;
using geom::dcprf;
using geom::rot_axisf;

//
using geom::rotation_2;
using geom::rotation_3;
using geom::rotation_2f;
using geom::rotation_3f;

//
using geom::quaternion;
using geom::quaternionf;

//
using geom::matrix_2;
using geom::matrix_3;
using geom::matrix_4;
using geom::matrix_2f;
using geom::matrix_3f;
using geom::matrix_4f;

//
using geom::transform_4 ;
using geom::transform_3 ;
using geom::transform_4f ;
using geom::transform_3f ;

using geom::scale_2 ;
using geom::scale_3 ;
using geom::scale_2f ;
using geom::scale_3f ;

using geom::normal_2 ;
using geom::normal_3 ;
using geom::normal_2f ;
using geom::normal_3f ;

using geom::translation_2 ;
using geom::translation_3 ;
using geom::translation_2f ;
using geom::translation_3f ;

using geom::vector_2 ;
using geom::vector_3 ;
using geom::vector_2f ;
using geom::vector_3f ;

//
using geom::geo_point_2 ;
using geom::geo_point_3 ;
using geom::geo_rect_2 ;
using geom::geo_rect_3 ;
using geom::geo_pos_2 ;
using geom::geo_triangle_2 ;
using geom::geo_segment_2 ;

//
using geom::glb_point_2 ;
using geom::glb_point_3 ;
using geom::glb_rect_2 ;
using geom::glb_rect_3 ;
using geom::glb_pos_2 ;

//
using geom::glb_base_2 ;
using geom::glb_base_3 ;
using geom::geo_base_2 ;
using geom::geo_base_3 ;

//
using geom::range_2f;
using geom::range_2i;
using geom::range_2;
using geom::range_2ui;

using geom::rectangle_2f;
using geom::rectangle_2i;
using geom::rectangle_2ui;
using geom::rectangle_2;
using geom::rectangle_3f;
using geom::rectangle_3i;
using geom::rectangle_3ui;
using geom::rectangle_3;

//
using geom::colorf;
using geom::colorb;
using geom::coloraf;
using geom::colorab;

using geom::line_2;
using geom::line_2f;
using geom::line_3;
using geom::line_3f;

//
using geom::segment_2;
using geom::segment_2f;
using geom::segment_3;
using geom::segment_3f;

//
using geom::triangle_3;
using geom::triangle_3f;
using geom::triangle_2;
using geom::triangle_2f;

//
using geom::sphere_2;
using geom::sphere_2f;
using geom::sphere_3;
using geom::sphere_3f;

//
using geom::plane;
using geom::planef;

//
using geom::arc_2;
using geom::arc_2f;

#endif // CG_PRIMITIVES