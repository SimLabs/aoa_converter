/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "common/fixed_id_vector.h"
#include "geometry/geo/geo_utils.h"
#include "geometry/primitives/point_fwd.h"
#include "geometry/primitives/geo_point_fwd.h"

#include "alloc/pool_stl.h"

namespace geom
{
    template <class Scalar>
    struct duplicate_point_2_traits
    {
        typedef Scalar scalar_type;
        typedef point_t<Scalar, 2> point_type;

        struct Comparer
        {
            bool operator () (point_type const & lhs, point_type const & rhs) const
            {
                return lhs.x < rhs.x || lhs.x == rhs.x && lhs.y < rhs.y;
            }
        };

        point_type construct_start_point(point_type const & src, scalar_type majorAxis) const
        {
            point_type pt(src);
            pt.x = src.x - majorAxis;
            pt.y = src.y;
            return pt;
        }

        bool  is_farther(point_type const & pt, point_type const & src, scalar_type majorAxis) const
        {
            return pt.x - src.x > majorAxis;
        }

        bool is_closer(point_type const & pt, point_type const & src, scalar_type threshold) const
        {
            return distance_sqr(pt, src) < threshold * threshold;
        }
    };

    template <class Scalar>
    struct duplicate_point_3_traits
        : duplicate_point_2_traits< Scalar >
    {
        typedef point_t<Scalar, 3> point_type;

        struct Comparer
        {
            bool operator () (point_type const & lhs, point_type const & rhs) const
            {
                return lhs.x < rhs.x
                    || (lhs.x == rhs.x && lhs.y < rhs.y)
                    || (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z);
            }
        };

        point_type construct_start_point(point_type const & src, double majorAxis) const
        {
            return point_type(src.x - majorAxis, src.y, src.z);
        }
    };

    struct duplicate_geo_point_2_traits
    {
        struct Comparer
        {
            bool operator () (geo_point_2 const & lhs, geo_point_2 const & rhs) const
            {
                return lhs.lat < rhs.lat || lhs.lat == rhs.lat && lhs.lon < rhs.lon;
            }
        };

        geo_point_2 construct_start_point(geo_point_2 const & src, double majorAxis) const
        {
            geo_point_2 pt(src);
            pt.lat = src.lat - majorAxis / wgs84::Rbig * geom::rad2grad();
            pt.lon = src.lon;
            return pt;
        }

        bool  is_farther(geo_point_2 const & pt, geo_point_2 const & src, double majorAxis) const
        {
            return (pt.lat - src.lat) * geom::grad2rad() * wgs84::Rbig > majorAxis;
        }

        bool is_closer(geo_point_2 const & pt, geo_point_2 const & src, double threshold) const
        {
            return distance_sqr(pt, src) < threshold * threshold;
        }
    };

    struct duplicate_geo_point_3_traits
        : duplicate_geo_point_2_traits
    {
        struct Comparer
        {
            bool operator () (geo_point_3 const & lhs, geo_point_3 const & rhs) const
            {
                return lhs.lat < rhs.lat
                    || (lhs.lat == rhs.lat && lhs.lon < rhs.lon)
                    || (lhs.lat == rhs.lat && lhs.lon == rhs.lon && lhs.height < rhs.height);
            }
        };

        geo_point_3 construct_start_point(geo_point_3 const & src, double majorAxis) const
        {
            return geo_point_3(src.lat - majorAxis / wgs84::Rbig * geom::rad2grad(), src.lon, src.height);
        }
    };


    template < class Point > struct duplicate_traits {};

    template < class Scalar > struct duplicate_traits< point_t<Scalar, 2> >
    { typedef duplicate_point_2_traits< Scalar > type; };

    template < class Scalar > struct duplicate_traits< point_t<Scalar, 3> >
    { typedef duplicate_point_3_traits< Scalar > type; };

    template <> struct duplicate_traits< geo_point_2 >
    { typedef duplicate_geo_point_2_traits type; };

    template <> struct duplicate_traits< geo_point_3 >
    { typedef duplicate_geo_point_3_traits type; };


    template <class Point, class Traits=typename duplicate_traits< Point >::type>
    struct duplicate_points_eliminator
    {
        typedef typename Traits::Comparer                           Comparer;
        typedef typename ph_set<Point, Comparer>::set_t  Points;
        typedef typename Points::const_iterator                     PCI;
        typedef typename Points::iterator                           PI;
        typedef PCI                                                 const_iterator;
        typedef std::pair<PI, bool>                                 insert_return_type;

        duplicate_points_eliminator(double eps)
            :   epsilon_    (eps)
        {}

        double eps() const { return epsilon_; }

