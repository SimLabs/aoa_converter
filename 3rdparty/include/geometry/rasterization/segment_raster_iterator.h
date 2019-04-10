/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "lazy_transform.h"

#include "details/long_number.h"
#include "details/long_point.h"

#include "details/point_world_to_local.h"

#include "geometry/primitives/point.h"
#include "geometry/primitives/segment.h"
#include "geometry/aa_transform.h"

namespace geom
{
namespace segment_rasterization
{
   namespace details
   {
      using geom::point_2;
      using geom::point_2i;
      using geom::segment_2;
      using geom::aa_transform;

      using geom::rasterization::details::long_number;
      using geom::rasterization::details::long_point;


      using geom::rasterization::details::exact_world_to_local;
      using geom::rasterization::details::world_to_local;

      template<class TForm = aa_transform>
      struct long_data
      {
         long_data(segment_2 const & seg, TForm const & tform)
            : seg_(seg)
            , tform_(tform)
         {}

         long_point const & size()
         {
            if (!size_.get())
               init();
            return *size_;
         }

         long_point const & shift()
         {
            if (!shift_.get())
               init();
            return *shift_;
         }

         long_point const & unit()
         { 
            if(!unit_.get())
               init();
            return *unit_; 
         }

      private:
         void init();

      private:
         segment_2     seg_;
         TForm         tform_;

         boost::scoped_ptr<long_point> unit_;
         boost::scoped_ptr<long_point> shift_;
         boost::scoped_ptr<long_point> size_;
      };

      template<>
      inline void long_data<aa_transform>::init()
      {
         size_.reset(new long_point(seg_[1]));
         shift_.reset(new long_point(tform_.origin()));
         unit_.reset(new long_point(tform_.unit()));

         const long_point lo(seg_[0]);
         *size_ -= lo;
         *shift_ -= lo;
      }

      template<>
      inline void long_data<lazy_transform>::init()
      {
         size_ .reset(new long_point(seg_[1]));
         shift_.reset(new long_point(tform_.grid_tform.unit()));

         shift_->x *= tform_.cell_idx.x;
         shift_->y *= tform_.cell_idx.y;

         const long_point grid_origin(tform_.grid_tform.origin());
         *shift_ += grid_origin;

         unit_.reset(new long_point(tform_.grid_tform.unit()));
         *unit_ /= tform_.cell_extents;

         const long_point lo(seg_[0]);
         *size_ -= lo;
         *shift_ -= lo;
      }

      inline long_point apply(segment_2 const & seg, double ratio)
      {
         if (seg[0] == seg[1])
            return long_point(seg[0]);
         else
         {
            long_number a(ratio);
            long_number b(1.0);
            b -= a;

            if (seg[0].x == seg[1].x)
            {
               long_number alpha(seg[0].y), beta(seg[1].y);
               b *= alpha;
               a *= beta;
               return long_point(long_number(seg[0].x), b + a);
            }
            else if (seg[0].y == seg[1].y)
            {
               long_number alpha(seg[0].x), beta(seg[1].x);
               b *= alpha;
               a *= beta;
               return long_point(b + a, long_number(seg[0].y));
            }
            else
            {
               long_point beg(seg[0]);
               long_point end(seg[1]);
               beg *= b;
               end *= a;
               beg += end;
               return beg;
            }
         }
      }

      template<class TForm>
      inline point_2i world_to_local(TForm const & tform, segment_2 const & seg, double ratio)
      {
         point_2 p = seg(ratio);
         point_2 pt = tform.world2local(p);
         point_2 frac;

         for (int axis = 0; axis <= 1; ++axis)
         {
            const double EPS = 1e-6;
            frac[axis] = pt[axis] - geom::floor(pt[axis]);
            if ((frac[axis] < EPS) || (frac[axis] + EPS > 1.0))
            {
               return exact_world_to_local(tform, p, apply(seg, ratio));
            }
         }

         point_2i res = geom::floor(pt);
         for (int axis = 0; axis <= 1; ++axis)      
            if ((res[axis] < 0) && (frac[axis] == 0))
               ++res[axis];

         return res;
      }

      // ToDo: due to changing long_number to double 
      inline std::auto_ptr<long_number> intersect_long(long_number const & range_size, long_number const & shift, long_number const & unit, int steps_num)
      {
         std::auto_ptr<long_number> x(new long_number(steps_num));
         *x *= unit;
         *x += shift;
         *x /= range_size;
         return x; 
      } 
         
