/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <limits>
#include <list>
#include <algorithm>

#include "boost/circular_buffer.hpp"


namespace util
{
   namespace detail
   {
      template<typename index_type, typename index_iterator>
      inline unsigned calculate_cache_misses( index_iterator begin_index, index_iterator end_index, unsigned vertices_cache_size )
      {
         boost::circular_buffer<index_type> vertices_cache(vertices_cache_size);
         unsigned cache_misses = 0;
         for (index_iterator index_iter = begin_index; index_iter != end_index; ++index_iter)
         {
            index_type const index = *index_iter;
            if (std::find(vertices_cache.begin(), vertices_cache.end(), index) == vertices_cache.end())
            {
               cache_misses++;
               vertices_cache.push_back(index);
            }
         }
         return cache_misses;
      }
   }

   template<typename index_iterator>
   inline unsigned calculate_cache_misses( index_iterator begin_index, index_iterator end_index, unsigned vertices_cache_size )
   {
      return detail::calculate_cache_misses<index_iterator::value_type>(begin_index, end_index, vertices_cache_size);
   }

   template<typename index_type>
   inline unsigned calculate_cache_misses( index_type const* indices, unsigned indices_count, unsigned vertices_cache_size )
   {
      return detail::calculate_cache_misses<index_type>(indices, indices + indices_count, vertices_cache_size);
   }


   //
   /// ACMR - Average cache miss ratio calculation,
   /// number of post-transform cache misses per triangle, (0.5,3.0]
   //

   template<typename index_interator>
   inline float calculate_acmr( index_interator begin_index, index_interator end_index, unsigned vertices_cache_size, unsigned primitive_size = 3 )
   {
      return float(calculate_cache_misses(begin_index, end_index, vertices_cache_size) * primitive_size) / (end_index - begin_index);
   }

   template<typename index_type>
   inline float calculate_acmr( index_type const* indices, unsigned indices_count, unsigned vertices_cache_size, unsigned primitive_size = 3 )
   {
      return float(calculate_cache_misses(indices, indices_count, vertices_cache_size) * primitive_size) / indices_count;
   }
}
