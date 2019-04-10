/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "cycle_val_fwd.h"

#include "geometry/primitives/range_fwd.h"

#include "logger/assert.hpp"
#include "common/no_deduce.h"
#include <geometry/common.h>

#define MAX_RANGE(S1, S2) range_t< MAX_TYPE(S1, S2) >

namespace geom
{
    //
#pragma pack( push, 1 )

    template <class value>
    struct range_t<cycle_value<value> >
    {
        typedef typename cycle_value<value>::scalar_type scalar_type;

        range_t() ;

        range_t( cycle_value<value> t1, cycle_value<value> t2 ) ;

        explicit range_t ( cycle_value<value> t ) ;

        template < class T > friend struct range_t;

        template <class Scalar>
        range_t( range_t<Scalar> const& r );

        cycle_value<value>        lo            () const ;
        cycle_value<value>        hi            () const ;

        scalar_type size          () const ;
        cycle_value<value>          center        () const ;

        bool     empty         () const ;
        bool     full          () const ;

        bool     contains      ( cycle_value<value> t ) const ;
        bool     contains      ( range_t const& r ) const ;

        scalar_type               closest       (cycle_value<value> t) const ;
        cycle_value<value>        closest_point ( cycle_value<value> t ) const ;
        cycle_value<value>        closest_point ( range_t const& r ) const ;

        cycle_value<value>        furthest_point( cycle_value<value> t ) const ;
        cycle_value<value>        furthest_point( range_t const& r ) const ;

        range_t& inflate       ( cycle_value<value> t ) ;
        range_t& offset        ( cycle_value<value> t ) ;
        range_t& unite         ( cycle_value<value> t ) ;

        range_t& operator += (cycle_value<value> t) ;
        range_t& operator -= (cycle_value<value> t) ;
        range_t& operator |= (cycle_value<value> t) ;
        range_t& operator |= (range_t const& r) ;
        range_t& operator &= (cycle_value<value> t) ;

        template <class Scalar>
        cycle_value<value> operator()( Scalar t ) const ;

        scalar_type operator()(cycle_value<value> const& other) const ;

    /*private:*/
        boost::optional<cycle_value<value>> lo_, hi_ ;
    };

#pragma pack( pop )

    template <class value> range_t<cycle_value<value>> range_by_size       ( cycle_value<value> lo, cycle_value<value> size ) ;

    template <class S1, class S2> bool             has_intersection ( range_t<S1> const& r1, range_t<S2> const& r2 );

    template <class S1, class S2> MAX_TYPE(S1,S2)  distance    ( range_t<S1> const& r, S2 t ) ;
    template <class S1, class S2> MAX_TYPE(S1,S2)  distance    ( S1 t, range_t<S2> const& r ) ;
    template <class S1, class S2> MAX_TYPE(S1,S2)  distance    ( range_t<S1> const& r1, range_t<S2> const& r2 );

    template <class value> cycle_value<value>                           rand        ( range_t<cycle_value<value>> const& r);

    template <class value>            bool             eq          ( range_t<cycle_value<value>> const& r1, range_t<cycle_value<value>> const& r2, NO_DEDUCE(cycle_value<value>) eps = epsilon<cycle_value<value>>() );
    template <class value>            bool             operator == ( range_t<cycle_value<value>> const& r1, range_t<cycle_value<value>> const& r2 );
    template <class value>            bool             operator != ( range_t<cycle_value<value>> const& r1, range_t<cycle_value<value>> const& r2 );

    template <class range>  range_t<cycle_value<range>> make_full_range();   

}