      inline double intersect(double range_size, double shift, double unit, int steps_num)
      {
         return (steps_num * unit + shift) / range_size;
      }

      template<class TForm>
      inline char exact_closest_axis(long_data<TForm> & ld, point_2i const & steps_num)
      {
         long_point const & shift = ld.shift();
         long_point const & size = ld.size();
         long_point const & unit = ld.unit();

         std::auto_ptr<long_number> horizontal_ratio  = intersect_long(size.x, shift.x, unit.x, steps_num.x);
         std::auto_ptr<long_number> vertical_ratio    = intersect_long(size.y, shift.y, unit.y, steps_num.y);

         int c = geom::rasterization::details::cmp(*horizontal_ratio, *vertical_ratio);
         switch (c)
         {
         case -1:
            return 0;
         case 1:
            return 1;
         default:
            return 2;
         }
      }

      // returns 0 if closest axis horizontal
      //         1 if closest axis vertical
      //         2 if closest -- corner
      template<class TForm>
      inline char closest_axis(point_2 const & ratio, long_data<TForm> & ld, point_2i const & steps_num)
      {
         const double EPS = 1e-9;

         if (ratio.x < ratio.y)
         {
            if (ratio.y < ratio.x + EPS)
               return exact_closest_axis(ld, steps_num);
            else
               return 0;
         }
         else 
         {
            if (ratio.x < ratio.y + EPS)
               return exact_closest_axis(ld, steps_num);
            else
               return 1;
         }
      }
   }

   template<class TForm = aa_transform>
   struct state : boost::noncopyable
   {
      struct side_t
      {
         enum type_t { horizontal = 0, vertical = 1, begin, end };

         int idx;
         type_t axis;

         side_t(type_t axis, int idx = 0)
            : idx(idx)
            , axis(axis)
         {}
      };

      double in_ratio         () const { return ratio(in_point ); }
      double out_ratio        () const { return ratio(out_point); }
      double exact_in_ratio   () const { return exact_ratio(in_point ); }
      double exact_out_ratio  () const { return exact_ratio(out_point); }

      point_2i cell;
      side_t in_point, out_point;

      state(point_2i const & cell, double beg_ratio, double end_ratio)
         : cell(cell)
         , in_point(side_t::begin)
         , out_point(side_t::end)
         , beg_ratio_(beg_ratio)
         , end_ratio_(end_ratio)
      {}

      state(geom::point_2i const & cell, side_t const & in_point, side_t const & out_point,
            geom::segment_2 const & rasterized_seg, TForm const & tform,
            double beg_ratio, double end_ratio, std::shared_ptr<details::long_data<TForm>> ld)
         : cell(cell)
         , in_point(in_point)
         , out_point(out_point)
         , beg_ratio_(beg_ratio)
         , end_ratio_(end_ratio)
         , unit_(tform.unit())
         , size_(rasterized_seg[1] - rasterized_seg[0])
         , shift_(tform.origin() - rasterized_seg[0])
         , ld_(ld)
      {}

   private:
      geom::point_2 size_, shift_, unit_; // for ratio calculation
      double beg_ratio_, end_ratio_;
      std::shared_ptr<details::long_data<TForm>> ld_;

      double ratio(side_t const & side) const;
      double exact_ratio(side_t const side) const;
   };

   template<class TForm>
   double state<TForm>::ratio(side_t const & side) const
   {
      switch (side.axis)
      {
      case side_t::begin:
         return beg_ratio_;
      case side_t::end:
         return end_ratio_;
      default:
         {
            size_t axis = 1 - side.axis;
            return details::intersect(size_[axis], shift_[axis], unit_[axis], side.idx);
         }
      }
   }

