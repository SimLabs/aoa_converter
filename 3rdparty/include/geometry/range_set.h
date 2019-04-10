/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{

struct range_less
{
    bool operator()(geom::range_2 const &a, geom::range_2 const &b) const
    {
        return geom::lt(a.lo(), b.lo());
    }
};

typedef set<geom::range_2, range_less> range_set;

template<typename T>
struct range_map
{
    typedef T mapped_type;
    typedef std::map<geom::range_2, mapped_type, range_less> map;
};


namespace range_set_traits
{

inline range_2 const &get_key(range_2 const &r)
{
    return r;
}

template<typename A, typename B>
inline range_2 const &get_key(pair<A, B> const &r)
{
    return r.first;
}

} // namespace range_set_traits

template<typename Map>
typename Map::const_iterator get_range(Map const &map, double val)
{
    return get_range(const_cast<Map &>(map), val);
}

template<typename Map>
typename Map::iterator get_range(Map &map, double val)
{
    auto it = map.lower_bound(range_2(val, val));
    if (it == map.begin())
        return map.end();

    --it;

    range_2 const &key = range_set_traits::get_key(*it);
    
    return key.contains(val) ? it : map.end();
}

template<typename Map>
typename Map::iterator insert_range(Map &map, typename Map::value_type const &val)
{
    const range_2 &key = range_set_traits::get_key(val);
    
    // Todo: replace with log(n)
    BOOST_FOREACH(auto const &r, map)
    {
        const range_2 &key2 = range_set_traits::get_key(r);
        Verify(!has_intersection(key, key2));
    }

    const auto insertion = map.insert(val);
    Assert(insertion.second);
    return insertion.first;
}

template<typename Map>
typename Map::iterator insert_range(Map &map, range_2 const &range, typename Map::mapped_type const &val)
{
    return insert_range(map, make_pair(range, val));
}


} // namespace geom
