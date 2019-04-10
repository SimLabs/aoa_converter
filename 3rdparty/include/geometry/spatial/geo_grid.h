/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "alloc/pool_stl.h"
#include "geometry/spatial/grid_2d/grid.h"
#include "geometry/polygon/triangulation/cdt.h"


namespace geometry
{

using boost::scoped_ptr;

enum neighbour_type
{
    nei_right   = 0,
    nei_right_upper,
    nei_upper      ,
    nei_left_upper ,
    nei_left       ,
    nei_left_lower ,
    nei_lower      ,
    nei_right_lower,
    nei_count
};

inline geom::point_2 neighbouring_point(geom::point_2 const& p, geom::point_2 const& unit, neighbour_type nei)
{
    geom::point_2 p2(p);
    switch (nei)
    {
    case nei_right:
        p2.x += unit.x;
        break;
    case nei_right_upper:
        p2.x += unit.x;
        p2.y += unit.y;
        break;
    case nei_upper:
        p2.y += unit.y;
        break;
    case nei_left_upper:
        p2.x -= unit.x;
        p2.y += unit.y;
        break;
    case nei_left:
        p2.x -= unit.x;
        break;
    case nei_left_lower:
        p2.x -= unit.x;
        p2.y -= unit.y;
        break;
    case nei_lower:
        p2.y -= unit.y;
        break;
    case nei_right_lower:
        p2.x += unit.x;
        p2.y -= unit.y;
        break;
    }
    return p2;
}

typedef std::vector<geom::geo_point_2>     geo_points2_t;
typedef std::vector<geom::geo_triangle_2>  triangulation_t;

inline triangulation_t triangulate(geo_points2_t const& poly)
{
    std::vector<geom::point_2> loc_points;
    geom::rotation_3 const rot(cpr(poly.front().lon, poly.front().lat, 0.));

    for (auto it = poly.begin(), end = poly.end(); it != end; ++it)
    {
        geom::point_3       const n  = polar_point_3(1., it->lon, it->lat);
        geom::polar_point_3 const nr = n * rot;
        loc_points.push_back(point_2(nr.course, nr.pitch));
    }

    if (geom::eq(loc_points.front(), loc_points.back()))
        loc_points.pop_back();

    std::vector<size_t> indices;
    indices.reserve(loc_points.size() * 3);
    geom::triangulation::cdt(loc_points, std::back_inserter(indices));

    triangulation_t  triangles;
    triangles.reserve(loc_points.size());

    for (size_t i = 0, size = indices.size() / 3; i != size; ++i)
    {
        geom::geo_triangle_2 gt(poly[indices[3 * i]], poly[indices[3 * i + 1]], poly[indices[3 * i + 2]]);
        triangles.push_back(gt);
    }
    return std::move(triangles);
}

/**
* A 2D grid along latitude-longitude cells with the ability to store contours (ids).
**/
template <class CellData>
struct geo_grid
{
    typedef geo_grid<CellData>       this_type;

    geo_grid();
    geo_grid(geom::geo_point_2 const& lower_left, geom::geo_point_2 const& step, geom::point_2i const& size);

    virtual ~geo_grid() { }

    geom::geo_rect_2 bounds() const;

    CellData      & operator[](geom::geo_point_2 const& geo_pos);
    CellData const& operator[](geom::geo_point_2 const& geo_pos) const;

    bool        has_neighbour(geom::geo_point_2 const& geo_pos, neighbour_type nei);
    CellData      & neighbour(geom::geo_point_2 const& geo_pos, neighbour_type nei);
    CellData const& neighbour(geom::geo_point_2 const& geo_pos, neighbour_type nei) const;

    // you can still use this class without Contours
    template<class Contour>
    void add_contour   (triangulation_t const& triang, Contour cntr);
    template<class Contour>
    void remove_contour(triangulation_t const& triang, Contour cntr);
    bool has_contours  (geom::geo_point_2 const& geo_pos) const;