        insert_return_type    insert(Point const & pt)
        {
            Point const & start_pt = traits_.construct_start_point(pt, epsilon_);

            for (PI it = points_.lower_bound(start_pt); it != points_.end(); ++it)
            {
                if (traits_.is_farther(*it, pt, epsilon_))
                {
                    break;
                }

                if (traits_.is_closer(*it, pt, epsilon_))
                {
                    return std::make_pair(it, false);
                }
            }
            // inserting...
            return points_.insert(pt);
        }

        void clear()
        {
            points_.clear();
        }

        const_iterator begin() const { return points_.begin(); }
        const_iterator end  () const { return points_.end();   }

    private:
        Points       points_;

        double const  epsilon_;
        Traits const  traits_;
    };

template <class Point, class Traits=typename geom::duplicate_traits< Point >::type>
struct duplicate_points_fixed_id
{
    typedef typename fixed_id_vector<Point>::iterator       iterator;
    typedef typename fixed_id_vector<Point>::const_iterator const_iterator;

private:
   typedef typename Traits::Comparer comparer_t;
   typedef typename ph_map<Point, binary::size_type>::map_t points_map_type;

public:
   typedef std::pair<size_t, bool> insert_return_type;

   duplicate_points_fixed_id( double eps )
      : epsilon_( eps )
      , traits_ ()
   {}

   duplicate_points_fixed_id( duplicate_points_fixed_id const& other )
      : traits_ ()
   {
      *this = other;
   }

   void save(binary::output_stream& stream) const
   {
       write(stream, epsilon_);
       write(stream, points_map_);
       write(stream, points_);
   }

   
   void load(binary::input_stream& stream)
   {
       read(stream, epsilon_);
       read(stream, points_map_);
       read(stream, points_);
   }


//    duplicate_points_fixed_id & operator=( duplicate_points_fixed_id const& other )
//    {
//       traits_ = other.traits_;
//       epsilon_ = other.epsilon_;
//       points_map_ = other.points_map_;
//       for (points_map_type::const_iterator it = points_map_.begin(); it != points_map_.end(); ++it)
//          points_.insert(it->second, it);
// 
//       return *this;
//    }

   size_t size() const
   {
      return points_.size();
   }

   const_iterator begin() const { return points_.begin(); }
   const_iterator end  () const { return points_.end();   }

   // true in case of successful inserting
   insert_return_type  insert( Point const & pt )
   {
      Point const & start_pt = traits_.construct_start_point(pt, epsilon_);

      for (auto it = points_map_.lower_bound(start_pt); it != points_map_.end(); ++it)
      {
         if (traits_.is_farther(it->first, pt, epsilon_))
         {
            break;
         }

         if (traits_.is_closer(it->first, pt, epsilon_))
         {
            return std::make_pair(it->second, false);
         }
      }

      // inserting...
      size_t id = points_.next_id() ;
      points_map_.insert(std::make_pair(pt, binary::size_type(id)));
      points_.insert(id, pt) ;

      return std::make_pair(id, true) ;
   }

   void erase( size_t id )
   {
      if (points_.valid(id))
      {
         points_map_.erase(points_[id]) ;
         points_.erase(id) ;
      }
   }

   void clear()
   {
        points_map_.clear();
        points_.clear();
   }

   boost::optional<size_t> valid( Point const& pt ) const
   {
      Point const & start_pt = traits_.construct_start_point(pt, epsilon_);

      for (auto it = points_map_.lower_bound(start_pt); it != points_map_.end(); ++it)
      {
         if (traits_.is_farther(it->first, pt, epsilon_))
         {
            return boost::none;
         }

         if (traits_.is_closer(it->first, pt, epsilon_))
         {
            return it->second;
         }
      }

      return boost::none ;
   }

   bool valid( size_t id ) const
   {
      return points_.valid(id);
   }

   Point const & get_point( size_t id ) const { return points_[id] ; }


   struct ids_iterator
      : boost::iterator_adaptor<ids_iterator, typename points_map_type::const_iterator, size_t const, boost::forward_traversal_tag>
   {
      ids_iterator()
      {}

      ids_iterator( typename points_map_type::const_iterator it )
         : ids_iterator::iterator_adaptor_(it)
      {}

   private:
      friend class boost::iterator_core_access;


      size_t const& dereference() const
      {
         return this->base()->second;
      }
   };

   std::pair<ids_iterator, ids_iterator> points_ids() const
   {
      return std::make_pair(ids_iterator(points_map_.begin()), ids_iterator(points_map_.end()));
   }

private:
   Traits const traits_;
   
   double                 epsilon_;
   points_map_type        points_map_ ;
   fixed_id_vector<Point> points_ ;
};


template <class Point, class Traits>
inline void write(binary::output_stream& stream, geom::duplicate_points_fixed_id<Point, Traits> const& dp)
{        
    dp.save(stream);
}

template <class Point, class Traits>
inline void read(binary::input_stream& stream, geom::duplicate_points_fixed_id<Point, Traits> & dp)
{                   
    dp.load(stream);
}

} // geom
