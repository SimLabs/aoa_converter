/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/geo/geo_utils.h"

#include "alloc/pool_stl.h"


namespace geom
{
namespace details
{
    template<class points_t>
    struct curve_traits
    {
        typedef typename ph_map<double, typename points_t::value_type>::multimap_t curve_t;
    };


    template<class points_t, class dist_f_t>
    typename curve_traits<points_t>::curve_t make_curve_map( points_t const& points, dist_f_t f )
    {
        typename curve_traits<points_t>::curve_t points_map;

        double len = 0;
        for (typename points_t::const_iterator it = points.begin(), end = boost::prior(points.end()); it != end; ++it)
        {
            points_map.insert(std::make_pair(len, *it));
            len += f(*it, *boost::next(it));
        }
        points_map.insert(std::make_pair(len, points.back()));

        return points_map;
    }

struct distance2d_t
{
    inline double operator()( geo_point_2 const& a, geo_point_2 const& b )
    {
        return geom::distance2d(a, b);
    }
};

}

template<typename T>
struct default_curve_lerp
{
    T operator()(const T &a, const T &b, double t) const
    {
        return geom::blend(a, b, t);
    }
};


template<typename T, typename L = default_curve_lerp<T> >
class curve_t
{
public:
    typedef T value_type;
    typedef typename ph_map<double, value_type, std::less<double> >::map_t points_t;
    typedef L lerp_t;

public:
    curve_t()
    { }

    explicit curve_t(const points_t &points)
        : points_(points)
    { }

    explicit curve_t(points_t &&points)
        : points_(std::forward<points_t>(points))
    { }

    value_type operator()(double arg) const
    {
        return value(arg);
    }

    double length() const
    {
        return !points_.empty() ? boost::prior(points_.end())->first : 0;
    }

    value_type value(double arg) const
    {
        const auto its = bounds(arg);

        Assert(its.first != points_.end());

        const auto prev_it = its.first;
        const auto it = its.second;
        
        const auto a = prev_it->first;
        const auto b = it->first;
        Assert(geom::ge(b, a));

        const auto det = b - a;
        const auto t = geom::eq(det, 0.0) ? 0.5 : (arg - a) / det;

        return lerp(prev_it->second, it->second, t);
    }

    std::pair<typename points_t::const_iterator, typename points_t::const_iterator> bounds(double arg) const
    {
        const points_t &m = points_;
        if (m.size() <= 1)
            return std::make_pair(m.begin(), m.begin());

        auto it = m.lower_bound(arg);
        if (it == m.end())
            --it;

        if (it == m.begin())
            ++it;

        const auto prev_it = boost::prior(it);

        return std::make_pair(prev_it, it);
    }

    const points_t &points() const
    {
        return points_;
    }

    points_t &points()
    {
        return points_;
    }

    value_type lerp(const value_type &a, const value_type &b, double t) const
    {
        lerp_t l;
        return l(a, b, t);
    }

    curve_t apply_offset(double offset) const
    {
        curve_t res;
        BOOST_FOREACH(const auto &r, points_)
            res.points_.insert(res.points_.end(), make_pair(r.first + offset, r.second));
    }

    void append(const curve_t &other) 
    {
        append(other, length());
    }

    void append(const curve_t &other, double start_dist) 
    {
        Assert(geom::ge(start_dist, length()));
        BOOST_FOREACH(const auto &p, other.points())
            points().insert(points().end(), make_pair(start_dist + p.first, p.second));
    }

    void remove_equal_points(double tolerance = geom::epsilon<double>())
    {
        if (points().empty())
            return;

        auto src_it = points_.begin();
        for (auto it = std::next(src_it); it != points_.end();)
        {
            if (geom::eq(it->first, src_it->first, tolerance))
            {
                it = points_.erase(it);
            }
            else
            {
                src_it = it;
                ++it;
            }
        }
    }