   template<class TForm>
   double state<TForm>::exact_ratio(side_t const side) const
   {
      switch (side.axis)
      {
      case side_t::begin:
         return beg_ratio_;
      case side_t::end:
         return end_ratio_;
      default:
         {
            Assert(ld_.get());

            size_t axis = 1 - side.axis;            
            return details::intersect(ld_->size()[axis], ld_->shift()[axis], ld_->unit()[axis], side.idx)->to_double();
         }
      }
   }
}

template<class TForm, class Processor>
bool rasterize_segment(TForm const & tform, geom::segment_2 const & seg, Processor & proc,
                       double beg_ratio = 0.0, double end_ratio = 1.0)
{
   Assert(beg_ratio <= end_ratio);

   using segment_rasterization::details::world_to_local;

   const point_2i beg_idx = world_to_local(tform, seg, beg_ratio);
   const point_2i end_idx = world_to_local(tform, seg, end_ratio);

   typedef segment_rasterization::details::long_data<TForm> long_data_t;
   typedef segment_rasterization::state<TForm>              state_t;

   if (beg_idx == end_idx) // singular
   {
      return proc(state_t(beg_idx, beg_ratio, end_ratio));
   }
   else 
   {
      std::shared_ptr<long_data_t> ld(new long_data_t(seg, tform));
      state_t st(
            beg_idx,
            typename state_t::side_t(state_t::side_t::begin),
            typename state_t::side_t(state_t::side_t::begin),
            seg,
            tform,
            beg_ratio,
            end_ratio,
            ld);

      if ((beg_idx.x == end_idx.x) || (beg_idx.y == end_idx.y)) // horizontal or vertical
      {
         auto axis = (beg_idx.y == end_idx.y) ? state_t::side_t::horizontal : state_t::side_t::vertical;
         const int delta = geom::sign(end_idx[axis] - beg_idx[axis]);
         const int shift = (1 - delta) / 2;

         for (; st.cell[axis] != end_idx[axis]; st.cell[axis] += delta)
         {
            st.in_point = st.out_point;
            st.out_point.axis = typename state_t::side_t::type_t(1 - axis);
            st.out_point.idx = st.cell[axis] + shift + delta;

            if (proc(st))
               return true;
         }
      }
      else // common
      {
         point_2 const & origin  = tform.origin();
         point_2 const & unit = tform.unit();

         const point_2 size = seg[1] - seg[0];
         const point_2 shift = origin - seg[0];

         const point_2i delta(geom::sign(end_idx.x - beg_idx.x), geom::sign(end_idx.y - beg_idx.y));
         const point_2i idx_shift = (point_2i(1, 1) - delta) / 2;

         point_2 ratio;
         for (int axis_idx = 0; axis_idx <= 1; ++axis_idx)
         {
            ratio[axis_idx] = segment_rasterization::details::intersect( size[axis_idx], shift[axis_idx], unit[axis_idx], 
                                                                         st.cell[axis_idx] + idx_shift[axis_idx] + delta[axis_idx]);
         }

         while ((st.cell.x != end_idx.x) && (st.cell.y != end_idx.y))
         {
            st.in_point = st.out_point;            
            char axis = closest_axis(ratio, *ld, st.cell + idx_shift + delta);
            if (axis == 2)
            {
               st.out_point.idx = st.cell.x + idx_shift.x + delta.x;
               st.out_point.axis = state_t::side_t::vertical;
               if (proc(st))
                  return true;

               if ((delta.x > 0) != (delta.y > 0))
               {
                  st.in_point = st.out_point;
                  if (delta.x > 0)
                  {
                     ++st.cell.x;
                     if (proc(st))
                        return true;
                     --st.cell.y;
                  }
                  else
                  {
                     ++st.cell.y;
                     if (proc(st))
                        return true;
                     --st.cell.x;
                  }
               }           
               else
                  st.cell += delta;

               ratio.x = segment_rasterization::details::intersect(  size.x, shift.x, unit.x, 
                                                                     st.cell.x + idx_shift.x + delta.x);
               ratio.y = segment_rasterization::details::intersect(  size.y, shift.y, unit.y,
                                                                     st.cell.y + idx_shift.y + delta.y);
            }
            else
            {
               st.out_point.idx = st.cell[axis] + idx_shift[axis] + delta[axis];
               st.out_point.axis = typename state_t::side_t::type_t(1 - axis);

               if (proc(st))
                  return true;

               st.cell[axis] += delta[axis];

               ratio[axis] = segment_rasterization::details::intersect( size[axis], shift[axis], unit[axis], 
                                                                        st.cell[axis] + idx_shift[axis] + delta[axis]);
            }
         }

         auto axis = (st.cell.y == end_idx.y) ? state_t::side_t::horizontal : state_t::side_t::vertical;
         for (; st.cell[axis] != end_idx[axis]; st.cell[axis] += delta[axis])
         {
            st.in_point = st.out_point;
            st.out_point.axis = typename state_t::side_t::type_t(1 - axis);
            st.out_point.idx = st.cell[axis] + idx_shift[axis] + delta[axis];

            if (proc(st))
               return true;
         }
      }

      st.in_point = st.out_point;
      st.out_point.axis = state_t::side_t::end;
      return proc(st);
   }
}

}