// implementation
namespace geom
{
    // ----------------------------------------------------------------------------------------------------- range_t
    template <class value>
    __forceinline range_t<cycle_value<value>> range_by_size ( cycle_value<value> lo, cycle_value<value> size )
    {
        Assert ( size >= 0 ) ;
        return range_t<cycle_value<value>> ( lo, lo + size ) ;
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>::range_t()
    {}

    template <class value>
    __forceinline range_t<cycle_value<value>>::range_t( cycle_value<value> t1, cycle_value<value> t2 )
        : lo_( t1 )
        , hi_( t2 )
    {}

    template <class value>
    __forceinline range_t<cycle_value<value>>::range_t( cycle_value<value> t )
        : lo_( t )
        , hi_( t )
    {}

    template <class value>
    template <class other_value>
    __forceinline range_t<cycle_value<value>>::range_t( range_t<other_value> const & other )
        : lo_(in_place(other.lo_))
        , hi_(in_place(other.hi_))
    {}

    template <class value>
    __forceinline cycle_value<value> range_t<cycle_value<value>>::lo() const
    {
        Assert ( !empty() ) ;
        return *lo_ ;
    }

    template <class value>
    __forceinline cycle_value<value> range_t<cycle_value<value>>::hi() const
    {
        Assert ( !empty() ) ;
        return *hi_ ;
    }

    template <class value>
    __forceinline typename cycle_value<value>::scalar_type range_t<cycle_value<value>>::size () const
    {
        return empty () ? 0 : path(*lo_, *hi_) ;
    }

    template <class value>
    __forceinline cycle_value<value> range_t<cycle_value<value>>::center () const
    {
        Assert (!empty()) ;
        return *lo_ + size()/2 ;
    }

    template <class value>
    __forceinline bool range_t<cycle_value<value>>::empty() const
    {
        return !lo_ || !hi_;
    }

    template <class value>
    __forceinline bool range_t<cycle_value<value>>::full() const
    {
        return !empty() && (*lo_ == cycle_value<value>::min() && *hi_ == cycle_value<value>::max());
    }

    template <class value>
    __forceinline bool range_t<cycle_value<value>>::contains ( cycle_value<value> t ) const
    {
        if (empty())
            return false;
        return path(*lo_, t) <= path(*lo_, *hi_) ;
    }

    template <class value>
    __forceinline bool range_t<cycle_value<value>>::contains  ( range_t<cycle_value<value>> const& r ) const
    {
        Assert ( !r.empty() ) ;
        return contains(r.lo()) && contains(r.hi()) ;
    }

    template <class value>
    typename cycle_value<value>::scalar_type range_t<cycle_value<value>>::closest(cycle_value<value> t) const
    {
        return contains(t) ? distance(t, *lo_) / size() : (distance(t, *lo_) <= distance(t, *hi_) ? 0 : 1);
    }

    template <class value>
    __forceinline cycle_value<value> range_t<cycle_value<value>>::closest_point ( cycle_value<value> t ) const
    {
        Assert ( !empty() ) ;
        return contains(t) ? t : (distance(t, *lo_) <= distance(t, *hi_) ? *lo_ : *hi_);
    }

//     template <class value>
//     __forceinline cycle_value<value> range_t<cycle_value<value>>::closest_point ( range_t const& r ) const
//     {
//         Assert ( !empty() ) ;
//         Assert ( !r.empty() ) ;
//         return r.hi_ < lo_ ? lo_ : r.lo_ > hi_ ? hi_ : r.lo_ < lo_ ? lo_ : r.lo_ ;
//     }

    template <class value>
    __forceinline cycle_value<value> range_t<cycle_value<value>>::furthest_point ( cycle_value<value> t ) const
    {
        Assert ( !empty() ) ;
        return distance(t, *lo_) >= distance(t, hi_) ? *lo_ : *hi_;
    }

//     template <class value>
//     __forceinline cycle_value<value> range_t<cycle_value<value>>::furthest_point ( range_t const& r ) const
//     {
//         Assert ( !empty() ) ;
//         Assert ( !r.empty() ) ;
//         return r.lo_ - lo_ > hi_ - r.hi_ ? lo_ : hi_ ;
//         //        return r.lo_ < lo_ ? hi_ : r.hi_ > hi_ ? lo_ : r.lo_ - lo_ > hi_ - r.hi_ ? lo_ : hi_ ;  may be faster?
//     }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::inflate  ( cycle_value<value> t )
    {
        Assert ( !empty() ) ;

        scalar_type s = size();
        lo_ -= t ;
        hi_ += t ;

        if (t > 0 && size() < s)
        {
            lo_ = cycle_value<value>::min();
            hi_ = cycle_value<value>::max();
        }
        if (t < 0 && size() > s)
        {
            lo_ = boost::none;
            hi_ = boost::none;
        }
        return *this ;
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::offset  ( cycle_value<value> t )
    {
        Assert ( !empty() ) ;
        *lo_ += t ;
        *hi_ += t ;

        return *this ;
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::unite  ( cycle_value<value> t )
    {
        if ( empty() )
            lo_ = hi_ = t ;
        else
        {
            if (!contains(t))
            {
                if (*lo_ == *hi_)
                {
                    if (left_turn(*lo_, t))
                        hi_ = t;
                    else
                        lo_ = t;
                }
                else if (distance(*lo_, t) < distance(*hi_, t))
                    lo_ = t;
                else
                    hi_ = t;

            }
        }
        return *this ;
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::operator += ( cycle_value<value> t )
    {
        return offset(t);
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::operator -= ( cycle_value<value> t )
    {
        return offset(-t);
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::operator |= ( cycle_value<value> t )
    {
        return unite(t);
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::operator |= ( range_t<cycle_value<value>> const& r )
    {
        if ( empty() )
        {
            lo_ = r.lo_ ;
            hi_ = r.hi_ ;
        }
        else if ( r.full() )
        {
            lo_ = cycle_value<value>::min();
            hi_ = cycle_value<value>::max();
        }
        else if ( !r.empty() )
        {
            if (*lo_ == r.hi() && *hi_ == r.lo())
            {
                lo_ = cycle_value<value>::min();
                hi_ = cycle_value<value>::max();
            }
            else
            {
                if (!left_turn(*lo_, r.lo())) // r.lo_ on the right
                    lo_ = r.lo();
                hi_ = *lo_ + max(path(*lo_, *hi_), path(*lo_, r.hi()));
            }
        }
        return *this;
    }

    template <class value>
    __forceinline range_t<cycle_value<value>>& range_t<cycle_value<value>>::operator &= ( cycle_value<value> t )
    {
        if ( contains(t) )
            lo_ = hi_ = t ;
        else
            *this = range_t () ;
        return *this;
    }

    template <class value> template <class S2>
    __forceinline cycle_value<value> range_t<cycle_value<value>>::operator() ( S2 t ) const
    {
        return *lo_ + t * path(*lo_, *hi_);
    }

    template <class value>
    typename cycle_value<value>::scalar_type range_t<cycle_value<value>>::operator()(cycle_value<value> const& other) const
    {
        if (contains(other))
            return other.size() / size();

        return distance(other, *lo_) <= distance(other, *hi_) ? 0. : 1.;
    }

    template <class value>
    __forceinline bool eq ( range_t<cycle_value<value>> const& r1, range_t<cycle_value<value>> const& r2, NO_DEDUCE(cycle_value<value>) eps )
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

    template <class value>
    __forceinline bool operator == ( range_t<cycle_value<value>> const& r1, range_t<cycle_value<value>> const& r2 )
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

    template <class value>
    __forceinline bool operator != ( range_t<cycle_value<value>> const& r1, range_t<cycle_value<value>> const& r2 )
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

    template <class range>  
        range_t<cycle_value<range>> make_full_range()
    {
        return range_t<cycle_value<range>>(range::min(), range::max());
    }
}

#undef MAX_RANGE

