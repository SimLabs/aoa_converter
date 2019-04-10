/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "range_fwd.h"

#include "logger/assert.hpp"
#include "common/no_deduce.h"
#include <geometry/common.h>

#define MAX_RANGE(S1, S2) range_t< MAX_TYPE(S1, S2) >

namespace geom
{
   //
   #pragma pack( push, 1 )

   template <class S>
      struct range_t
      {
         typedef S scalar_t;

         range_t() ;

         range_t( S t1, S t2 ) ;

         explicit range_t ( S t ) ;

         template < class T > friend struct range_t;

         template <class Scalar>
            range_t( range_t<Scalar> const& r );

         S        lo            () const ;
         S        hi            () const ;

         S        size          () const ;
         S        center        () const ;

         bool     empty         () const ;

         bool     contains      ( S t ) const ;
         bool     contains      ( range_t const& r ) const ;

         bool     contains      ( S t, S eps ) const ;
         bool     contains      ( range_t const& r, S eps ) const ;

         S        closest_point ( S t ) const ;
         S        closest_point ( range_t const& r ) const ;

         S        furthest_point( S t ) const ;
         S        furthest_point( range_t const& r ) const ;

         range_t& inflate       ( S t ) ;
         range_t& offset        ( S t ) ;
         range_t& unite         ( S t ) ;

         range_t& operator *= (S t) ;
         range_t& operator /= (S t) ;
         range_t& operator += (S t) ;
         range_t& operator -= (S t) ;
         range_t& operator |= (S t) ;
         range_t& operator |= (range_t const& r) ;
         range_t& operator &= (S t) ;
         range_t& operator &= (range_t const& r) ;

         template <class Scalar>
            scalar_t operator()( Scalar t ) const ;

         template <class Scalar>
            scalar_t map(Scalar t) const;

      /*private :*/
         S lo_, hi_ ;
      };

   #pragma pack( pop )

   template <class S> range_t<S> range_by_size       ( S lo, S size ) ;

   template <class S> range_t<S> halfopened_by_closed( range_t<S> const& range );
   template <class S> range_t<S> closed_by_halfopened( range_t<S> const& range );

   template <class S1, class S2> bool             has_intersection ( range_t<S1> const& r1, range_t<S2> const& r2 );

   template <class S1, class S2> MAX_TYPE(S1,S2)  distance    ( range_t<S1> const& r, S2 t ) ;
   template <class S1, class S2> MAX_TYPE(S1,S2)  distance    ( S1 t, range_t<S2> const& r ) ;
   template <class S1, class S2> MAX_TYPE(S1,S2)  distance    ( range_t<S1> const& r1, range_t<S2> const& r2 );

   template <class S> S                           rand        ( range_t<S> const& r);

   template <class S1, class S2> MAX_RANGE(S1,S2) operator +  ( range_t<S1> const& r, S2 t );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator +  ( S1 t, range_t<S2> const& r );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator -  ( range_t<S1> const& r, S2 t );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator *  ( range_t<S1> const& r, S2 t );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator *  ( S1 t, range_t<S2> const& r );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator /  ( range_t<S1> const& r, S2 t );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator |  ( range_t<S1> const& r, S2 t );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator |  ( S1 t, range_t<S2> const& r );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator &  ( range_t<S1> const& r, S2 t );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator &  ( S1 t, range_t<S2> const& r );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator |  ( range_t<S1> const& r1, range_t<S2> const& r2 );
   template <class S1, class S2> MAX_RANGE(S1,S2) operator &  ( range_t<S1> const& r1, range_t<S2> const& r2 );

   template <class S>            bool             eq          ( range_t<S> const& r1, range_t<S> const& r2, NO_DEDUCE(S) eps = epsilon<S>() );
   template <class S>            bool             operator == ( range_t<S> const& r1, range_t<S> const& r2 );
   template <class S>            bool             operator != ( range_t<S> const& r1, range_t<S> const& r2 );

}

// implementation
namespace geom
{
   // ----------------------------------------------------------------------------------------------------- range_t
   template <class S>
      __forceinline range_t<S> range_by_size ( S lo, S size )
      {
         Assert ( size >= 0 ) ;
         return range_t<S> ( lo, lo + size ) ;
      }

   template<class S>
      __forceinline range_t<S> halfopened_by_closed( range_t<S> const& range )
   {
      BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

      Assert ( !range.empty() ) ;
      return range_t<S>(range.lo(), range.hi()+1);
   }

   template<class S>
      __forceinline range_t<S> closed_by_halfopened( range_t<S> const& range )
   {
      BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

      Assert ( range.size() >= 1 ) ;
      return range_t<S>(range.lo(), range.hi()-1);
   }

   template <class S>
      __forceinline range_t<S>::range_t()
         : lo_(1)
         , hi_(0)
      {}

   template <class S>
      __forceinline range_t<S>::range_t( S t1, S t2 )
         : lo_( min(t1,t2) )
         , hi_( max(t1,t2) )
      {}

   template <class S>
      __forceinline range_t<S>::range_t( S t )
         : lo_ ( t )
         , hi_ ( t )
      {}

