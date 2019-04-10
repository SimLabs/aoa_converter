/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/primitives/geo_point.h"
#include "geometry/primitives/polar_point.h"
#include "geometry/geo/projection/mercator.h"
#include "geometry/primitives/geo_base.h"

namespace geom
{
    // geodetic <-> cartesian <-> enu

    // see: http://en.wikipedia.org/wiki/Geodetic_datum
    // geodetic  - lat, lon, height (ECEF)
    // cartesian - zero is Earth center, X goes prime meridian, Y to 90 degrees to east
    // enu       - east, north, up

    __forceinline point_3 geodetic_to_cartesian(geo_point_3 const& gp)
    {
        using namespace geom::wgs84;

        double sin_lat = sin(geom::grad2rad(gp.lat));
        double cos_lat = cos(geom::grad2rad(gp.lat));
        double sin_lon = sin(geom::grad2rad(gp.lon));
        double cos_lon = cos(geom::grad2rad(gp.lon));

        double N = Rbig / sqrt(1 - sqr(E) * sqr(sin_lat));

        return point_3
            ((N + gp.height) * cos_lat * cos_lon,
            (N + gp.height) * cos_lat * sin_lon,
            (N * (1 - sqr(E)) + gp.height)  * sin_lat);
    }

    __forceinline geo_point_3 cartesian_to_geodetic(point_3 const& gp)
    {
        using namespace geom::wgs84;

        // see http://www.researchgate.net/publication/227215135_Transformation_from_Cartesian_to_Geodetic_Coordinates_Accelerated_by_Halleys_Method
        double aeps2 = Rbig * Rbig * 1.0e-32;
        double EC2 = 1.0 - sqr(E);
        double ec = sqrt(EC2);
        double b = Rbig * ec;
        double c = Rbig * sqr(E);

        double lon = atan2(gp.y, gp.x);

        double s0 = fabs(gp.z);
        double p2 = sqr(gp.x) + sqr(gp.y);
        double lat = 0.0;
        double h = 0.0;
        if (p2 >= aeps2)
        {
            double p = sqrt(p2);
            double zc = ec * s0;
            double c0 = ec * p;
            double c02 = c0 * c0;
            double c03 = c02 * c0;
            double s02 = s0 * s0;
            double s03 = s02 * s0;
            double a02 = c02 + s02;
            double a0 = sqrt(a02);
            double a03 = a02 * a0;
            double s1 = zc * a03 + c * s03;
            double c1 = p * a03 - c * c03;
            double cs0c0 = c * c0 * s0;
            double b0 = 1.5 * cs0c0 * ((p * s0 - zc * c0) * a0 - cs0c0);
            s1 = s1 * a03 - b0 * s0;
            double cc = ec * (c1 * a03 - b0 * c0);
            lat = atan(s1 / cc);
            double s12 = s1 * s1;
            double cc2 = cc * cc;
            h = (p * cc + s0 * s1 - Rbig * sqrt(EC2 * s12 + cc2)) / sqrt(s12 + cc2);
        }
        else
        {
            lat = pi / 2.0;
            h = s0 - b;
        }
        if (gp.z < 0)
        {
            lat = -lat;
        }

        return geo_point_3(geom::rad2grad(lat), geom::rad2grad(lon), h);
    }

    __forceinline point_3 cartesian_to_enu(geo_point_3 const& gp0, point_3 const& p0, point_3 const& p1)
    {
        double sin_lat = sin(geom::grad2rad(gp0.lat));
        double cos_lat = cos(geom::grad2rad(gp0.lat));
        double sin_lon = sin(geom::grad2rad(gp0.lon));
        double cos_lon = cos(geom::grad2rad(gp0.lon));

        matrix_3 m;
        m.put_row(0, point_3(-sin_lon           ,  cos_lon          , 0      ));
        m.put_row(1, point_3(-sin_lat * cos_lon , -sin_lat * sin_lon, cos_lat));
        m.put_row(2, point_3( cos_lat * cos_lon ,  cos_lat * sin_lon, sin_lat));
        
        return m * (p1 - p0);
    }

    __forceinline point_3 enu_to_cartesian(geo_point_3 const& gp0, point_3 const& cartesian_p, point_3 const& enu_p)
    {
        double sin_lat = sin(geom::grad2rad(gp0.lat));
        double cos_lat = cos(geom::grad2rad(gp0.lat));
        double sin_lon = sin(geom::grad2rad(gp0.lon));
        double cos_lon = cos(geom::grad2rad(gp0.lon));

        matrix_3 m;
        m.put_row(0, point_3(-sin_lon, -sin_lat * cos_lon, cos_lat * cos_lon));
        m.put_row(1, point_3( cos_lon, -sin_lat * sin_lon, cos_lat * sin_lon));
        m.put_row(2, point_3( 0      ,  cos_lat          , sin_lat          ));

        return m * enu_p + cartesian_p;
    }
}
