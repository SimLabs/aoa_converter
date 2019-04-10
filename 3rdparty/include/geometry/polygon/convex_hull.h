/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "common/util.h"
#include "primitives/point_fwd.h"
#include "primitives/rectangle_fwd.h"
#include "point_predicates.h"

#include <iterator>
#include <boost/utility.hpp>

namespace geom {

    // convex hull routine
    // assume points are laid as vector
    // enveloping is used, so it is O(n*p)
    template<typename vector_type, typename output_iterator_type>
    inline void point_set_convex_hull_envelope( const vector_type * points, unsigned count, output_iterator_type out )
    {
        // find very first point, which we should start with
        // let it be most lower-left
        unsigned start_idx = 0;
        for (unsigned i = 1; i < count; ++i)
        {
            if (points[i].x < points[start_idx].x || (points[i].x == points[start_idx].x && points[i].y < points[start_idx].y))
                start_idx = i;
        }

        std::vector<unsigned> result;

        // make convex-hull by enveloping (O(n*P), but points count is small)
        unsigned next_idx = start_idx;
        vector_type dir_to_test;
        dir_to_test.x = 0;
        dir_to_test.y = -1;

        // distance threshold
        static const typename vector_type::scalar_type dist_threshold_sqr = 0.000001;

        // scan every other points and build convex hull
        do
        {
            // add point
            result.push_back(next_idx);
            const vector_type & last_point = points[result.back()];
            // go through other, find candidate
            typename vector_type::scalar_type max_dot = -1;
            for (unsigned i = 0; i < count; ++i)
            {
                // skip the same or close points to last one added
                vector_type delta = points[i] - last_point;
                if (norm_sqr(delta) <= dist_threshold_sqr)
                    continue;
                // calc angle dot
                normalize(delta);
                typename vector_type::scalar_type dot = delta * dir_to_test;
                // save max
                if (dot > max_dot)
                {
                    max_dot = dot;
                    next_idx = i;
                }
            }
            // save next dir to test
            dir_to_test = points[next_idx] - last_point;
            normalize(dir_to_test);
        } while (distance_sqr(points[next_idx], points[start_idx]) > dist_threshold_sqr);

        // all is done, collect result
        for (unsigned i = 0; i < result.size(); ++i)
            out = points[result[i]];
    };

}

namespace geom {

   namespace details
   {
      template <typename PointsRnaIter,
                typename IndexesFwdIter,
                typename IndexesOutIter,
                typename RightTurnStrictFunction>
      void build_convexhull_chain(PointsRnaIter points_first,
                                  IndexesFwdIter indices_begin,
                                  IndexesFwdIter indices_end,
                                  IndexesOutIter out,
                                  RightTurnStrictFunction rts_function)
      {
         IndexesFwdIter idx = indices_begin;
         IndexesFwdIter last = boost::prior(indices_end);

         std::vector<size_t> r;
         r.push_back(*idx++);

         for (; idx != last; ++idx)
         {
            if (!rts_function(points_first[r.back()],
                              points_first[*idx],
                              points_first[*last]))
               continue;

            r.push_back(*idx);

            while (r.size() >= 3
               && !rts_function(points_first[r.end()[-3]],
                                points_first[r.end()[-2]],
                                points_first[r.end()[-1]]))
            {
               r.erase(r.end() - 2);
            }
         }

         std::copy(r.begin(), r.end(), out);
      }
   }

   template <typename PointsRnaIter,
             typename IndexesOutIter,
             typename RightTurnStrictFunction>
   void build_convex_hull(PointsRnaIter points_first,
                          PointsRnaIter points_last,
                          IndexesOutIter out,
                          RightTurnStrictFunction rts_function)
   {
      if (points_first == points_last)
         return;

      std::vector<size_t> indexset(points_last - points_first);
      for (size_t i = 0; i != indexset.size(); ++i )
         indexset[i] = i;

      struct index_compare
      {
         index_compare(PointsRnaIter points_first)
            : points_first(points_first)
         {}

         bool operator() (size_t index1, size_t index2) const
         {
            return points_first[index1] < points_first[index2];
         }

      private:
         PointsRnaIter points_first;
      };

      std::sort(indexset.begin(), indexset.end(), index_compare(points_first));

      if ((points_last - points_first) < 3)
      {
         // create convex hull as degenerate rectangle
         std::copy(indexset.begin(), indexset.end(), out);
         return;
      }

      details::build_convexhull_chain(points_first, indexset.begin(),  indexset.end(), out, rts_function);
      details::build_convexhull_chain(points_first, indexset.rbegin(), indexset.rend(), out, rts_function);
   }

