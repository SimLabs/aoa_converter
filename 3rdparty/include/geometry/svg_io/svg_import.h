/*
    * Copyright (C) 2016 SimLabs LLC - All rights reserved.
    * Unauthorized copying of this file or its part is strictly prohibited.
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/common.h"
#include "common/boost.h"

#include "xml/tixml_xinclude.h"

namespace svg
{

//
// svg data holder, placed as contours list
//

template<typename point_type>
struct svg_data_t : boost::noncopyable
{
    typedef point_type point_type_t;
    typedef typename point_type_t::scalar_type geom_scalar_type_t;
    typedef geom::rectangle_t<geom_scalar_type_t, point_type_t::dimension> geom_bound_t;

    typedef std::vector<point_type_t> geom_contour_t;
    typedef std::vector<geom_contour_t> geom_contour_array_t;

    typedef std::function<void (point_type_t &)> point_op_func_t;

public:
    svg_data_t(std::string const & path, bool negate_vertical = true);

    template<typename other_point>
    svg_data_t(svg_data_t<other_point> const & other)
    {
        data_.resize(other.get_geom().size());
        unsigned i = 0;
        for (auto && pl : other.get_geom())
        {
            for (auto && p : pl)
                data_[i].emplace_back(point_type_t(p));
            ++i;
        }

        update_bounds();
    }

public: // interface
    geom_bound_t const & get_bound() const { return data_bound_; }
    geom_contour_array_t const & get_geom() const { return data_; }

    void apply_op(point_op_func_t const & op_func);

private: // impl
    void update_bounds();

private: // data
    geom_contour_array_t data_;
    geom_bound_t data_bound_;
};

namespace util
{

// helper normalization functor
template<typename svg_data_t>
__forceinline typename svg_data_t::point_op_func_t converter_to_01(typename svg_data_t::geom_bound_t const & original_bound)
{
    return [&original_bound](svg_data_t::point_type_t & pnt)
    {
        pnt = original_bound.map(pnt);
    };
}

template<typename svg_data_t>
__forceinline typename svg_data_t::point_op_func_t converter_to_11(typename svg_data_t::geom_bound_t const & original_bound)
{
    return [&original_bound](svg_data_t::point_type_t & pnt)
    {
        pnt = original_bound.map(pnt) * 2 - svg_data_t::point_type_t::from_scalar(1);
    };
}

template<typename svg_data_t>
__forceinline typename svg_data_t::point_op_func_t y_negater()
{
    return [](svg_data_t::point_type_t & pnt)
    {
        pnt.y = -pnt.y;
    };
}


}

// helpers
typedef svg_data_t<geom::point_2f> svg_planar_data;
typedef std::shared_ptr<svg_planar_data> svg_planar_data_ptr;

typedef svg_data_t<geom::point_3f> svg_3d_data;
typedef std::shared_ptr<svg_3d_data> svg_3d_data_ptr;


//////////////////////////////////////////////////////////////////////////

// read function
template <typename svg_class>
__forceinline std::shared_ptr<svg_class> read_svg(std::string const & path)
{
    namespace fs = boost::filesystem;

    return fs::is_regular_file(path) ? std::make_shared<svg_class>(path) : nullptr;
}

// read cached, converted to 3d
__forceinline svg_3d_data_ptr get_cached_svg(std::string const & path)
{
    static std::map<std::string, svg_3d_data_ptr> cache;

    auto it = cache.find(path);
    if (it != cache.end())
    {
        return it->second;
    }
    else
    {
        auto planar_item = read_svg<svg_planar_data>(path);
        if (!planar_item)
            return nullptr;

        // normalize coordinates to be NDC
        planar_item->apply_op(util::converter_to_11<svg_planar_data>(planar_item->get_bound()));
        // convert to 3d points and save
        auto new_item = std::make_shared<svg_3d_data>(*planar_item);
        cache.emplace(std::make_pair(path, new_item));

        return new_item;
    }
}
//////////////////////////////////////////////////////////////////////////

//
// impl
//

// ctor
template<typename point_type>
__forceinline svg_data_t<point_type>::svg_data_t(std::string const & path, bool negate_vertical)
{
    using namespace tinyxml2;

    // svg structure visitor class
    struct svg_visitor_t : XMLVisitor
    {
        svg_visitor_t(geom_contour_array_t & contours) : contours_(contours) {}

        bool VisitEnter(const XMLElement& element, const XMLAttribute* firstAttribute) override
        {
            static const std::string
                polyline_name = "polyline",
                polygon_name = "polygon",
                line_name = "line";

            if (polyline_name == element.Name())
            {
                // read polyline
                geom_contour_t polyline;
                auto && points_text = element.Attribute("points");

                double x = 0, y = 0;
                int fields_read = 0, chars_read = 0;
                do {
                    fields_read = sscanf_s(points_text, "%lf,%lf%n", &x, &y, &chars_read);
                    if (fields_read >= 2)
                    {
                        point_type new_pnt;
                        new_pnt.x = x;
                        new_pnt.y = y;
                        polyline.emplace_back(new_pnt);
                        points_text += chars_read;
                    }
                    else
                        break;
                } while (fields_read > 0);

                if (!polyline.empty())
                    contours_.emplace_back(polyline);
            }
            else if (polygon_name == element.Name())
            {
                // read polygon
                geom_contour_t polygon;
                auto && points_text = element.Attribute("points");

                double x = 0, y = 0;
                int fields_read = 0, chars_read = 0;
                do {
                    fields_read = sscanf_s(points_text, "%lf,%lf%n", &x, &y, &chars_read);
                    if (fields_read >= 2) // GE is because it is not clearly specified if %n increases output counter
                    {
                        point_type new_pnt;
                        new_pnt.x = x;
                        new_pnt.y = y;
                        polygon.emplace_back(new_pnt);
                        points_text += chars_read;
                    }
                    else
                        break;
                } while (fields_read > 0);

                // polygon is closed polyline, so add very first point
                if (!polygon.empty())
                {
                    polygon.push_back(polygon.front());
                    contours_.emplace_back(polygon);
                }
            }
            else if (line_name == element.Name())
            {
                // read line
                geom_contour_t line;

                const double
                    x1 = atof(element.Attribute("x1")),
                    y1 = atof(element.Attribute("y1")),
                    x2 = atof(element.Attribute("x2")),
                    y2 = atof(element.Attribute("y2"));

                point_type new_pnt;
                new_pnt.x = x1;
                new_pnt.y = y1;
                line.push_back(new_pnt);
                new_pnt.x = x2;
                new_pnt.y = y2;
                line.push_back(new_pnt);

                contours_.emplace_back(line);
            }

            return XMLVisitor::VisitEnter(element, firstAttribute);
        }

    private:
        geom_contour_array_t & contours_;
    };

    XMLDocument svg_doc;
    load_file(svg_doc, path);

    svg_visitor_t geom_collector(data_);
    svg_doc.Accept(&geom_collector);

    if (negate_vertical)
        apply_op(util::y_negater<svg_data_t<point_type>>());
    else
        update_bounds();
}

// functor application (MAP pattern)
template<typename point_type>
__forceinline void svg_data_t<point_type>::apply_op(point_op_func_t const & op_func)
{
    for (auto && contour : data_)
        for (auto && vert : contour)
            op_func(vert);

    update_bounds();
}

// update bounds
template<typename point_type>
__forceinline void svg_data_t<point_type>::update_bounds()
{
    data_bound_ = geom_bound_t();
    for (auto && contour : data_)
        for (auto && vert : contour)
            data_bound_ |= vert;
}

}
