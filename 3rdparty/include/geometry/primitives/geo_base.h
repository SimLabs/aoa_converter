/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geo_base_fwd.h"
#include "geometry/geo/coordinates.h"

namespace geom
{
    struct geo_base_2 : geo_point_2
    {
        geo_base_2() {}
        geo_base_2(geo_point_2 const& base)
            : geo_point_2(base)
        {}

        geo_point_2 operator () (const point_2  & pos) const;
        point_2 operator () (const geo_point_2  & pos) const;

        geo_base_2& operator += (point_2 const pos)
        {
            return *this = geo_base_2((*this)(pos));
        }
    };

    struct geo_base_3 : geo_point_3
    {
        geo_base_3() {}
        geo_base_3(geo_point_3 const& base) : geo_point_3(base) {}
        geo_base_3(geo_base_2 const& base, double height) : geo_point_3(base, height) {}


        geo_point_3 operator () (const point_3     & pos) const;
        point_3     operator () (const geo_point_3 & pos) const;
        geo_point_2 operator () (const point_2     & pos) const;
        point_2     operator () (const geo_point_2 & pos) const;

        geo_base_3& operator += (point_3 const pos)
        {
            return *this = geo_base_3((*this)(pos));
        }
    };

    __forceinline geo_point_3 geo_base_3::operator () (const point_3    & pos) const
    {
        return geo_point_3(geo_base_2(*this)(pos), height + pos.z);
    }

    __forceinline point_3 geo_base_3::operator () (const geo_point_3    & pos) const
    {
         return point_3(geo_base_2(*this)(pos), pos.height - height);
    }

    __forceinline  geo_point_2 geo_base_3::operator () (const point_2    & pos) const
    {
        point_3 cartesian_base = geodetic_to_cartesian(*this);
        point_3 cartesian_pos = enu_to_cartesian(*this, cartesian_base, point_3(pos, 0));
        return cartesian_to_geodetic(cartesian_pos);
    }

    __forceinline point_2 geo_base_3::operator () (const geo_point_2    & pos) const
    {
        point_3 cartesian_base = geodetic_to_cartesian(*this);
        point_3 cartesian_pos = geodetic_to_cartesian(geo_point_3(pos, 0));
        return cartesian_to_enu(*this, cartesian_base, cartesian_pos);
    }

    __forceinline geo_point_2 geo_base_2::operator () (const point_2  & pos) const
    {
        return geo_base_3(*this, 0)(pos);
    }

    __forceinline  point_2 geo_base_2::operator () (const geo_point_2  & pos) const
    {
         return geo_base_3(*this, 0)(pos);
    }
}
