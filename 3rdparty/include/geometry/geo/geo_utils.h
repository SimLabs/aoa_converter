/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/geo/projection/mercator.h"

#include "geometry/primitives/geo_point.h"
#include "geometry/primitives/polar_point.h"
#include "geometry/primitives/glb_point.h"
#include "geometry/primitives/geo_base.h"
#include "coordinates.h"
#include "karney.h"
#include "vincenty.h"

#include "cpp_utils/inplace_format.h"

namespace geom
{
    inline geo_point_2 geo_direct(geo_point_2 const & p, polar_point_2 const& offset)
    {
        return vincenty::geo_direct(p, offset);
    }

    inline polar_point_2 geo_inverse(geo_point_2 const& a, geo_point_2 const& b)
    {
        double dlon = abs(norm180(a.lon - b.lon));
        double dlat = abs(a.lat - b.lat);

        // Vincenty works faster, but Karney gives much more accurate solution on almost opposite points on Earth
        if (dlon > 90.0 || dlat > 90.0)
        {
            return karney::geo_inverse(a, b);
        }
        else
        {
            return vincenty::geo_inverse(a, b);
        }
    }

    inline double distance( geo_point_2 const& a, geo_point_2 const& b )
    {
        return geo_inverse(a, b).range;
    }

    inline double distance_sqr( geo_point_2 const& a, geo_point_2 const& b )
    {
        return sqr(distance(a, b));
    }

    inline double distance( geo_point_3 const& a, geo_point_3 const& b )
    {
        double range = geo_inverse(a, b).range;
        double dheight = b.height - a.height;
        return sqrt(sqr(range) + sqr(dheight));
    }

    inline double distance_sqr( geo_point_3 const& a, geo_point_3 const& b )
    {
        return sqr(distance(a, b));
    }

    inline double distance2d( geo_point_3 const& a, geo_point_3 const& b )
    {
        return distance(geo_point_2(a), geo_point_2(b));
    }

    inline double distance2d( geo_point_3 const& a, geo_point_2 const& b )
    {
        return distance(geo_point_2(a), b);
    }

    inline double distance2d( geo_point_2 const& a, geo_point_3 const& b )
    {
        return distance(a, geo_point_2(b));
    }

    inline double distance2d( geo_point_2 const& a, geo_point_2 const& b )
    {
        return distance(a, b);
    }

    inline point_2 geo_direction( geo_point_2 const& p, geo_point_2 const& q )
    {
        return point_2(geo_inverse(p, q));
    }

    inline geo_point_2 geo_offset( geo_point_2 const& p, polar_point_2 const& offset )
    {
        return geo_direct(p, offset);
    }

    inline polar_point_3 geo_direction(geo_point_3 const& from, geo_point_3 const& to)
    {
        polar_point_2 offset = geo_direction((geo_point_2)from, (geo_point_2)to);
        return polar_point_3(offset.range, offset.course, geom::rad2grad(atan2(to.height - from.height, offset.range)));
    }

    inline polar_point_3 slerp(point_3 n0, point_3 n1, double t, double angle)
    {
        // slerp
        double A = sin((1 - t) * angle) / sin(angle);
        double B = sin(t * angle) / sin(angle);
        return A * n0 + B * n1;
    }


    inline geo_point_2 blend(geo_point_2 const & a, geo_point_2 const & b, double t)
    {
        polar_point_2 offset = geo_inverse(a, b);
        offset.range *= t;
        return geo_direct(a, offset);
    }

    inline geo_point_3 blend(geo_point_3 const & a, geo_point_3 const & b, double t)
    {
        return geo_point_3(blend(geo_point_2(a), geo_point_2(b), t), blend(a.height, b.height, t));
    }


    inline geo_point_3 geo_offset( geo_point_3 const & p, polar_point_3 const& offset )
    {
        return geo_point_3(geo_offset(geo_point_2(p), polar_point_2(offset)), p.height + point_3(offset).z);
    }

