/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <limits>
#include <vector>
#include <algorithm>

#include "boost/scoped_array.hpp"
#include "boost/circular_buffer.hpp"


namespace util
{
   namespace detail
   {
      struct cached_vertex_score_data
      {
         int cache_position, score;

         typedef std::vector<unsigned> triangles_vector;
         triangles_vector triangles;

         cached_vertex_score_data()
            : cache_position(-1)
         {
            triangles.reserve(8);
         }
      };

      template<typename index_type>
      struct cached_triangle_score_data
      {
         index_type indices[3];
         int score;
         bool added;

         cached_triangle_score_data()
            : score(0)
            , added(false)
         {
            indices[0] = indices[1] = indices[2] = 0;
         }
      };


      template<typename score_type, int multiplier>
      struct score_tables_data
      {
         std::vector<score_type> vertex_cache_score, valence_boost_score;

         score_tables_data()
         {
         }

         void init( unsigned vertex_cache_size, unsigned max_vertex_usage )
         {
            if (vertex_cache_score.size() != vertex_cache_size)
            {
               vertex_cache_score.resize(vertex_cache_size);
               for (unsigned i = 0; i < vertex_cache_size; i++)
                  vertex_cache_score[i] = (score_type)(calculate_vertex_cahce_score(i, vertex_cache_size) * multiplier);
            }

            while (valence_boost_score.size() < max_vertex_usage)
            {
               valence_boost_score.push_back((score_type)(calculate_valence_boost_score((unsigned)valence_boost_score.size() + 1) * multiplier));
            }
         }

         inline score_type vertex_score( int cache_position, unsigned vertex_usage )
         {
            Assert(cache_position < (int)vertex_cache_score.size());
            Assert(vertex_usage <= valence_boost_score.size());

            if (vertex_usage == 0)
               return -1; // No triangles needs this vertex

            score_type score = cache_position >= 0 ? vertex_cache_score[cache_position] : 0;
            score += valence_boost_score[vertex_usage - 1];

            return score;
         }

         static float calculate_vertex_cahce_score( unsigned cache_position, unsigned vertex_cache_size )
         {
            if (cache_position < 3)
            {
               // This vertex was used in the last triangle,
               // so it has a fixed score, whichever of the three
               // it's in. Otherwise, you can get very different
               // answers depending on whether you add
               // the triangle 1,2,3 or 3,1,2 - which is silly.
               float const last_triangle_score = 0.75f;
               return last_triangle_score;
            }
            else
            {
               Assert(cache_position < vertex_cache_size);
               // Points for being high in the cache.
               float const
                  cache_decay_power = 1.5f,
                  scaler = 1.0f / (vertex_cache_size - 3),
                  pre_score = 1.0f - (cache_position - 3) * scaler,
                  score = powf(pre_score, cache_decay_power);
               return score;
            }
         }

         static float calculate_valence_boost_score( unsigned vertex_usage )
         {
            // Bonus points for having a low number of triangles still to
            // use the vertex, so we get rid of lone vertices quickly.
            float const
               valence_boost_scale = 2.0f,
               valence_boost_power = 0.5f,
               valence_boost = powf((float)vertex_usage, -valence_boost_power),
               score = valence_boost_scale * valence_boost;
            return score;
         }
      };
   }

