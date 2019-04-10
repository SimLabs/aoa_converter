/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/spatial/grid_2d/grid.h"
#include "geometry/spatial/grid_2d/visit_grid_by_segment.h"
#include "geometry/intersection/segment_2_intersection.h"

namespace geom
{

namespace details
{
    struct add_segment_processor_type
    {
        add_segment_processor_type( size_t id )
            : id_( id )
        {}

        template< class State, class cell_type >
        bool operator( )( State const & /*st*/, cell_type & cell )
        {
            std::vector<size_t>::iterator it = std::find(cell.segments.begin(), cell.segments.end(), id_) ;
            if (it == cell.segments.end())
                cell.segments.push_back(id_) ;

            return false ;
        }

    private:
        size_t id_ ;
    };

    struct find_first_intersection_processor_type
    {
        find_first_intersection_processor_type( std::vector<geom::segment_2> const& segments, geom::segment_2 const& seg )
            : seg_( seg )
            , segments_(segments)
        {}

        template< class State, class cell_type >
        bool operator( )( State const & /*st*/, cell_type const & cell )
        {
            for (size_t i = 0; i < cell.segments.size(); i++)
            {
                if (geom::has_intersection(seg_, segments_[cell.segments[i]]))
                {
                    res_ = cell.segments[i] ;
                    return true ;
                }
            }

            return false ;
        }

    private:
        std::vector<geom::segment_2> const& segments_;
        geom::segment_2 seg_;
        size_t res_;
    };

}

struct segments_intersection
{
    segments_intersection( rectangle_2 const& bound )
        : bound_(bound)
        , grid_(boost::in_place<grid_type>(geom::aa_transform(bound.xy(), bound.size() / point_2i(30, 30)), point_2i(30, 30)))
    {
    }

    void add_segment( segment_2 const& seg )
    {
        // rebuild grid if needed
        if (!bound_.contains(seg.P0()) || !bound_.contains(seg.P1()))
        {
            bound_ |= seg.P0();
            bound_ |= seg.P1();
            grid_  = boost::in_place<grid_type>(geom::aa_transform(bound_.xy(), bound_.size() / point_2i(30, 30)), point_2i(30, 30));

            for (size_t i = 0; i < segments_.size(); ++i)
                add_segment_to_grid(i);
        }

        segments_.push_back(seg);
        size_t id = segments_.size() - 1;
        add_segment_to_grid(id);
    }

    bool has_intersection(segment_2 const& seg) const
    {
        details::find_first_intersection_processor_type proc(segments_, seg) ;

        return geom::visit(*grid_, seg, proc) ;
    }

private:
    void add_segment_to_grid(size_t id)
    {
        details::add_segment_processor_type proc(id) ;
        geom::visit(*grid_, segments_[id], proc) ;
    }

private:
    struct cell_type
    {
        std::vector<size_t> segments ;
    };

    typedef geom::grid_2d<cell_type> grid_type ;

    geom::rectangle_2 bound_;
    optional<grid_type> grid_ ;
    std::vector<segment_2> segments_;
};

}