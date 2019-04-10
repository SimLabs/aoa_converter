/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/


#pragma once

namespace geom
{
   template<typename> struct aabb_clip_data_t;
   template<typename> struct obb_clip_data_t;
   template<typename> struct sphere_clip_data_t;
   template<typename> struct cylinder_clip_data_t;
   template<typename> struct capsule_cylinder_clip_data_t;
   template<typename> struct ellipsoid_clip_data_t;

   template<typename> struct aabb_full_clip_data_t;
   template<typename> struct obb_full_clip_data_t;
   template<typename> struct sphere_full_clip_data_t;
   template<typename> struct cylinder_full_clip_data_t;
   template<typename> struct capsule_cylinder_full_clip_data_t;
   template<typename> struct ellipsoid_full_clip_data_t;

   template<typename> struct clip_plane_t;

   template<typename> struct frustum_clipper_t;
   template<typename> class shadow_cap_prism_clipper_t;


   typedef  aabb_clip_data_t                    <float>  aabb_clip_data;
   typedef  obb_clip_data_t                     <float>  obb_clip_data;
   typedef  sphere_clip_data_t                  <float>  sphere_clip_data;
   typedef  cylinder_clip_data_t                <float>  cylinder_clip_data;
   typedef  capsule_cylinder_clip_data_t        <float>  capsule_cylinder_clip_data;
   typedef  ellipsoid_clip_data_t               <float>  ellipsoid_clip_data;

   typedef  aabb_full_clip_data_t               <float>  aabb_full_clip_data;
   typedef  obb_full_clip_data_t                <float>  obb_full_clip_data;
   typedef  sphere_full_clip_data_t             <float>  sphere_full_clip_data;
   typedef  cylinder_full_clip_data_t           <float>  cylinder_full_clip_data;
   typedef  capsule_cylinder_full_clip_data_t   <float>  capsule_cylinder_full_clip_data;
   typedef  ellipsoid_full_clip_data_t          <float>  ellipsoid_full_clip_data;

   typedef  clip_plane_t                        <float>  clip_plane;

   typedef  frustum_clipper_t                   <float>  frustum_clipper;
   typedef  shadow_cap_prism_clipper_t          <float>  shadow_cap_prism_clipper;
}