   template <class S1> template <class S2>
      __forceinline range_t<S1>::range_t( range_t<S2> const & other )
         : lo_( (S1) other.lo_ )
         , hi_( (S1) other.hi_ )
      {}

   template <class S>
      __forceinline S range_t<S>::lo() const
      {
         Assert ( !empty() ) ;
         return lo_ ;
      }

   template <class S>
      __forceinline S range_t<S>::hi() const
      {
         Assert ( !empty() ) ;
         return hi_ ;
      }

   template <class S>
      __forceinline S range_t<S>::size () const
      {
         return empty () ? 0 : hi_ - lo_ ;
      }

   template <class S>
      __forceinline S range_t<S>::center () const
      {
         Assert ( ! empty() ) ;
         return (lo_ + hi_) / 2 ;
      }

   template <class S>
      __forceinline bool range_t<S>::empty() const
      {
         return lo_ == 1 && hi_ == 0 ;
      }

   template <class S>
      __forceinline bool range_t<S>::contains ( S t ) const
      {
         return lo_ <= t && t <= hi_ ;
      }

   template <class S>
      __forceinline bool range_t<S>::contains  ( range_t<S> const& r ) const
      {
         Assert ( !r.empty() ) ;
         return lo_ <= r.lo_ && r.hi_ <= hi_ ;
      }

   template <class S>
      __forceinline bool range_t<S>::contains ( S t, S eps ) const
      {
         return le(lo_, t, eps) && le(t, hi_, eps);
      }

   template <class S>
      __forceinline bool range_t<S>::contains  ( range_t<S> const& r, S eps ) const
      {
         Assert ( !r.empty() ) ;
         return le(lo_, r.lo_, eps) && le(r.hi_, hi_, eps);
      }

   template <class S>
      __forceinline S range_t<S>::closest_point ( S t ) const
      {
        Assert ( !empty() ) ;
        return t < lo_ ? lo_ : t > hi_ ? hi_ : t ;
      }

   template <class S>
      __forceinline S range_t<S>::closest_point ( range_t const& r ) const
      {
        Assert ( !empty() ) ;
        Assert ( !r.empty() ) ;
        return r.hi_ < lo_ ? lo_ : r.lo_ > hi_ ? hi_ : r.lo_ < lo_ ? lo_ : r.lo_ ;
      }

   template <class S>
      __forceinline S range_t<S>::furthest_point ( S t ) const
      {
        Assert ( !empty() ) ;
        return t - lo_ > hi_ - t ? lo_ : hi_ ;
//        return t < lo_ ? hi_ : t > hi_ ? lo_ : t - lo_ > hi_ - t ? lo_ : hi_ ;   may be faster?
      }