    inline geo_point_2 geo_closest_point( geo_point_2 const & l0, geo_point_2 const& l1, geo_point_2 const & p )
    {
        polar_point_2 offset = geo_inverse(l0, l1);
        double dst = offset.range;
        if (dst < 10000)
        {
            point_2 pl0 = geo_base_2(l0)(l0);
            point_2 pl1 = geo_base_2(l0)(l1);
            point_2 pp = geo_base_2(l0)(p);
            point_2 closest = segment_2(pl0, pl1).closest_point(pp);
            return geo_base_2(l0)(closest);
        }
        double eps = 0.1 / dst;

        const double phi = (1 + sqrt(5.)) / 2;
        double l = 0., r = 1.;

        bool bf1 = false, bf2 = false;
        double value;
        while (true)
        {
            double len = r - l;
            if (len < eps)
            {
                break;
            }
            double diff = len / phi;
            double m1 = r - diff;
            double f1 = bf1 ? value : distance(geo_direct(l0, geom::polar_point_2(offset.range * m1, offset.course)), p);
            double m2 = l + diff;
            double f2 = bf2 ? value : distance(geo_direct(l0, geom::polar_point_2(offset.range * m2, offset.course)), p);
            bf1 = bf2 = false;
            if (f1 < f2)
            {
                r = m2;
                bf2 = true;
                value = f1;
            }
            else
            {
                l = m1;
                bf1 = true;
                value = f2;
            }
        }
        return geo_direct(l0, geom::polar_point_2(offset.range * 0.5 * (l + r), offset.course));
    }

    inline bool on_right_side( geo_point_2 const & l0, geo_point_2 const& l1, geo_point_2 const & p )
    {
        polar_point_2 offset = geo_inverse(l0, l1);
        polar_point_2 offsetp = geo_inverse(l0, p);
        return norm360(offsetp.course - offset.course) < 180.;
    }

    inline bool on_left_side( geo_point_2 const & l0, geo_point_2 const& l1, geo_point_2 const & p )
    {
        return !on_right_side(l0, l1, p);
    }

    template<typename point_type>
    inline std::vector<point_type> geo_circle( point_type const & geo_center, double rad, unsigned subd );

    template<>
    inline std::vector<geom::geo_point_2> geo_circle( geom::geo_point_2 const & geo_center, double rad, unsigned subd )
    {
        double const angle_step = 360. / subd ;

        std::vector<geom::geo_point_2> result(subd + 1) ;
        for (size_t j = 0, size = result.size(); j != size; ++j)
            result[j] = geom::geo_offset(geo_center, geom::polar_point_2(rad, angle_step * j)) ;

        return result;
    }

    template<>
    inline std::vector<geom::geo_point_3> geo_circle( geom::geo_point_3 const & geo_center, double rad, unsigned subd )
    {
        double const angle_step = 360. / subd ;

        std::vector<geom::geo_point_3> result(subd + 1) ;
        for (size_t j = 0, size = result.size(); j != size; ++j)
            result[j] = geom::geo_offset(geo_center, geom::polar_point_3(rad, angle_step * j, 0.0)) ;

        return result;
    }


    inline std::string geo_pos_to_string( geom::geo_point_2 const & geo_pos )
    {
        const auto lat_letter = (geo_pos.lat > 0.0) ? 'N' : 'S';
        const double lat_deg = fabs(geo_pos.lat);
        const double lat_min = fmod(lat_deg, 1.0) * 60.0;
        const double lat_sec = fmod(lat_min, 1.0) * 60.0;

        const auto lon_letter = (geo_pos.lon > 0.0) ? 'E' : 'W';
        const double lon_deg = fabs(geo_pos.lon);
        const double lon_min = fmod(lon_deg, 1.0) * 60.0;
        const double lon_sec = fmod(lon_min, 1.0) * 60.0;

        return str(cpp_utils::inplace_format("%c %03d %02d %02d\n%c %03d %02d %02d") %
            lat_letter % int(lat_deg) % int(lat_min) % int(lat_sec) %
            lon_letter % int(lon_deg) % int(lon_min) % int(lon_sec));
    }

    inline std::function<bool (geo_point_2 const &, geo_point_2 const &)> geo_point_2_eq_predicate()
    {
        std::function<bool (geo_point_2 const &, geo_point_2 const &)> f;
        f = boost::bind<bool>((bool (*)(geo_point_2 const &, geo_point_2 const &, double ))&geom::eq, _1, _2, geom::epsilon<double>());
        return f;
    }


}
