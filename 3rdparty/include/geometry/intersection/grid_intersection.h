/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "geometry/curve.h"
#include "geometry/spatial/grid_2d/grid.h"

namespace geom
{

enum contour_cross_type
{
    top,
    bottom,
    side
};

struct contour_intersection_detail
{
    contour_intersection_detail(size_t sid, double len, bool from_outside, contour_cross_type ct)
        : sid         (sid)
        , len         (len)
        , from_outside(from_outside)
        , ct          (ct)
    {}

    size_t sid ;
    double len ;

    bool from_outside ; // true if sector is intersected from outside and false otherwise
    contour_cross_type ct ;
};

template<class value_type, class _Pr, class _Alloc>
bool contains( set<value_type, _Pr, _Alloc> const& m, value_type v)
{
    return m.find(v) != m.end();
}

template<class traj_type, class contours_type, class area_type>
    struct grid_intersection_3d_processor
{
    grid_intersection_3d_processor(traj_type const &traj, contours_type const &contours)
        : traj_    (traj)
        , contours_(contours)
    {
        Assert(traj_.size() > 0) ;

        auto to_find = std::move(contours_.get_contours(traj_(0).as_2d())) ;
        for (auto it = to_find.begin(), end = to_find.end(); it != end; ++it)
        {
            if (!contours_.want_intersection(*it))
                continue ;

            cur_area_2d_.insert(area_type::value_type(*it));

            geom::range_2 const &cur_sec_height = contours_.get_contour_height_range(*it) ;

            if (cur_sec_height.contains(traj_(0).height))
                cur_area_3d_.insert(area_type::value_type(*it));
        }
        start_area_ = cur_area_3d_ ;

        double len = 0. ;
        double h0 = 0. ;
        double h1 = 0. ;

        for (auto it = traj_.begin(), end = traj_.end(); it != std::prev(end); ++it)
        {
            double seg_len = geom::distance2d(*it, *std::next(it)) ;
            len_range_ = geom::range_2(len, len + seg_len) ;

            h0 = traj_(len).height ;
            h1 = traj_(len + seg_len).height ;

            last_cross_len_ = len_range_.lo() ;
            last_cross_h_   = h0 ;

            len += seg_len ;

            if(!contours_.intersect(*it, *std::next(it), std::ref(*this)))
                intersect_vertically(len_range_, h0, h1) ;
            else if (*last_cross_len_ < len)
                intersect_vertically(geom::range_2(*last_cross_len_, len), *last_cross_h_, h1) ;
        }

        for (auto it = cur_area_2d_.begin(), end = cur_area_2d_.end(); it != end; ++it)
        {
            geom::range_2 const &cur_sec_height = contours_.get_contour_height_range(*it) ;

            if (cur_sec_height.contains(traj_(traj_.length()).height))
                end_area_.insert(*it) ;
        }
    }

public:
    bool operator()(double ratio, size_t contour_id, size_t)
    {
        if (!contours_.want_intersection(contours_type::value_type(contour_id)))
            return true ;

        double cross_len = len_range_(ratio) ;
        double cross_h = traj_(cross_len).height ;

        if (last_cross_len_ && geom::lt(*last_cross_len_, cross_len, 1.))
            intersect_vertically(geom::range_2(*last_cross_len_, cross_len), *last_cross_h_, cross_h) ;

        last_cross_len_ = cross_len ;
        last_cross_h_   = cross_h ;

        // check current sector for side intersection
        geom::range_2 const &cur_sec_height = contours_.get_contour_height_range(contours_type::value_type(contour_id)) ;

        bool now_in_cur_2d = contains(cur_area_2d_, area_type::value_type(contour_id)) ;

        if (cur_sec_height.contains(cross_h))
        {
            contour_intersection_detail sd(contour_id, cross_len, !now_in_cur_2d, side) ;
            sec_sequence_.push_back(sd) ;

            if (sd.from_outside)
                cur_area_3d_.insert(area_type::value_type(sd.sid)) ;
            else
                cur_area_3d_.erase(area_type::value_type(sd.sid)) ;
        }

        if (now_in_cur_2d)
            cur_area_2d_.erase(area_type::value_type(contour_id)) ;
        else // entering new sector in 2d
            cur_area_2d_.insert(area_type::value_type(contour_id)) ;

        return true ;
    }

public:
    std::vector<contour_intersection_detail> const& sequence() const
    {
        return sec_sequence_ ;
    }

    area_type const& start_area() const
    {
        return start_area_ ;
    }

    area_type const &end_area() const
    {
        return end_area_ ;
    }

private:
    // check sectors we are in (in 2d) for vertical intersection
    void intersect_vertically(range_2 const &len, double h0, double h1)
    {
        if (geom::eq(h0, h1))
            return ;

        std::vector<contour_intersection_detail> vertical_intersections ;

        typedef typename geom::curve_t<double>::points_t eq_heights_t ;

        eq_heights_t const &equal_heights = traj_.heights().points() ;
        eq_heights_t::const_iterator len2h = equal_heights.lower_bound(len.lo()) ;

        auto end_len2h = equal_heights.end() ;
        if (len2h != end_len2h && len2h->first < len.hi())
        {
            double prev_len = len.lo() ;
            double prev_h   = h0 ;

            while (len2h != end_len2h)
            {
                if (len2h->first > len.hi())
                {
                    if (!geom::eq(prev_h, h1))
                        for (auto it = cur_area_2d_.begin(), end = cur_area_2d_.end(); it != end; ++it)
                            intersect_sector_vertically(geom::range_2(prev_len, len.hi()), prev_h, h1, unsigned(*it), vertical_intersections) ;

                    break ;
                }
                else
                {
                    if (!geom::eq(prev_h, len2h->second))
                        for (auto it = cur_area_2d_.begin(), end = cur_area_2d_.end(); it != end; ++it)
                            intersect_sector_vertically(geom::range_2(prev_len, len2h->first), prev_h, len2h->second, unsigned(*it), vertical_intersections) ;

                    prev_len = len2h->first ;
                    prev_h   = len2h->second ;

                    ++len2h ;
                }
            }
        }
        else
        {
            for (auto it = cur_area_2d_.begin(), end = cur_area_2d_.end(); it != end; ++it)
                intersect_sector_vertically(len, h0, h1, unsigned(*it), vertical_intersections) ;
        }

        std::sort(vertical_intersections.begin(), vertical_intersections.end(), [](contour_intersection_detail const &d1, contour_intersection_detail const &d2)->bool
        {
            return d1.len < d2.len ;
        }) ;

        for (auto it = vertical_intersections.begin(), end = vertical_intersections.end(); it != end; ++it)
            sec_sequence_.push_back(*it) ;
    }

