/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "grid.h"
#include "geometry/rasterization/segment_raster_iterator.h"

namespace geom
{

    template <class T>
        struct visit_grid1L_by_segment
    {
        template <class Grid, class ActualProcessor>
            struct CellProcessor
        {
            CellProcessor(Grid & data, ActualProcessor & actual_processor)
                : data(data), actual_processor(actual_processor)
            {}

            template<class State>
            bool operator () (State const & st)
            {
                return data.contains(st.cell) ? actual_processor(st, data[st.cell]) : false;
            }

        private:
            Grid            & data;
            ActualProcessor & actual_processor;
        };

        template <class Processor>
            static bool process(grid_2d<T> & grid,
                segment_2 const &s, Processor & processor)
        {
            if (!has_intersection(bounding(grid), s.P0(), s.P1()))
                return false;

            CellProcessor<grid_2d<T>, Processor>    cell_processor(grid, processor);

            return rasterize_segment(grid.tform(), s, cell_processor);
        }

        template <class Processor>
            static bool process(grid_2d<T> const & grid,
                segment_2 const &s, Processor & processor)
        {
            if (!has_intersection(bounding(grid), s.P0(), s.P1()))
                return false;

            CellProcessor<grid_2d<T>, Processor>    cell_processor(grid, processor);

            return rasterize_segment(grid.tform(), s, cell_processor);
        }
    };

    template <class T, class Processor, class Scalar>
        inline bool visit(grid_2d<T> & grid, segment_t< Scalar, 2 > const & seg, Processor & proc)
    {
        return visit_grid1L_by_segment<T>::process(grid, geom::segment_2( seg.P0(), seg.P1() ), proc);
    }

    template <class T, class Processor, class Scalar>
        inline bool visit(grid_2d<T> const & grid, segment_t< Scalar, 2 > const & seg, Processor & proc)
    {
        return visit_grid1L_by_segment<T>::process(const_cast<grid_2d<T>&>(grid), geom::segment_2( seg.P0(), seg.P1() ), proc);
    }
}