   template<typename index_type>
   inline void tom_forsyth_optimize( index_type * indices, unsigned indices_count, unsigned vertices_count, unsigned vertex_offset = 0, unsigned vertices_cache_size = 32 )
   {
      Assert(vertices_count != 0);
      Assert(vertices_count <= std::numeric_limits<index_type>::max());

      unsigned const triangles_count = indices_count / 3;
      Assert(triangles_count != 0 && indices_count == triangles_count * 3);

      typedef typename detail::cached_triangle_score_data<index_type> cached_triangle_score_data;
      boost::scoped_array<cached_triangle_score_data> triangles(new cached_triangle_score_data[triangles_count]);

      typedef typename detail::cached_vertex_score_data cached_vertex_score_data;
      boost::scoped_array<cached_vertex_score_data> vertices(new cached_vertex_score_data[vertices_count]);

      unsigned best_triangle_idx = 0;

      typedef typename detail::score_tables_data<int,1000> score_tables_data;
      score_tables_data score_tables;

      unsigned max_vertex_usage = 0;
      // initialization
      {
         index_type const* index = indices;
         for (unsigned triangle_idx = 0; triangle_idx != triangles_count; triangle_idx++, index += 3)
         {
            cached_triangle_score_data & triangle = triangles[triangle_idx];

            triangle.indices[0] = index[0] - vertex_offset;
            triangle.indices[1] = index[1] - vertex_offset;
            triangle.indices[2] = index[2] - vertex_offset;

            cached_vertex_score_data & v0 = vertices[triangle.indices[0]];
            v0.triangles.push_back(triangle_idx);
            if (v0.triangles.size() > max_vertex_usage)
               max_vertex_usage = (unsigned)v0.triangles.size();

            cached_vertex_score_data & v1 = vertices[triangle.indices[1]];
            v1.triangles.push_back(triangle_idx);
            if (v1.triangles.size() > max_vertex_usage)
               max_vertex_usage = (unsigned)v1.triangles.size();

            cached_vertex_score_data & v2 = vertices[triangle.indices[2]];
            v2.triangles.push_back(triangle_idx);
            if (v2.triangles.size() > max_vertex_usage)
               max_vertex_usage = (unsigned)v2.triangles.size();
         }

         score_tables.init(vertices_cache_size, max_vertex_usage);

         for (unsigned vertex_idx = 0; vertex_idx < vertices_count; vertex_idx++)
            vertices[vertex_idx].score = score_tables.vertex_score(vertices[vertex_idx].cache_position, (unsigned)vertices[vertex_idx].triangles.size());

         int best_triangle_score = -1;
         for (unsigned triangle_idx = 0; triangle_idx != triangles_count; triangle_idx++)
         {
            cached_triangle_score_data & triangle = triangles[triangle_idx];
            triangle.score = vertices[triangle.indices[0]].score + vertices[triangle.indices[1]].score + vertices[triangle.indices[2]].score;

            if (triangle.score > best_triangle_score)
            {
               best_triangle_score = triangle.score;
               best_triangle_idx = triangle_idx;
            }
         }
      }

      /// main body of the algorithm
      {
         unsigned triangles_left = triangles_count;
         typedef typename boost::circular_buffer<index_type> vertices_cache_type;
         vertices_cache_type vertices_cache(vertices_cache_size + 3);

         while (triangles_left--)
         {
            /// add triangle to output list
            {
               cached_triangle_score_data & triangle = triangles[best_triangle_idx];
               indices[0] = triangle.indices[0] + vertex_offset;
               indices[1] = triangle.indices[1] + vertex_offset;
               indices[2] = triangle.indices[2] + vertex_offset;
               indices += 3;
               triangle.added = true;

               /// reduce valence of used vertices
               for (unsigned i = 0; i < 3; i++)
               {
                  cached_vertex_score_data & vertex = vertices[triangle.indices[i]];
                  cached_vertex_score_data::triangles_vector::iterator triangle_iter = std::find(vertex.triangles.begin(), vertex.triangles.end(), best_triangle_idx);
                  Assert(triangle_iter != vertex.triangles.end());
                  vertex.triangles.erase(triangle_iter);

                  vertices_cache_type::iterator cache_iter = std::find(vertices_cache.begin(), vertices_cache.end(), triangle.indices[i]);
                  if (cache_iter != vertices_cache.end())
                     vertices_cache.erase(cache_iter); // TODO try rerase if closer to begin
               }

               vertices_cache.push_back(triangle.indices[0]);
               vertices_cache.push_back(triangle.indices[1]);
               vertices_cache.push_back(triangle.indices[2]);
            }

            /// update cache pos and score of vertices in cache
            {
               int cache_position = (int)vertices_cache.size() - 1;
               for (vertices_cache_type::iterator it = vertices_cache.begin(); it != vertices_cache.end(); ++it, cache_position--)
               {
                  cached_vertex_score_data & vertex = vertices[*it];
                  vertex.cache_position = cache_position >= (int)vertices_cache_size ? -1 : cache_position;
                  vertex.score = score_tables.vertex_score(vertex.cache_position, (unsigned)vertex.triangles.size());
               }
            }

            /// update triangles score and look for best triangle
            {
               int best_triangle_score = -1;
               for (vertices_cache_type::iterator it = vertices_cache.begin(); it != vertices_cache.end(); ++it)
               {
                  cached_vertex_score_data & vertex = vertices[*it];

                  for (std::vector<unsigned>::iterator triangle_iter = vertex.triangles.begin(); triangle_iter != vertex.triangles.end(); ++triangle_iter)
                  {
                     cached_triangle_score_data & triangle = triangles[*triangle_iter];
                     Assert(!triangle.added);

                     triangle.score =
                        vertices[triangle.indices[0]].score +
                        vertices[triangle.indices[1]].score +
                        vertices[triangle.indices[2]].score;

                     if (triangle.score > best_triangle_score)
                     {
                        best_triangle_score = triangle.score;
                        best_triangle_idx = *triangle_iter;
                     }
                  }
               }

               if (best_triangle_score == -1)
               {
                  /// best triangle not found
                  /// cache didn't hold vertices with non-added triangles
                  for (unsigned triangle_idx = 0; triangle_idx < triangles_count; triangle_idx++)
                  {
                     cached_triangle_score_data & triangle = triangles[triangle_idx];
                     if (!triangle.added && triangle.score > best_triangle_score)
                     {
                        best_triangle_score = triangle.score;
                        best_triangle_idx = triangle_idx;
                     }
                  }
               }
            }

            while (vertices_cache.size() > vertices_cache_size)
               vertices_cache.pop_front();
         }
      }
   }
}