   template <typename PointsRnaIter, typename IndexesOutIter>
   void build_convex_hull(PointsRnaIter points_first,
                          PointsRnaIter points_last,
                          IndexesOutIter out)
   {
      typedef typename std::iterator_traits<PointsRnaIter>::value_type::scalar_type scalar_type;
      build_convex_hull(points_first,
                        points_last,
                        out,
                        &geom::right_turn_strict<scalar_type, scalar_type, scalar_type>);
   }


#pragma pack ( push, 1 )
   struct naa_rect_2
   {
      naa_rect_2()
         : angle ( 0 )
      {}

      double getangle() const
      {
         return -angle;
      }
      rectangle_2 getrect() const
      {
         return rectangle_2( rect.x, range_2(-rect.y.hi(), -rect.y.lo()) );
      }

      rectangle_2 rect;
      double angle;
   };
#pragma pack ( pop )

   template < class OutIt >
   inline void points( const naa_rect_2 & nar, OutIt out )
   {
      geom::rectangle_2 rect = nar.getrect();

      double const sina = sin(nar.getangle());
      double const cosa = cos(nar.getangle());

      double const x_len = rect.x.size() / 2;
      double const y_len = rect.y.size() / 2;

      geom::point_2 const p1 = rect.lo();
      geom::point_2 const p2 = rect.hi();

      geom::point_2 const p1_( p1.x*cosa - p1.y*sina, p1.x*sina + p1.y*cosa );
      geom::point_2 const p2_( p2.x*cosa - p2.y*sina, p2.x*sina + p2.y*cosa );

      geom::point_2 origin = (p1_+p2_) / 2;
      origin.y = -origin.y;

      geom::point_2 extents[] =
      {
         geom::point_2( cosa, sina  ) * x_len,
         geom::point_2( -sina, cosa ) * y_len
      };

      *out++ = origin + extents[0] + extents[1];
      *out++ = origin - extents[0] + extents[1];
      *out++ = origin - extents[0] - extents[1];
      *out++ = origin + extents[0] - extents[1];
   }

   inline bool eq( const naa_rect_2 & a, const naa_rect_2 & b )
   {
      return eq( a.angle, b.angle ) && eq( a.rect, b.rect );
   }


   // clockwise ordered angle from horizontal right directed axis
   template <class Point>
      inline double vector_angle( Point const &p1, Point const &p2 )
   {
      const double my_eps = 1e-20;

      double dy = p2.y - p1.y;
      double dx = p2.x - p1.x;

      if (eq(dx, 0.0, my_eps))
         dx = my_eps;//epsilon/2;//eps10;

      double alpha_tan = geom::abs(dy / dx);
      double alpha = atan( alpha_tan );

      if (dx>=0 && dy>=0) // first quarter
         alpha = -alpha;
      else
         if (dx>=0 && dy<=0) // second quarter
            ;
         else
            if (dx<=0 && dy<=0) // third quarter
               alpha = pi - alpha;
            else
               //    if (dx<=0 && dy>=0) // forth quarter
               alpha = -(pi - alpha);

      return alpha;
   }




