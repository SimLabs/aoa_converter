/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "grid_params.h"

#include "geometry/vector_2d/vector_2d.h"
#include "geometry/aa_transform.h"

#include <boost/noncopyable.hpp>

namespace geom
{
    template
        <
            class T,
            class TForm   = aa_transform,
            class Storage = vector_2d<T, extents::rect, extents::rowwise>
        >
    struct grid_2d : TForm, Storage
    {
        typedef T       cell_type;
        typedef TForm   tform_type;
        typedef Storage storage_type;

        typedef typename Storage::extents_type extents_type;

        grid_2d(tform_type const &tform, point_2i const &ext)
            :   tform_type    (tform)
            ,   storage_type  (ext)
        {}

        grid_2d(tform_type const &tform, point_2i const &ext, T const &defval)
            :   tform_type  (tform)
            ,   storage_type(ext, defval)
        {}

        explicit grid_2d(grid_params_type const & gp)
            :   tform_type  (gp.tform())
            ,   storage_type(gp.extents())
        {}

        grid_2d(grid_params_type const & gp, T const & defval)
           :   tform_type  (gp.tform())
           ,   storage_type(gp.extents(), defval)
        {}

        grid_2d() {}

    public:

        grid_params_type grid_params() const { return grid_params_type( tform().origin(), tform().unit(), this->extents() ); }

        tform_type const & tform() const { return *this; }
        tform_type       & tform()       { return *this; }

        storage_type       & array()       { return *this; }
        storage_type const & array() const { return *this; }

        using storage_type::contains;

        bool contains(point_2 const & pt) const
        {   return storage_type::contains(floor(tform_type::world2local(pt))); }
    };

    template <class T>
        rectangle_2 bounding(grid_2d<T> const &grid)
    {
        return
            rectangle_2(
                grid.origin(),
                grid.origin() + (grid.extents() & grid.unit())
                );
    }

    template <class T>
        rectangle_2 bounding(grid_2d<T> const &grid, point_2i const & idx)
    {
        point_2 xy = grid.local2world(idx);
        return rectangle_2(xy, xy + grid.unit());
    }

   template< class T >
   point_2 center(grid_2d<T> const & grid, point_2i const & idx )
   {
      return grid.local2world( idx + point_2(0.5, 0.5 ) );
   }

   template< class T >
   point_2i index(grid_2d<T> const & grid, point_2 const & pt )
   {
      return floor(grid.world2local(pt));
   }

   template <class T, class TForm, class Storage>
      inline void write (binary::output_stream& stream, geom::grid_2d<T, TForm, Storage> const & grid)
   {
      write(stream, grid.tform());
      write(stream, grid.array());
   }

   template <class T, class TForm, class Storage>
      inline void read (binary::input_stream& stream, geom::grid_2d<T, TForm, Storage> & grid)
   {
      read(stream, grid.tform());
      read(stream, grid.array());
   }

   struct empty_processor
   {
      template< class State, class CellType >
         bool operator () ( State const &, CellType & )
      {
         return false;
      }
   };
}