    void resize(geom::geo_point_2 const& lower_left, geom::geo_point_2 const& step, geom::point_2i const& size);

protected:
    template <class Visitor>
    void visit_contour(triangulation_t const& triang, Visitor& visitor);

    typedef geom::grid_2d<CellData>        lat_lon_grid_t;
    typedef scoped_ptr<lat_lon_grid_t>  lat_lon_grid_ptr;

protected:
    lat_lon_grid_ptr  data_;  // [lat, lon] -> CellData

protected:
    template <class Derived, class Contour>
    struct contour_visitor
    {
        contour_visitor(lat_lon_grid_t& grid, Contour& cntr)
            : grid_(grid)
            , cntr_(cntr)
        { }

        void set_triangle(geom::geo_triangle_2 const& tri) { triangle_ = tri; }

        template <class State, class CellType>
        bool operator()(State const& /*state*/, CellType& cell)
        {
            dynamic_cast<Derived*>(this)->operation(cell);

            return false;
        }

    protected:
        virtual ~contour_visitor() { }

        lat_lon_grid_t     & grid_;
        geom::geo_triangle_2   triangle_;
        Contour            & cntr_;
    };

    template<class Contour>
    struct add_contour_visitor
        : contour_visitor<add_contour_visitor<Contour>, Contour>
    {
        add_contour_visitor(lat_lon_grid_t& grid, Contour& cntr)
            : contour_visitor<add_contour_visitor<Contour>, Contour>(grid, cntr)
        { }

        template <class CellType>
        void operation(CellType& cell)
        {
            cell.add_contour(this->cntr_);
        }
    };

