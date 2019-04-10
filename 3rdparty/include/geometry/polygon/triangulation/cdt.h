/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#pragma warning(push)
#pragma warning(disable:4267)
//warning C4267 : 'return' : conversion from 'size_t' to 'int', possible loss of data (because of x64)
#include "poly2tri/poly2tri.h"
#pragma warning(pop)

namespace geom
{
namespace triangulation
{
    struct cdt_error
        : std::runtime_error
    {
        explicit cdt_error(string const &msg)
            : std::runtime_error(msg)
        {}
    };


    template< class Vertices, class OutputIterator >
    void cdt( Vertices const & poly, OutputIterator out )
    {
        struct point_type : p2t::Point
        {
            point_type(){}
            point_type(double x, double y, int idx) : p2t::Point(x, y) , idx (idx) {}
            int idx;
        };

        std::vector<point_type> points(poly.size());
        std::vector<p2t::Point*> polyline(poly.size());

        for (size_t i = 0; i < poly.size(); ++i)
        {
            points[i] = point_type(poly[i].x, poly[i].y, int(i));
            polyline[i] = &points[i];
        }

        p2t::CDT impl(polyline);
        impl.Triangulate();

        std::vector<p2t::Triangle*> triangles = std::move(impl.GetTriangles());

        for (auto it = triangles.begin(); it != triangles.end(); ++it)
        {
            for (size_t i = 0; i < 3; ++i)
            {
                point_type const *point = static_cast<point_type const*>((*it)->GetPoint(int(i)));
                
                auto offset_in_bytes = reinterpret_cast<char const *>(point) - reinterpret_cast<char const *>(&points[0]);
                if (offset_in_bytes % sizeof(point_type) != 0)
                    throw cdt_error("offset_in_bytes is not divisible by point_type size");

                if (offset_in_bytes < 0)
                    throw cdt_error("offset_in_bytes is negative");

                if (size_t(offset_in_bytes) >= points.size() * sizeof(point_type))
                    throw cdt_error("offset_in_bytes is too big");

                *(++out) = point->idx;
            }
        }
    }

}
}
