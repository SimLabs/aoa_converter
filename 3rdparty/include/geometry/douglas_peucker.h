/*
    * Copyright (C) 2016 SimLabs LLC - All rights reserved.
    * Unauthorized copying of this file or its part is strictly prohibited.
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
/*
                                    INTERFACE DESCRIPTION

    1) R geom::douglas_peucker::reduce(I b, I e, double eps)

        This version uses the predefined distance function available for many types from namespace geom.
        The epsilon is used as a stop criterion.

    2) R geom::douglas_peucker::reduce(I b, I e, D distance)
    
        This is a generic version which accepts a distance function.
     
        The interface of the distance function is as follows:
    
            I dist(I b, I e, state_t& s)

            That is, it accepts a range of iterators and returns an iterator from this range, 
            end meaning that the algorithm must stop, otherwise keep the point pointed to by result, continue recursively.

            The third parameter is the global state of the algorithm.

    Both versions return a container with points of the same type as was fed into them.
*/
namespace geom
{

namespace douglas_peucker{

struct state_t 
{
    int num_points = 2;
};

namespace detail{

// distance along normal from a point to the line connecting two given points
template<class T>
struct distance_from_line;

template<class T, size_t D>
struct distance_from_line<point_t<T, D>>
{
    using point_type  = point_t<T, D>;
    using result_type = double;

    distance_from_line(result_type eps)
        : eps_(eps)
    {
    }

    result_type operator()(const point_type& p)
    {
        return geom::distance(line_, p);
    }

    template<class I>
    I operator()(I b, I e, state_t&)
    {
        // to be able to use *this below
        line_ = {*b, *std::prev(e), line::by_points_tag{}};

        std::vector<result_type> distances;
        distances.reserve(std::distance(b, e));
        std::transform(b, e, std::back_inserter(distances), *this);
        auto max_pos = std::max_element(distances.begin(), distances.end());
        if(*max_pos < eps_ || max_pos == distances.begin() || max_pos == std::prev(distances.end()))
            return e;

        auto r = b;
        std::advance(r, std::distance(distances.begin(), max_pos));
        return r;
    }

private:
    geom::line_t<T, D> line_;
    result_type eps_;
};

template<>
struct distance_from_line<geo_point_2>
{
    using base_type   = distance_from_line<point_2>;
    using point_type  = geo_point_2;
    using result_type = base_type::result_type;

    distance_from_line(result_type eps)
        : impl_(eps)
    {
    }

    template<class I>
    I operator()(I b, I e, state_t& s)
    {
        geo_base_2 geo_base(*b);
        std::vector<typename base_type::point_type> points;
        points.reserve(std::distance(b, e));
        std::transform(b, e, std::back_inserter(points), geo_base);

        auto r = b;
        std::advance(r, std::distance(points.begin(), impl_(points.begin(), points.end(), s)));
        return r;
    }

private:
    base_type    impl_;
};

template<class P>
struct distance_from_end_points
{
    using point_type = P;
    using result_type = double;

    distance_from_end_points(result_type eps)
        : eps_(eps)
    {
    }

    result_type operator()(const point_type& p)
    {
        return min(distance(p, p1_), distance(p, p2_));
    }

    template<class I>
    I operator()(I b, I e, state_t& s)
    {
        p1_ = *b;
        p2_ = *prev(e);

        std::vector<result_type> distances;
        distances.reserve(std::distance(b, e));
        std::transform(b, e, std::back_inserter(distances), *this);
        auto max_pos = std::max_element(distances.begin(), distances.end());
        if(*max_pos < eps_ || max_pos == distances.begin() || max_pos == std::prev(distances.end()))
            return e;

        auto r = b;
        std::advance(r, std::distance(distances.begin(), max_pos));
        return r;
    }
private:
    point_type p1_;
    point_type p2_;
    result_type eps_;
};

template<class P>
struct distance;

// default, e.g. for scalar types, cycle_value
template<>
struct distance<double>
{
    using type = distance_from_end_points < double > ;
};
// geom::points
template<class T, size_t D>
struct distance<point_t<T, D>>
{
    using type = distance_from_line<point_t<T, D>>;
};
// geo points
template<>
struct distance<geo_point_2>
{
    using type = distance_from_line<geo_point_2>;
};

template<class I, class D>
struct impl
{
    using point_type = typename std::iterator_traits<I>::value_type;
    using distance_type = D;
    using result_type = vector<point_type>;

    template<class T>
    impl(T&& d)
        : distance_func_(std::forward<T>(d))
    {}

    result_type operator()(I b, I e)
    {
        do_reduce(b, e);
        return output_result(b, e);
    }

    size_t operator()(I b, I e, I r)
    {
        do_reduce(b, e);
        return output(b, e, r);
    }

private:
    void do_reduce(I b, I e)
    {
        state_t s;
        auto size = std::distance(b, e);
        indices_ = { 0, size_t(size - 1) };
        indices_.reserve(size);
        reduce(b, e, 0, s);
    }
 
    void reduce(I b, I e, size_t b_pos, state_t& s)
    {
        if(std::distance(b, e) < 3) return;
        auto result = distance_func_(b, e, s);
        
        if(result == e) return;
        
        auto dist_to_max = std::distance(b, result);

        indices_.push_back(b_pos + dist_to_max);
        s.num_points++;

        auto middle = b;
        std::advance(middle, dist_to_max);

        reduce(b, ++I(middle), b_pos, s);
        reduce(middle, e, b_pos + dist_to_max, s);
    }

    result_type output_result(I b, I e)
    {
        result_type result;
        result.reserve(indices_.size());
        output(b, e, std::back_inserter(result));
        return result;
    }

    template<class R>
    size_t output(I b, I e, R res)
    {
        std::sort(indices_.begin(), indices_.end());
        size_t index = 0;
        auto index_it = indices_.begin();
        while(b != e)
        {
            if(*index_it == index)
            {
                *res++ = *b;
                ++index_it;
            }
            ++b;
            ++index;
        }
        return indices_.size();
    }
private:
    vector<size_t>                  indices_;
    distance_type                   distance_func_;
};

} // namespace detail

template<class I, class D, class R = typename detail::impl<I, D>::result_type>
R reduce(I b, I e, D&& d) 
{
    detail::impl<I, D> impl_{std::forward<D>(d)};
    return impl_(b, e);
}

template<class I, class D = typename detail::distance<typename std::iterator_traits<I>::value_type>::type>
auto reduce(I b, I e, double eps) 
    -> typename detail::impl<I, D>::result_type
{
    return reduce(b, e, D(eps));
}

template<class I, class D>
size_t reduce_inplace(I b, I e, D&& d)
{
    detail::impl<I, D> impl_{ std::forward<D>(d) };
    return impl_(b, e, b);
}

template<class I, class D = typename detail::distance<typename std::iterator_traits<I>::value_type>::type>
size_t reduce_inplace(I b, I e, double eps)
{
    return reduce_inplace(b, e, D(eps));
}

} // namespace douglas_peucker
} // namespace geom