#pragma once

#include "matrix_helpers.h"
#include "primitives/matrix.h"

#include "geometry/camera/frustum.h"
#include "geometry/camera/clipping/clipping.h"
#include "geometry/polygon/convex_hull.h"

#include "lispsm_matrix.h"

namespace geom
{

namespace tsm_detail
{
    const float THRESHOLD = 0.0001f;

inline float compare_y(point_3f const &p1, point_3f const &p2)
{
    return p1.y < p2.y;
}

inline float compare_x_div_y(point_3f const &p1, point_3f const &p2)
{
    Assert(geom::gt(p1.y, 0.f));
    Assert(geom::gt(p2.y, 0.f));
    return (p1.x / p1.y) < (p2.x / p2.y);
}

template<typename Arr>
std::pair<float, float> get_xy_ratios(Arr const &moved_points)
{
    auto it1 = std::min_element(moved_points.begin(), moved_points.end(), tsm_detail::compare_x_div_y);
    auto it2 = std::max_element(moved_points.begin(), moved_points.end(), tsm_detail::compare_x_div_y);

    return make_pair(it1->x / it1->y, it2->x / it2->y);
}

inline float compare_z_div_y(point_3f const &p1, point_3f const &p2)
{
    Assert(geom::gt(p1.y, 0.f));
    Assert(geom::gt(p2.y, 0.f));
    return (p1.z / p1.y) < (p2.z / p2.y);
}

template<typename Arr>
std::pair<float, float> get_zy_ratios(Arr const &moved_points)
{
    auto it1 = std::min_element(moved_points.begin(), moved_points.end(), tsm_detail::compare_z_div_y);
    auto it2 = std::max_element(moved_points.begin(), moved_points.end(), tsm_detail::compare_z_div_y);

    return make_pair(it1->z / it1->y, it2->z / it2->y);
}

float get_offset_from_min_y(float min_y, float max_y, float p_y, float xi)
{
    float lambda = max_y - min_y;
    float delta = p_y - min_y;

    float a = lambda * delta * (1.f - xi);
    float b = lambda - 2.f * delta + lambda * xi;

    Assert(!geom::eq(b, 0.f, THRESHOLD));

    float eta = a / b;

    Assert(geom::gt(eta, 0.f));

    return eta;
}


} // namespace tsm_detail

rectangle_3f tsm_world_box(
   frustum_f const & frustum,
   geom::range_2f const & height_range)
{
   point_3f const cam_pos = frustum.camera().position();
   rectangle_3f receiver_box(point_3f(cam_pos.x, cam_pos.y, height_range.center()));
   float const far_fr_dist = geom::norm(frustum.camera_space_frustum_point(FR_right_top_far));
   receiver_box.inflate(point_3f(far_fr_dist, far_fr_dist, 0.5f * height_range.size()));

   return receiver_box;
}

optional<matrix_4f> tsm_matrix(frustum_f const &frustum, point_3f const &world_space_light_dir, rectangle_3f const &aabb)
{
    bool use_intersection = true;
    
    point_3f center1_world = 0.25f *
        ( frustum.world_space_frustum_point(FR_left_bottom_near )
        + frustum.world_space_frustum_point(FR_right_bottom_near)
        + frustum.world_space_frustum_point(FR_left_top_near    )
        + frustum.world_space_frustum_point(FR_right_top_near   ));

    point_3f center2_world = 0.25f *
        ( frustum.world_space_frustum_point(FR_left_bottom_far )
        + frustum.world_space_frustum_point(FR_right_bottom_far)
        + frustum.world_space_frustum_point(FR_left_top_far    )
        + frustum.world_space_frustum_point(FR_right_top_far   ));

    point_3f centers_dir = normalized(center2_world - center1_world);

    float dot_product = min(fabsf(centers_dir * -world_space_light_dir), 1.f);
    float sin_gamma = sqrtf(1.0f - dot_product * dot_product);
   
    if (eq_zero(sin_gamma, 0.01f))
        return ortho_shadow_matrix(frustum, aabb, world_space_light_dir);
        
    point_3f const light_side = normalized(centers_dir ^ world_space_light_dir);
    point_3f const light_up   = world_space_light_dir ^ light_side;

    Assert(geom::gt(centers_dir * light_up, 0.f));
    
    transform_4f const light_world_view(matrix_camera(center1_world, world_space_light_dir, light_up, light_side));

    vector<point_3f> frustum_points_world;
    vector<point_3f> frustum_points;
    vector<point_3f> intersection_points_world;
    vector<point_3f> intersection_points;
    
    frustum_points_world.reserve(FR_total_points);
    for (size_t i = 0; i < FR_total_points; ++i)
        frustum_points_world.push_back(frustum.world_space_frustum_point(frustum_point_id(i)));
    
    if (use_intersection)
        frustum_rectangle_intersection(frustum, aabb, 0.01f, &intersection_points_world);
    else
        intersection_points_world = frustum_points_world;

    if (intersection_points_world.empty())
        return none;

    frustum_points.reserve(frustum_points_world.size());
    for (point_3f const &p : frustum_points_world)
        frustum_points.push_back(light_world_view.treat_point(p));

    intersection_points.reserve(intersection_points_world.size());
    for (point_3f const &p : intersection_points_world)
        intersection_points.push_back(light_world_view.treat_point(p));

    point_3f center1 = light_world_view.treat_point(center1_world);
    point_3f center2 = light_world_view.treat_point(center2_world);

    //float min_y_from_center1 = std::min_element(intersection_points.begin(), intersection_points.end(), tsm_detail::compare_y)->y;
    float max_y_from_center1 = std::max_element(intersection_points.begin(), intersection_points.end(), tsm_detail::compare_y)->y;
    float min_frustum_y_from_center1 = std::min_element(frustum_points.begin(), frustum_points.end(), tsm_detail::compare_y)->y;
    float max_frustum_y_from_center1 = std::max_element(frustum_points.begin(), frustum_points.end(), tsm_detail::compare_y)->y;

    Assert(geom::le(min_frustum_y_from_center1, 0.f, tsm_detail::THRESHOLD));
    Assert(geom::ge(max_frustum_y_from_center1, 0.f, tsm_detail::THRESHOLD));
    Assert(geom::lt(min_frustum_y_from_center1, max_frustum_y_from_center1, tsm_detail::THRESHOLD));

    float poi_y = geom::blend(center1.y, center2.y, 0.4f);
    float offset_from_min_y = tsm_detail::get_offset_from_min_y(min_frustum_y_from_center1, max_frustum_y_from_center1, poi_y, 0.2f);
    float offset_from_center1 = offset_from_min_y - min_frustum_y_from_center1;

    point_3f projector_pos = center1 - offset_from_center1 * point_3f(0.f, 1.f, 0.f);

    vector<point_3f> moved_points;
    
    moved_points.reserve(intersection_points.size());
    for (point_3f const &p : intersection_points)
        moved_points.push_back(p - projector_pos);

    float min_xy_ratio, max_xy_ratio;
    std::tie(min_xy_ratio, max_xy_ratio) = tsm_detail::get_xy_ratios(moved_points);

    float min_zy_ratio, max_zy_ratio;
    std::tie(min_zy_ratio, max_zy_ratio) = tsm_detail::get_zy_ratios(moved_points);

    float f_near       = offset_from_min_y;
    float f_far        = offset_from_center1 + max_y_from_center1;
    float f_right      = max_xy_ratio * f_near;
    float f_left       = min_xy_ratio * f_near;
    float f_top        = max_zy_ratio * f_near;
    float f_bottom     = min_zy_ratio * f_near;
    float width_inv    = 1.0f / (f_right - f_left);
    float height_inv   = 1.0f / (f_top - f_bottom);
    float far_near_inv = 1.0f / (f_far - f_near);

    float proj[4][4] =
    {
       {width_inv * 2.0f * f_near,    -width_inv    * (f_left + f_right),      0.0f,                            0.0f},
       {0.0f,                          far_near_inv * (f_near + f_far),        0.0f,                           -far_near_inv * 2.0f * f_near * f_far},
       {0.0f,                          height_inv   * (f_top + f_bottom),     -height_inv * 2.0f * f_near,      0.0f},
       {0.0f,                          1.0f,                                   0.0f,                            0.0f},
    };

    matrix_4f light_projection((float *)proj);
    point_3f projector_pos_world = light_world_view.treat_point(projector_pos, false);
    
    matrix_4f light_view = matrix_camera(projector_pos_world, world_space_light_dir, light_up, light_side);
    matrix_4f light_view_proj = light_projection * light_view;

    return light_view_proj;
}

} // namespace geom