    vector<value_type> extract_values() const
    {
        vector<value_type> values;
        values.reserve(points_.size());
        BOOST_FOREACH(auto const &r, points_)
            values.push_back(r.second);
        return values;
    }

    REFL_INNER(curve_t)
        REFL_ENTRY(points_)
    REFL_END()

private:
    points_t points_;
};

template<typename T, typename LERP = default_curve_lerp<geo_point_2>, typename DIST = details::distance2d_t>
class geo_curve_2_ext
    : public curve_t<T, LERP>
{
public:
    typedef curve_t<T, LERP> base_type;
    typedef typename base_type::value_type value_type;
    typedef typename base_type::points_t points_t;
    typedef geom::geo_segment_2 segment_t;
    typedef DIST distance_t;

    geo_curve_2_ext() {}

    explicit geo_curve_2_ext(const std::vector<value_type> &points) 
    {
        BOOST_FOREACH(const auto &point, points)
            push_back(point);
    }

    explicit geo_curve_2_ext(const points_t  &points) : base_type(points)                         {}
    explicit geo_curve_2_ext(      points_t &&points) : base_type(std::forward<points_t>(points)) {}

    double closest(geo_point_2 const& pos, boost::optional<double> low_bound = boost::none, boost::optional<double> up_bound = boost::none) const
    {
        if (base_type::points().size() <= 1)
            return 0;

        distance_t distance;

        optional<double> param;
        optional<double> dist;

        auto begin = base_type::points().begin();
        if (low_bound)
        {
            begin = base_type::points().lower_bound(*low_bound);

            if (begin == base_type::points().end())
            {
                --begin;
                return begin->first;
            }

            if (begin != base_type::points().begin())
                --begin;
        }

        const auto end = up_bound ? base_type::points().upper_bound(*up_bound) : base_type::points().end();

        for (auto it = begin; it != end; ++it)
        {
            const auto next_it = boost::next(it);
            if (next_it == base_type::points().end())
                break;

            const segment_t seg(it->second, next_it->second, true);
            const geo_point_2 clst = seg.closest_point(pos);
            const double dst = distance(clst, pos);
            if (!dist || dst < *dist)
            {
                dist = dst;
                param = geom::blend(it->first, next_it->first, seg(clst));
            }
        }
        
        Assert(param);
        return *param;
    }

    double push_back(const value_type &point)
    {
        distance_t distance;
        
        if (base_type::points().empty())
        {
            base_type::points().insert(make_pair(0., point));
            return 0.;
        }
        
        const auto last_it = boost::prior(base_type::points().end());

        if (geom::eq(last_it->second, point))
            return last_it->first;

        const double dist = last_it->first + distance(last_it->second, point);
        base_type::points().insert(base_type::points().end(), make_pair(dist, point));

        return dist;
    }

    double angle(double dist) const
    {
        return course(dist);
    }

    double course(double dist) const
    {
        Verify(!base_type::points().empty());
        if (base_type::points().size() == 1)
            return 0.;
        
        const geo_point_2 pos1 = base_type::value(dist);
        const geo_point_2 pos2 = base_type::value(dist + 1.);

        const geom::polar_point_2 p = geom::geo_direction(pos1, pos2);

        return p.course;
    }

    double length() const
    {
        if (base_type::points().empty())
            return 0.;

        return base_type::points().rbegin()->first;
    }
    
    value_type const &last_point() const
    {
        return base_type::points().rbegin()->second;
    }

    geo_curve_2_ext apply_offset(double offset) const
    {
        return geo_curve_2_ext(base_type::apply_offset(offset));
    }

    std::function<void(const value_type&)> back_inserter()
    {
        return boost::bind(&geo_curve_2_ext::push_back, this, _1);
    }

    bool empty() const
    {
        return points().empty();
    }

    REFL_INNER(geo_curve_2_ext)
        REFL_SER_BIN(points())
    REFL_END()
};

typedef geo_curve_2_ext<geo_point_2, default_curve_lerp<geo_point_2>, details::distance2d_t> geo_curve_2;

} // namespace geom