    template<class Contour>
    struct remove_contour_visitor
        : contour_visitor<remove_contour_visitor<Contour>, Contour>
    {
        remove_contour_visitor(lat_lon_grid_t& grid, Contour& cntr)
            : contour_visitor<remove_contour_visitor<Contour>, Contour>(grid, cntr)
        { }

        template <class CellType>
        void operation(CellType& cell)
        {
            cell.remove_contour(this->cntr_);
        }
    };
};



/************************ Implementations ************************/
namespace
{
geom::geo_point_2 latlon_grid_origin()
{
    return geom::geo_point_2(-90., -180.);
}
geom::geo_point_2 latlon_grid_step()
{
    return geom::geo_point_2(1., 1.);
}
geom::point_2i latlon_grid_size()
{
    static point_2i grid_size = point_2i();
    if (grid_size.x != 0 && grid_size.y != 0)
        return grid_size;

    geom::geo_point_2 const grid_step = latlon_grid_step();

    grid_size.x = int(360 / grid_step.lon);
    if (grid_size.x * grid_step.lon < 360)
        ++grid_size.x;
    grid_size.y = int(180 / grid_step.lat);
    if (grid_size.y * grid_step.lat < 180)
        ++grid_size.y;

    return grid_size;
}
} // anonymous namespace


template <class CellData>
geo_grid<CellData>::geo_grid()
   : data_(new lat_lon_grid_t(geom::aa_transform(
           geom::point_2(latlon_grid_origin().lon, latlon_grid_origin().lat),
           geom::point_2(latlon_grid_step().lon  , latlon_grid_step().lat  )),
           latlon_grid_size(), CellData()))
{ }

template <class CellData>
geo_grid<CellData>::geo_grid(geom::geo_point_2 const& lower_left, geom::geo_point_2 const& step, geom::point_2i const& size)
    : data_(new lat_lon_grid_t(geom::aa_transform(
            geom::point_2(lower_left.lon, lower_left.lat),
            geom::point_2(step.lon      , step.lat      )),
            geom::point_2i(size.y, size.x), CellData()))
{ }

template <class CellData>
geom::geo_rect_2 geo_grid<CellData>::bounds() const
{
    geom::point_2 const& LL = data_->origin();
    geom::point_2 UR = LL;
    UR.x += data_->unit().x * data_->extents().x;
    UR.y += data_->unit().y * data_->extents().y;
    // TODO: hotfix for degree180range
    if (UR.x == 180)
        UR.x = 179;
    return geom::geo_rect_2(geom::range_2(LL.y, UR.y), geom::degree180_range(LL.x, UR.x));
}

template <class CellData>
CellData& geo_grid<CellData>::operator[](geom::geo_point_2 const& geo_pos)
{
    return (*data_)[data_->world2local(geom::point_2(geo_pos.lon, geo_pos.lat))];
}

template <class CellData>
CellData const& geo_grid<CellData>::operator[](geom::geo_point_2 const& geo_pos) const
{
    return operator[](geo_pos);
}

template <class CellData>
bool geo_grid<CellData>::has_neighbour(geom::geo_point_2 const& geo_pos, neighbour_type nei)
{
    geom::point_2 p2(geo_pos.lon, geo_pos.lat);
    p2 = neighbouring_point(p2, data_->unit(), nei);
    return data_->contains(p2);
}

template <class CellData>
CellData& geo_grid<CellData>::neighbour(geom::geo_point_2 const& geo_pos, neighbour_type nei)
{
    geom::point_2 p2(geo_pos.lon, geo_pos.lat);
    p2 = neighbouring_point(p2, data_->unit(), nei);
    return (*data_)[data_->world2local(p2)].data;
}

template <class CellData>
CellData const& geo_grid<CellData>::neighbour(geom::geo_point_2 const& geo_pos, neighbour_type nei) const
{
    return neighbour(geo_pos, nei);
}

template <class CellData>
template <class Visitor>
void geo_grid<CellData>::visit_contour(triangulation_t const& triang, Visitor& visitor)
{
    for (size_t i = 0; i < triang.size(); ++i)
    {
        geom::geo_triangle_2 const& gt = triang[i];
        geom::geo_rect_2 rc = geom::bounding(gt);

        visitor.set_triangle(gt);
        if (rc.lon.contains(180.))
        {
            geom::visit(*data_, geom::rectangle_2(geom::range_2(rc.lon.lo(), 180.) , rc.lat), visitor);
            geom::visit(*data_, geom::rectangle_2(geom::range_2(-180., rc.lon.hi()), rc.lat), visitor);
        }
        else
            geom::visit(*data_, geom::rectangle_2(geom::range_2(rc.lon.lo(), rc.lon.hi()), rc.lat), visitor);
    }
}

template <class CellData>
template <class Contour>
void geo_grid<CellData>::add_contour(triangulation_t const& triang, Contour cntr)
{
    this_type::add_contour_visitor<Contour> acv(*data_, cntr);
    visit_contour<this_type::add_contour_visitor<Contour> >(triang, acv);
}

template <class CellData>
template <class Contour>
void geo_grid<CellData>::remove_contour(triangulation_t const& triang, Contour cntr)
{
    this_type::remove_contour_visitor<Contour> rcv(*data_, cntr);
    visit_contour<this_type::remove_contour_visitor<Contour> >(triang, rcv);
}

template <class CellData>
bool geo_grid<CellData>::has_contours(geom::geo_point_2 const& geo_pos) const
{
    geom::point_2 const idx(data_->world2local(geom::point_2(geo_pos.lon, geo_pos.lat)));
    return !(*data_)[idx].has_contours();
}

template <class CellData>
void geo_grid<CellData>::resize(geom::geo_point_2 const& lower_left, geom::geo_point_2 const& step, geom::point_2i const& size)
{
    data_.reset(new lat_lon_grid_t(geom::aa_transform(
                geom::point_2(lower_left.lon, lower_left.lat),
                geom::point_2(step.lon      , step.lat      )),
                geom::point_2i(size.y, size.x), CellData()));
}
} // namespace geometry