    void intersect_sector_vertically(geom::range_2 const &seg_len, double seg_h0, double seg_h1, unsigned sid, std::vector<contour_intersection_detail> &out)
    {
        double top_len = -1. ;
        double bottom_len = -1. ;

        geom::range_2 seg_height(seg_h0, seg_h1) ;

        Assert(contours_.want_intersection(sid)) ;
        geom::range_2 const &sec_height =  contours_.get_contour_height_range(sid) ;

        if (seg_height.contains(sec_height.lo()))
            bottom_len = geom::lerp(seg_h0, seg_h1, seg_len.lo(), seg_len.hi())(sec_height.lo()) ;

        if (seg_height.contains(sec_height.hi()))
            top_len = geom::lerp(seg_h0, seg_h1, seg_len.lo(), seg_len.hi())(sec_height.hi()) ;

        bool in_sector = contains(cur_area_3d_, area_type::value_type(sid));

        if (bottom_len < top_len)
        {
            if (geom::ge(bottom_len, 0.))
            {
                Assert(!in_sector) ;
                out.push_back(contour_intersection_detail(sid, bottom_len, true,  bottom)) ;
                out.push_back(contour_intersection_detail(sid, top_len,    false, top)) ;
            }
            else
            {
                out.push_back(contour_intersection_detail(sid, top_len, !in_sector, top)) ;

                if (!in_sector)
                    cur_area_3d_.insert(sid) ;
                else
                    cur_area_3d_.erase(sid) ;
            }
        }
        else if (top_len < bottom_len)
        {
            if (geom::ge(top_len, 0.))
            {
                Assert(!in_sector) ;
                out.push_back(contour_intersection_detail(sid, top_len,    true,  top)) ;
                out.push_back(contour_intersection_detail(sid, bottom_len, false, bottom)) ;
            }
            else
            {
                out.push_back(contour_intersection_detail(sid, bottom_len, !in_sector, bottom)) ;

                if (!in_sector)
                    cur_area_3d_.insert(sid) ;
                else
                    cur_area_3d_.erase(sid) ;
            }
        }
    }

private:
    boost::optional<double> last_cross_len_ ;
    boost::optional<double> last_cross_h_ ;

private:
    traj_type     const &traj_ ;
    contours_type const &contours_ ;

    area_type start_area_ ;
    area_type end_area_ ;

    area_type cur_area_2d_ ;

    area_type cur_area_3d_ ;
private:
    geom::range_2 len_range_ ;

private:
    std::vector<contour_intersection_detail> sec_sequence_ ;
} ;

template<class grid_type, class visitor_type>
    bool grid_intersect2d(geom::geo_point_2 const &p, geom::geo_point_2 const &q, std::function<bool (double, size_t, size_t)> callback, 
                          grid_type const &grid, visitor_type visitor, std::function<std::vector<geo_point_2> (size_t)> get_contour_points)
{
    geo_rect_2 rc = geom::bounding(geom::geo_segment_2(p, q, true));

    if (rc.lon.contains(180.))
    {
        geom::visit(grid, geom::rectangle_2(geom::range_2(rc.lon.lo(), 180.), rc.lat), visitor);
        geom::visit(grid, geom::rectangle_2(geom::range_2(-180., rc.lon.hi()), rc.lat), visitor);
    }
    else
        geom::visit(grid, geom::rectangle_2(geom::range_2(rc.lon.lo(), rc.lon.hi()), rc.lat), visitor);

    geom::geo_segment_2 seg0(p, q, true);

    struct isection_type
    {
        isection_type(double ratio, size_t contour_id, size_t seg_id)
            : ratio(ratio), contour_id(contour_id), seg_id(seg_id)
        {}

        bool operator<(isection_type const& other) const
        {
            return ratio < other.ratio;
        }

        double ratio;
        size_t contour_id;
        size_t seg_id;
    };

    std::vector<isection_type> isections;

    for (auto it = visitor.contours().begin(); it != visitor.contours().end(); ++it)
    {
        Assert(get_contour_points) ;

        auto const& points = get_contour_points(*it);
        for (size_t i = 0; i < points.size(); ++i)
        {
            geo_segment_2 seg1(points[i], points[(i+1)%points.size()], true);

            optional<geo_point_2> isect = geom::intersection(seg0, seg1);

            if (isect)
            {
                double ratio = seg0(*isect);
                isections.push_back(isection_type(ratio, *it, i));

                if (!callback)
                    return true;
            }
        }
    }

    std::sort(isections.begin(), isections.end());

    for (auto it = isections.begin(); it != isections.end(); ++it)
    {
        if (!callback(it->ratio, it->contour_id, it->seg_id))
            return true;
    }

    return !isections.empty() ;
}

} // end of cg
