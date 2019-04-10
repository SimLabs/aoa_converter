/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "grid.h"

namespace geom
{
    template <class G>
        struct visit_grid1l_every_cell
    {
        typedef G                       grid_type;
        typedef point_2i                smallidx_type;

        template <class Processor>
            static bool process(grid_type & grid, Processor & processor)
        {
            for (auto it = grid.begin(); it != grid.end(); ++it)
            {
                if (processor(it, *it))
                    return true;
            }

            return false;
        }
    };

    template <class T, class Processor>
        inline bool visit_every_cell(grid_2d<T> & grid, Processor & processor)
    {
        return visit_grid1l_every_cell<grid_2d<T> >::process(grid, processor);
    }

    template <class T, class Processor>
        inline bool visit_every_cell(MappedGrid1L<T> const & grid, Processor & processor)
    {
        return visit_grid1l_every_cell<MappedGrid1L<T> const >::process(grid, processor);
    }
}