   template <class S>
      __forceinline S range_t<S>::furthest_point ( range_t const& r ) const
      {
        Assert ( !empty() ) ;
        Assert ( !r.empty() ) ;
        return r.lo_ - lo_ > hi_ - r.hi_ ? lo_ : hi_ ;
//        return r.lo_ < lo_ ? hi_ : r.hi_ > hi_ ? lo_ : r.lo_ - lo_ > hi_ - r.hi_ ? lo_ : hi_ ;  may be faster?
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::inflate  ( S t )
      {
         Assert ( !empty() ) ;
         lo_ -= t ;
         hi_ += t ;
         if ( lo_ > hi_ )
            *this = range_t() ;

         return *this ;
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::offset  ( S t )
      {
         Assert ( !empty() ) ;
         lo_ += t ;
         hi_ += t ;

         return *this ;
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::unite  ( S t )
      {
         if ( empty() )
            lo_ = hi_ = t ;
         else
         {
            lo_ = min(lo_, t ) ;
            hi_ = max(hi_, t ) ;
         }
         return *this ;
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator += ( S t )
      {
         return offset(t);
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator -= ( S t )
      {
         return offset(-t);
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator *= ( S t )
      {
         Assert ( !empty() ) ;
         Assert ( t >= 0 ) ;
         lo_ *= t ;
         hi_ *= t ;
         return *this ;
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator /= ( S t )
      {
         Assert ( !empty() ) ;
         Assert ( t > 0 ) ;
         lo_ /= t ;
         hi_ /= t ;
         return *this;
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator |= ( S t )
      {
         return unite(t);
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator |= ( range_t<S> const& r )
      {
         if ( empty() )
         {
            lo_ = r.lo_ ;
            hi_ = r.hi_ ;
         }
         else if ( !r.empty() )
         {
            lo_ = min(lo_, r.lo_ ) ;
            hi_ = max(hi_, r.hi_ ) ;
         }
         return *this;
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator &= ( S t )
      {
         if ( lo_ <= t && t <= hi_ )
            lo_ = hi_ = t ;
         else
            *this = range_t () ;
         return *this;
      }

   template <class S>
      __forceinline range_t<S>& range_t<S>::operator &= ( range_t<S> const& r )
      {
         if ( !empty() && !r.empty() )
         {
            lo_ = max(lo_, r.lo_) ;
            hi_ = min(hi_, r.hi_) ;
            if ( lo_ > hi_ )
               *this = range_t () ;
         }
         else
            *this = range_t () ;
         return *this;
      }

   template <class S1> template <class S2>
      __forceinline S1 range_t<S1>::operator() ( S2 t ) const
      {
         return (S1)((1 - t) * lo_ + t * hi_);
      }

   template <class S1> template <class S2>
      __forceinline S1 range_t<S1>::map(S2 x) const
      {
          Assert(!empty());
          return (S1)((x - lo_) / (hi_ - lo_));
      }


   // ---------------------------------------------------------------------------------------------- operations
   template <class S1, class S2>
      __forceinline bool has_intersection ( range_t<S1> const& r1, range_t<S2> const& r2 )
      {
         if ( r1.empty () || r2.empty () )
            return false ;

         return r1.hi() >= r2.lo() && r1.lo() <= r2.hi() ;
      }

   template <class S1, class S2>
      __forceinline MAX_TYPE(S1,S2) distance ( range_t<S1> const& r, S2 t )
      {
         Assert ( !r.empty() ) ;
         return t < r.lo() ? r.lo() - t : t > r.hi() ? t - r.hi() : 0;
      }

   template <class S1, class S2>
      __forceinline MAX_TYPE(S1,S2) distance ( S1 t, range_t<S2> const& r )
      {
         return distance(r,t) ;
      }

   template <class S1, class S2>
      __forceinline MAX_TYPE(S1,S2) distance ( range_t<S1> const& r1, range_t<S2> const& r2 )
      {
         Assert ( !r1.empty() ) ;
         Assert ( !r2.empty() ) ;
         return r1.hi() < r2.lo() ? r2.lo() - r1.hi() : r2.hi() < r1.lo() ? r1.lo() - r2.hi() : 0;
      }

   template <class S>
      __forceinline S rand ( range_t<S> const& r)
      {
         Assert ( !r.empty() ) ;
         return r.lo() + rand(r.size()) ;
      }

      template< class Scalar >
      __forceinline bool is_finite( range_t<Scalar> const & r )
      {
          if (!is_finite(r.lo()) || !is_finite(r.hi()))
              return false;

          return true;
      }

   // --------------------------------------------------------------------------------------------- operators
   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator + ( range_t<S1> const& r, S2 t )
      {
         return MAX_RANGE(S1,S2) (r) += t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator + ( S1 t, range_t<S2> const& r )
      {
         return MAX_RANGE(S1,S2) (r) += t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator - ( range_t<S1> const& r, S2 t )
      {
         return MAX_RANGE(S1,S2) (r) -= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator * ( range_t<S1> const& r, S2 t )
      {
         return MAX_RANGE(S1,S2) (r) *= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator * ( S1 t, range_t<S2> const& r )
      {
         return MAX_RANGE(S1,S2) (r) *= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator / ( range_t<S1> const& r, S2 t )
      {
         return MAX_RANGE(S1,S2) (r) /= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator | ( range_t<S1> const& r, S2 t )
      {
         return MAX_RANGE(S1,S2) (r) |= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator | ( S1 t, range_t<S2> const& r)
      {
         return MAX_RANGE(S1,S2) (r) |= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator & ( range_t<S1> const& r, S2 t )
      {
         return MAX_RANGE(S1,S2) (r) &= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator & ( S1 t, range_t<S2> const& r)
      {
         return MAX_RANGE(S1,S2) (r) &= t ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator & ( range_t<S1> const& r1, range_t<S2> const& r2)
      {
         return MAX_RANGE(S1,S2) (r1) &= r2 ;
      }

   template <class S1, class S2>
      __forceinline MAX_RANGE(S1,S2) operator | ( range_t<S1> const& r1, range_t<S2> const& r2)
      {
         return MAX_RANGE(S1,S2) (r1) |= r2 ;
      }

   // --------------------------------------------------------------------------------------------------------- comparasion
   template <class S>
      __forceinline bool eq ( range_t<S> const& r1, range_t<S> const& r2, NO_DEDUCE(S) eps )
      {
         if (r1.empty())
         {
            if (r2.empty())
               return true;
            return false;
         }
         if (r2.empty())
            return false;

          return eq( r1.lo(), r2.lo(), eps ) && eq( r1.hi(), r2.hi(), eps );
      }

   template <class S>
      __forceinline bool operator == ( range_t<S> const& r1, range_t<S> const& r2 )
      {
         if (r1.empty())
         {
            if (r2.empty())
               return true;
            return false;
         }
         if (r2.empty())
            return false;

         return r1.lo() == r2.lo() && r1.hi() == r2.hi() ;
      }

   template <class S>
      __forceinline bool operator != ( range_t<S> const& r1, range_t<S> const& r2 )
      {
         if (r1.empty())
         {
            if (r2.empty())
               return false;
            return true;
         }
         if (r2.empty())
            return true;

         return r1.lo() != r2.lo() || r1.hi() != r2.hi() ;
      }
}

#undef MAX_RANGE