   // find not-axis-alined bounding box
   template <class Point>
      inline void build_naa_rect( Point const *pointset, size_t p_num, naa_rect_2 &naa_rect )
   {
      //Assert( p_num >= 3 );

      // minimal rectangle
      naa_rect.rect = rectangle_2();
      double min_perimetr = std::numeric_limits<double>::max();

      // build convex hull
      std::vector<size_t> hull;
      build_convex_hull( pointset, pointset + p_num, std::back_inserter(hull) );

      int N = hull.size();
      if (N<3)
      {
         // find horizontal bounding box and yield
         for (size_t i=0; i<p_num; i++)
            naa_rect.rect |= point_2( pointset[i].x, -pointset[i].y ) ;
         naa_rect.angle = 0.0;
         return;
      }

      // take first side of the CH and find left-most, right-most and farthest points
      Point const &p1 = pointset[ hull[0] ];
      Point const &p2 = pointset[ hull[1] ];

      double alpha = vector_angle( p1, p2 );

      double sina = sin(alpha);
      double cosa = cos(alpha);

      double minx =  std::numeric_limits<double>::max();
      double maxx = -std::numeric_limits<double>::max();
      double miny =  std::numeric_limits<double>::max();
      double maxy = -std::numeric_limits<double>::max();

      int left    = -1;
      int right   = -1;
      int top     = -1;
      int bottom  = -1;

      for (int i=0; i<N; i++)
      {
         Point const &p = pointset[ hull[i] ];
         double x =   p.x * cosa  +  (-p.y) * sina;
         double y = - p.x * sina  +  (-p.y) * cosa;

         if (minx > x) { minx = x; left   = i; }
         if (maxx < x) { maxx = x; right  = i; }
         if (miny > y) { miny = y; bottom = i; }
         if (maxy < y) { maxy = y; top    = i; }
      }

      // check perimeter
      double perimetr = (maxx - minx) + (maxy - miny);
      if (perimetr < min_perimetr)
      {
         min_perimetr = perimetr;
         naa_rect.rect = rectangle_2( range_2(minx, maxx), range_2(miny, maxy) );
         naa_rect.angle = alpha;
      }

      // scan all points of CH clockwise ordered
      for (int cur = 1; cur<N; cur++ )
      {
         int next = (cur + 1) % N;

         // take new segment
         Point const &p1 = pointset[ hull[cur ] ];
         Point const &p2 = pointset[ hull[next] ];

         // calculate new rotation angle
         double alpha = vector_angle( p1, p2 );

         double sina = sin(alpha);
         double cosa = cos(alpha);

         // update left point
         {
            Point const &p = pointset[ hull[left] ];
            minx =   p.x * cosa  +  (-p.y) * sina;

            do
            {
               int left1 = (left + 1) % N;
               Point const &p = pointset[ hull[left1] ];
               double x =   p.x * cosa  +  (-p.y) * sina;

               if (x < minx) // + eps10)
               {
                  minx = x;
                  left = left1;
               } else
                  break;

            } while (true);
         }

         // update right point
         {
            Point const &p = pointset[ hull[right] ];
            maxx =   p.x * cosa  +  (-p.y) * sina;

            do
            {
               int right1 = (right + 1) % N;
               Point const &p = pointset[ hull[right1] ];
               double x =   p.x * cosa  +  (-p.y) * sina;

               if (x > maxx)// - eps10)
               {
                  maxx = x;
                  right = right1;
               } else
                  break;

            } while (true);
         }

         // update lower point
         {
            Point const &p = pointset[ hull[bottom] ];
            miny = - p.x * sina  +  (-p.y) * cosa;

            do
            {
               int bottom1 = (bottom + 1) % N;
               Point const &p = pointset[ hull[bottom1] ];
               double y = - p.x * sina  +  (-p.y) * cosa;

               if (y < miny)// + eps10)
               {
                  miny = y;
                  bottom = bottom1;
               } else
                  break;

            } while (true);
         }

         // update upper point
         {
            Point const &p = pointset[ hull[top] ];
            maxy = - p.x * sina  +  (-p.y) * cosa;

            do
            {
               int top1 = (top + 1) % N;
               Point const &p = pointset[ hull[top1] ];
               double y = - p.x * sina  +  (-p.y) * cosa;

               if (y > maxy)// - eps10)
               {
                  maxy = y;
                  top = top1;
               } else
                  break;

            } while (true);
         }

         // check perimeter
         double perimetr = (maxx - minx) + (maxy - miny);
         if (perimetr < min_perimetr)
         {
            min_perimetr = perimetr;
            naa_rect.rect = rectangle_2( range_2(minx, maxx), range_2(miny, maxy) );
            naa_rect.angle = alpha;
         }

      } // for (cur)
   }

}
