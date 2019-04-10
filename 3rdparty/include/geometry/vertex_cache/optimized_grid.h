/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace util
{
   namespace detail
   {
      template<typename index_type>
      inline index_type horizontal_indexer( unsigned x, unsigned y, unsigned width, unsigned height )
      {
         return index_type(y * width + x);
      }

      template<typename index_type>
      inline index_type vertical_indexer( unsigned x, unsigned y, unsigned width, unsigned height )
      {
         return index_type(x * height + y);
      }

      template<typename index_type>
      inline void ccw_filler( index_type *& index, index_type i1, index_type i2, index_type i3 )
      {
         *(index++) = i1;
         *(index++) = i2;
         *(index++) = i3;
      }

      template<typename index_type>
      inline void cw_filler( index_type *& index, index_type i1, index_type i2, index_type i3 )
      {
         *(index++) = i3;
         *(index++) = i2;
         *(index++) = i1;
      }
   }


   //
   // fill grid area indices
   //
   //    [left,right] - area horizontal size ([0..width-1] - all horizontal range)
   //    [bottom,top] - area vertical size ([0..height-1] - all vertical range)
   //    width,height - dimension of vertices grid
   //
   //    returns - indices count
   //

   template<typename index_type, class indexer_type, class filler_type>
   inline unsigned fill_grid_indices( index_type * indices, unsigned left, unsigned right, unsigned bottom, unsigned top, unsigned width, unsigned height, indexer_type indexer, filler_type filler )
   {
      index_type * index = indices;
      for (unsigned y = bottom; y < top; y++)
      {
         for (unsigned x = left; x < right; x++)
         {
            // lower-left triangle
            filler(index,
               indexer(x, y + 1, width, height),
               indexer(x, y, width, height),
               indexer(x + 1, y, width, height));

            // upper-right triangle
            filler(index,
               indexer(x + 1, y, width, height),
               indexer(x + 1, y + 1, width, height),
               indexer(x, y + 1, width, height));
         }
      }

      return index - indices;
   }

   template<typename index_type>
   inline unsigned fill_grid_indices( std::vector<index_type> & indices, unsigned left, unsigned right, unsigned bottom, unsigned top, unsigned width, unsigned height )
   {
      unsigned const
         grid_width = right - left,
         grid_height = top - bottom,
         indices_count = grid_width * grid_height * 6;

      indices.resize(indices_count);

      unsigned const indices_filled = fill_grid_indices(&indices.front(), left, right, bottom, top,
         width, height, detail::horizontal_indexer<index_type>, detail::ccw_filler<index_type>);
      Assert(indices_filled == indices_count);

      return indices_filled;
   }


   //
   // fill grid area indices with cache optimization
   //
   //    [left,right] - area horizontal size ([0..width-1] - all horizontal range)
   //    [bottom,top] - area vertical size ([0..height-1] - all vertical range)
   //    width,height - dimension of vertices grid
   //
   //    returns - indices count
   //

   template<typename index_type>
   inline unsigned fill_grid_indices_vco( std::vector<index_type> & indices, unsigned left, unsigned right, unsigned bottom, unsigned top, unsigned width, unsigned height, unsigned cache_size )
   {
      if (cache_size <= 2)
         return 0;

      unsigned const
         grid_width = right - left,
         grid_height = top - bottom,
         cached_vertices = cache_size - 1,
         cached_grid_size = cached_vertices - 1,
         cached_columns = grid_width / cached_grid_size,
         grid_width_remainder = grid_width - cached_columns * cached_grid_size,
         cached_rows = grid_width_remainder > 0 ? grid_height / cached_grid_size : 0,
         grid_height_remainder = grid_width_remainder > 0 ? grid_height - cached_rows * cached_grid_size : 0,
         column_indices_count = grid_height * cached_grid_size * 6,
         row_indices_count = grid_width_remainder * cached_grid_size * 6,
         cache_init_triangles_count = cached_grid_size,
         cache_init_indices_count = cache_init_triangles_count * 3,
         cached_grid_count =
            (cache_init_indices_count + column_indices_count) * cached_columns +
            (cache_init_indices_count + row_indices_count) * cached_rows;

      indices.resize(cached_grid_count, 0);

      unsigned idx = 0;
      unsigned const col_bottom = bottom, col_top = top;
      for (unsigned col_idx = 0; col_idx < cached_columns; col_idx++)
      {
         unsigned const
            col_left = left + col_idx * cached_grid_size,
            col_right = left + (col_idx + 1) * cached_grid_size;

         for (unsigned init_triangle_idx = 0; init_triangle_idx < cache_init_triangles_count; init_triangle_idx++)
         {
            indices[idx++] = detail::horizontal_indexer<index_type>(col_left + init_triangle_idx, col_bottom, width, height);
            indices[idx++] = detail::horizontal_indexer<index_type>(col_left + init_triangle_idx + 1, col_bottom, width, height);
            indices[idx++] = detail::horizontal_indexer<index_type>(col_left + init_triangle_idx, col_bottom, width, height);
         }

         unsigned const indices_filled = fill_grid_indices(&indices[idx], col_left, col_right, col_bottom, col_top,
            width, height, detail::horizontal_indexer<index_type>, detail::ccw_filler<index_type>);
         idx += indices_filled;
      }
      Assert(idx == (cache_init_indices_count + column_indices_count) * cached_columns);

      unsigned const row_left = left + cached_columns * cached_grid_size, row_right = right;
      for (unsigned row_idx = 0; row_idx < cached_rows; row_idx++)
      {
         unsigned const
            row_bottom = bottom + row_idx * cached_grid_size,
            row_top = bottom + (row_idx + 1) * cached_grid_size;

         for (unsigned init_triangle_idx = 0; init_triangle_idx < cache_init_triangles_count; init_triangle_idx++)
         {
            indices[idx++] = detail::vertical_indexer<index_type>(row_bottom + init_triangle_idx, row_left, height, width);
            indices[idx++] = detail::vertical_indexer<index_type>(row_bottom + init_triangle_idx + 1, row_left, height, width);
            indices[idx++] = detail::vertical_indexer<index_type>(row_bottom + init_triangle_idx, row_left, height, width);
         }

         unsigned const indices_filled = fill_grid_indices(&indices[idx], row_bottom, row_top, row_left, row_right,
            height, width, detail::vertical_indexer<index_type>, detail::cw_filler<index_type>);
         idx += indices_filled;
      }
      Assert(idx ==
         (cache_init_indices_count + column_indices_count) * cached_columns +
         (cache_init_indices_count + row_indices_count) * cached_rows);

      if (grid_height_remainder != 0)
      {
         std::vector<index_type> adjoint_indices;
         unsigned const
            adjoint_left = row_left,
            adjoint_right = right,
            adjoint_bottom = cached_rows * cached_grid_size,
            adjoint_top = top,
            adjoint_grid_width = adjoint_right - adjoint_left,
            adjoint_grid_height = adjoint_top - adjoint_bottom,
            adjoint_cache_size = (adjoint_grid_width > adjoint_grid_height ? adjoint_grid_width : adjoint_grid_height) + 2;
         unsigned const adjoint_indices_filled = fill_grid_indices_vco(adjoint_indices, adjoint_left, adjoint_right,
            adjoint_bottom, adjoint_top, width, height, adjoint_cache_size);
         indices.insert(indices.end(), adjoint_indices.begin(), adjoint_indices.end());
      }

      return indices.size();
   }
}
