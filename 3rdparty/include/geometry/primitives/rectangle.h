/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <common/no_deduce.h>
#include <common/safe_bool.h>
#include <geometry/common.h>

#include "range.h"
#include "point.h"
#include "segment.h"

#include "rectangle_fwd.h"
#include "sphere_fwd.h"

#define MAX_RECTANGLE(S1, S2, D)   rectangle_t< MAX_TYPE(S1, S2), D>
#define MAX_POINT(S1, S2, D)       point_t< MAX_TYPE(S1, S2), D>
#define MAX_RANGE(S1, S2)          range_t< MAX_TYPE(S1, S2) >

namespace geom
{
   template <class S, size_t D>
      struct rectangle_t
      {
         typedef  S              scalar_t;
         typedef  range_t<S>     range_t ;
         typedef  point_t<S,D>   point_t ;
         typedef  segment_t<S,D> segment_t ;
         enum  { dimension = D };

         rectangle_t() ;
         rectangle_t( point_t const& p1, point_t const& p2 ) ;

         explicit rectangle_t( point_t const& p ) ;

         template <class Scalar>
            rectangle_t( rectangle_t<Scalar,D> const& r ) ;

         point_t        lo            () const ;
         point_t        hi            () const ;

         point_t        size          () const ;
         point_t        center        () const ;

         bool           empty         () const ;

         bool           contains      ( point_t     const& p ) const ;
         bool           contains      ( rectangle_t const& r ) const ;
         bool           contains      ( segment_t   const& s ) const ;

         bool           contains      ( point_t     const& p, S eps ) const ;
         bool           contains      ( rectangle_t const& r, S eps ) const ;
         bool           contains      ( segment_t   const& s, S eps ) const ;

         point_t        closest_point ( point_t     const& p ) const ;
         point_t        closest_point ( rectangle_t const& r ) const ;

         point_t        furthest_point( point_t     const& p ) const ;
         point_t        furthest_point( rectangle_t const& p ) const ;

         rectangle_t&   offset        ( point_t const& p ) ;
         rectangle_t&   offset        ( S t ) ;

         rectangle_t&   inflate       ( point_t const& p ) ;
         rectangle_t&   inflate       ( S t ) ;

         range_t      & operator [] ( size_t i ) ;
         range_t const& operator [] ( size_t i ) const ;

         template <class Scalar>
            point_t     operator()( geom::point_t<Scalar,D> const& t ) const ;

         template <class Scalar>
            point_t map(geom::point_t<Scalar, D> const& t) const;

         struct iterator
         {
            iterator (rectangle_t const &r) ;

            point_t const&  operator *    () const ;
            point_t const * operator ->   () const ;
            iterator&       operator ++   () ;
         };

         template < typename FwdIter >
            static rectangle_t bounding ( FwdIter p, FwdIter q ) ;
      } ;

   template<class S, size_t D> rectangle_t<S,D> rectangle_by_size   ( point_t<S,D> const& lo, point_t<S,D> const& size );
   template<class S, size_t D> rectangle_t<S,D> rectangle_by_sphere ( point_t<S,D> const& center, S radius );
   template<class S, size_t D> rectangle_t<S,D> rectangle_by_extents( point_t<S,D> const& lo, point_t<S,D> const& extents );
   template<class S, size_t D> rectangle_t<S,D> rectangle_by_extents( point_t<S,D> const& extents );

   template<class S, size_t D> point_t<S,D>     extents_by_size( point_t<S,D> const& size );
   template<class S, size_t D> point_t<S,D>     size_by_extents( point_t<S,D> const& extents );

   template<class S, size_t D> rectangle_t<S,D> halfopened_by_closed( rectangle_t<S,D> const& rect );
   template<class S, size_t D> rectangle_t<S,D> closed_by_halfopened( rectangle_t<S,D> const& rect );


   // ---------------------------------------------------------------------------------------------------------
   template <class S, size_t D> rectangle_t<S,D>& operator += ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p );
   template <class S, size_t D> rectangle_t<S,D>& operator -= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p );
   template <class S, size_t D> rectangle_t<S,D>& operator *= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p );
   template <class S, size_t D> rectangle_t<S,D>& operator /= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p );
   template <class S, size_t D> rectangle_t<S,D>& operator *= ( rectangle_t<S,D>& r,  NO_DEDUCE(S) t );
   template <class S, size_t D> rectangle_t<S,D>& operator /= ( rectangle_t<S,D>& r,  NO_DEDUCE(S) t );
   template <class S, size_t D> rectangle_t<S,D>& operator |= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p );
   template <class S, size_t D> rectangle_t<S,D>& operator &= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p );
   template <class S, size_t D> rectangle_t<S,D>& operator |= ( rectangle_t<S,D>& r,  NO_DEDUCE2(segment_t<S,D>) const& s );
   template <class S, size_t D> rectangle_t<S,D>& operator |= ( rectangle_t<S,D>& r1, NO_DEDUCE2(rectangle_t<S,D>) const& r2 );
   template <class S, size_t D> rectangle_t<S,D>& operator &= ( rectangle_t<S,D>& r1, NO_DEDUCE2(rectangle_t<S,D>) const& r2 );

   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator +  ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator +  ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator -  ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator *  ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator *  ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator /  ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator *  ( rectangle_t<S1,D> const& r, S2 t );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator *  ( S1 t, rectangle_t<S2,D> const& r );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator /  ( rectangle_t<S1,D> const& r, S2 t );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator |  ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator |  ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator &  ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator &  ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator |  ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2 );
   template <class S1, class S2, size_t D> MAX_RECTANGLE(S1,S2,D) operator &  ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2 );

   template <class S1, class S2, size_t D> bool has_intersection ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2 ) ;

   template <class S1, class S2, size_t D> bool has_intersection ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p0, point_t<S2,D> const& p1 ) ;

   template <class S1, class S2, size_t D> MAX_TYPE(S1,S2)  distance    ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p ) ;
   template <class S1, class S2, size_t D> MAX_TYPE(S1,S2)  distance    ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r ) ;
   template <class S1, class S2, size_t D> MAX_TYPE(S1,S2)  distance    ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2 );

   template <class S1, class S2, size_t D> MAX_RANGE(S1,S2)  distance_range ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p ) ;
   template <class S1, class S2, size_t D> MAX_RANGE(S1,S2)  distance_range ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r ) ;

   template <class S, size_t D>            point_t<S,D>     rand        ( rectangle_t<S,D> const& r);

   template <class S, size_t D> bool eq			 (rectangle_t<S, D> const& r1, rectangle_t<S, D> const& r2, 
												  NO_DEDUCE(S) eps = epsilon<NO_DEDUCE(S)>());

   template <class S, size_t D> bool operator == ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2 );
   template <class S, size_t D> bool operator != ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2 );
}

// implementation
namespace geom
{
   namespace rectangle_details
   {
       template <class S, size_t D> struct iterator ;

       template <class S, size_t D>
          __forceinline point_t<S,D> corner ( rectangle_t<S, D> const & r, size_t i )
          {
             switch ( i )
             {
             case 0: return r.xy();
             case 1: return r.Xy();
             case 2: return r.XY();
             case 3: return r.xY();
             default: Assert( !"Invalid index" ); return r.xy();
             };
          }

       template <class S>
          __forceinline point_t<S,3> corner ( rectangle_t<S, 3> const & r, size_t i )
          {
             switch ( i )
             {
             case 0: return r.xyz();
             case 1: return r.Xyz();
             case 2: return r.xYz();
             case 3: return r.XYz();
             case 4: return r.xyZ();
             case 5: return r.XyZ();
             case 6: return r.xYZ();
             case 7: return r.XYZ();
             default: Assert( !"Invalid index" ); return r.xyz();
             };
          }

          template <class S, size_t D>
          __forceinline point_t<S,D> size ( rectangle_t<S,D> const& r )
          {
             point_t<S,D> ret ;
             for ( size_t n = 0; n != D; ++n )
                ret[n] = r[n].size();
             return ret ;
          }

       template <class S, size_t D>
          __forceinline point_t<S,D> center ( rectangle_t<S,D> const& r )
          {
             point_t<S,D> ret ;
             for ( size_t n = 0; n != D; ++n )
                ret[n] = r[n].center();
             return ret ;
          }

       template <class S, size_t D>
          __forceinline bool empty ( rectangle_t<S,D> const& r )
          {
             for ( size_t n = 0; n != D; ++n )
                if ( r[n].empty() )
                   return true ;
             return false ;
          }

       template <class S, size_t D>
          __forceinline bool contains ( rectangle_t<S,D> const& r, point_t<S,D> const& p )
          {
             for ( size_t n = 0; n != D; ++n )
                if ( !r[n].contains( p[n] ) )
                   return false ;
             return true ;
          }

       template <class S, size_t D>
          __forceinline bool contains ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2 )
          {
             for ( size_t n = 0; n != D; ++n )
                if ( !r1[n].contains( r2[n] ) )
                   return false ;
             return true ;
          }

       template <class S, size_t D>
          __forceinline bool contains ( rectangle_t<S,D> const& r, segment_t<S,D> const& s )
          {
             return r.contains(s.P0()) && r.contains(s.P1()) ;
          }

       template <class S, size_t D>
          __forceinline bool contains ( rectangle_t<S,D> const& r, point_t<S,D> const& p, S eps )
          {
             for ( size_t n = 0; n != D; ++n )
                if ( !r[n].contains( p[n], eps ) )
                   return false ;
             return true ;
          }


       template <class S, size_t D>
          __forceinline bool contains ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2, S eps )
          {
             for ( size_t n = 0; n != D; ++n )
                if ( !r1[n].contains( r2[n], eps ) )
                   return false ;
             return true ;
          }

       template <class S, size_t D>
          __forceinline bool contains ( rectangle_t<S,D> const& r, segment_t<S,D> const& s, S eps )
          {
             return r.contains(s.P0(), eps) && r.contains(s.P1(), eps) ;
          }

       template <class S, size_t D>
          __forceinline point_t<S,D> closest_point ( rectangle_t<S,D> const& r, point_t<S,D> const& p )
          {
             point_t<S,D> ret ;
             for ( size_t n = 0; n != D; ++n )
                ret[n] = r[n].closest_point ( p[n] ) ;
             return ret ;
          }

       template <class S, size_t D>
          __forceinline point_t<S,D> closest_point ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2 )
          {
             point_t<S,D> ret ;
             for ( size_t n = 0; n != D; ++n )
                ret[n] = r1[n].closest_point ( r2[n] ) ;
             return ret ;
          }

       template <class S, size_t D>
          __forceinline point_t<S,D> furthest_point ( rectangle_t<S,D> const& r, point_t<S,D> const& p )
          {
             point_t<S,D> ret ;
             for ( size_t n = 0; n != D; ++n )
                ret[n] = r[n].furthest_point ( p[n] ) ;
             return ret ;
          }

       template <class S, size_t D>
          __forceinline point_t<S,D> furthest_point ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2 )
          {
             point_t<S,D> ret ;
             for ( size_t n = 0; n != D; ++n )
                ret[n] = r1[n].furthest_point ( r2[n] ) ;
             return ret ;
          }

       template <class S, size_t D>
          __forceinline rectangle_t<S,D>& inflate ( rectangle_t<S,D>& r, point_t<S,D> const& p )
          {
             for ( size_t n = 0; n != D; ++n )
                r[n].inflate ( p[n] ) ;

             return r ;
          }

       template <class S, size_t D>
          __forceinline rectangle_t<S,D>& inflate ( rectangle_t<S,D>& r, S t )
          {
             for ( size_t n = 0; n != D; ++n )
                r[n].inflate ( t ) ;

             return r ;
          }

       template <class S, size_t D>
          __forceinline rectangle_t<S,D>& offset ( rectangle_t<S,D>& r, point_t<S,D> const& p )
          {
             for ( size_t n = 0; n != D; ++n )
                r[n].offset ( p[n] ) ;

             return r ;
          }

       template <class S, size_t D>
          __forceinline rectangle_t<S,D>& offset ( rectangle_t<S,D>& r, S t )
          {
             for ( size_t n = 0; n != D; ++n )
                r[n].offset ( t ) ;

             return r ;
          }

       template <class S, size_t D>
          __forceinline range_t<S>& at ( rectangle_t<S,D>& r, size_t n )
          {
             Assert (n < D) ;
             return (&r.x)[n] ;
          }


       template <class S, size_t D>
          __forceinline range_t<S> const& at ( rectangle_t<S,D> const& r, size_t n )
          {
             Assert (n < D) ;
             return (&r.x)[n] ;
          }

       template <class S1, class S2, size_t D>
           __forceinline point_t<S1,D> lerp ( rectangle_t<S1,D> const& r, point_t<S2,D> const& t )
           {
              return r.lo() + (r.size() & t) ;
           }

       template <class S1, class S2, size_t D>
           __forceinline point_t<S1,D> map ( rectangle_t<S1,D> const& r, point_t<S2,D> const& x )
           {
              return (x - r.lo()) / r.size();
           }

       template < class S, size_t D, typename FwdIter >
          __forceinline rectangle_t<S,D> bounding ( FwdIter p, FwdIter q )
          {
             rectangle_t<S,D> ret ;
             while (p != q)
                ret |= *p++;
             return ret ;
          }

       //
       template <class S, size_t D>
       struct iterator
       {
          __forceinline iterator (rectangle_t<S,D> const &r)
             : rect_  ( r )
             , valid_ ( !r.empty() )
          {
             if ( valid_ )
                pos_ = r.lo () ;
          }

          SAFE_BOOL_OPERATOR(valid_)

          __forceinline point_t<S,D> const&  operator * () const
          {
             Assert ( valid_ ) ;
             return pos_ ;
          }
          __forceinline point_t<S,D> const * operator -> () const
          {
             Assert ( valid_ ) ;
             return &pos_ ;
          }
          __forceinline iterator& operator ++ ()
          {
             Assert ( valid_ ) ;
             for ( size_t n = 0 ; n != D ; n ++ )
             {
                if ( ++ pos_[n] <= rect_[n].hi () )
                   return *this;

                pos_[n] = rect_[n].lo () ;
             }

             valid_ = false ;
             return *this;
          }
       private :
          rectangle_t<S,D> rect_ ;
          point_t<S,D>     pos_ ;
          bool             valid_ ;
       };
   } // } // namespace rectangle_details

   #pragma pack( push, 1 )

   //
   template <class S>
      struct rectangle_t<S, 2>
      {
         typedef  S                  scalar_t;
         typedef  geom::range_t<S>     range_t ;
         typedef  geom::point_t<S,2>   point_t ;
         typedef  geom::segment_t<S,2> segment_t ;
         enum  { dimension = 2 };

         typedef  rectangle_details::iterator<S,2> iterator ;

         range_t x;
         range_t y;


         __forceinline rectangle_t()
         {}

         __forceinline rectangle_t( range_t x, range_t y )
            : x ( x )
            , y ( y )
         {}

         __forceinline rectangle_t( point_t const& p1, point_t const& p2 )
            : x ( p1.x, p2.x )
            , y ( p1.y, p2.y )
         {}

         __forceinline explicit rectangle_t( point_t const& p )
            : x ( p.x )
            , y ( p.y )
         {}

         __forceinline rectangle_t( rectangle_t const& r )
               : x ( r.x )
               , y ( r.y )
            {}

         template <class Scalar>
            __forceinline rectangle_t( rectangle_t<Scalar,2> const& r )
               : x ( r.x )
               , y ( r.y )
            {}

         __forceinline point_t        lo            () const { return point_t(x.lo(), y.lo()) ; }
         __forceinline point_t        hi            () const { return point_t(x.hi(), y.hi()) ; }

         __forceinline point_t        xy            () const { return point_t(x.lo(), y.lo()) ; }
         __forceinline point_t        xY            () const { return point_t(x.lo(), y.hi()) ; }
         __forceinline point_t        Xy            () const { return point_t(x.hi(), y.lo()) ; }
         __forceinline point_t        XY            () const { return point_t(x.hi(), y.hi()) ; }

         __forceinline point_t        corner        ( size_t i ) const                    { return rectangle_details::corner(*this, i); }

         __forceinline point_t        size          () const                              { return rectangle_details::size(*this) ; }
         __forceinline point_t        center        () const                              { return rectangle_details::center(*this) ; }

         __forceinline bool           empty         () const                              { return rectangle_details::empty(*this) ; }

         __forceinline bool           contains      ( point_t const& p ) const            { return rectangle_details::contains(*this, p) ; }
         __forceinline bool           contains      ( rectangle_t const& r ) const        { return rectangle_details::contains(*this, r) ; }
         __forceinline bool           contains      ( segment_t   const& s ) const        { return rectangle_details::contains(*this, s) ; }

         __forceinline bool           contains      ( point_t const& p, S eps ) const     { return rectangle_details::contains(*this, p, eps) ; }
         __forceinline bool           contains      ( rectangle_t const& r, S eps ) const { return rectangle_details::contains(*this, r, eps) ; }
         __forceinline bool           contains      ( segment_t const& s, S eps ) const   { return rectangle_details::contains(*this, s, eps) ; }

         __forceinline point_t        closest_point ( point_t const& p ) const            { return rectangle_details::closest_point(*this, p) ; }
         __forceinline point_t        closest_point ( rectangle_t const& r ) const        { return rectangle_details::closest_point(*this, r) ; }

         __forceinline point_t        furthest_point( point_t const& p ) const            { return rectangle_details::furthest_point(*this, p) ; }
         __forceinline point_t        furthest_point( rectangle_t const& r ) const        { return rectangle_details::furthest_point(*this, r) ; }

         __forceinline rectangle_t&   offset        ( point_t const& p )                  { return rectangle_details::offset(*this, p) ; }
         __forceinline rectangle_t&   offset        ( S t )                               { return rectangle_details::offset(*this, t) ; }

         __forceinline rectangle_t&   inflate       ( point_t const& p )                  { return rectangle_details::inflate(*this, p) ; }
         __forceinline rectangle_t&   inflate       ( S t )                               { return rectangle_details::inflate(*this, t) ; }


         __forceinline range_t      & operator []   ( size_t i )                          { return rectangle_details::at(*this, i) ; }
         __forceinline range_t const& operator []   ( size_t i ) const                    { return rectangle_details::at(*this, i) ; }

         template <class Scalar>
            point_t        operator()( geom::point_t<Scalar,2> const& t ) const           { return rectangle_details::lerp(*this, t) ; }

         template <class Scalar>
            point_t        map( geom::point_t<Scalar,2> const& x ) const                 { return rectangle_details::map(*this, x) ; }


         template < typename FwdIter >
            __forceinline static rectangle_t bounding ( FwdIter p, FwdIter q )           { return rectangle_details::bounding<S,2,FwdIter>(p,q) ; }
      } ;

   //
   template <class S>
      struct rectangle_t<S, 3>
         : rectangle_t<S, 2>
      {
         typedef  S                  scalar_t;
         typedef  geom::range_t<S>     range_t ;
         typedef  geom::point_t<S,3>   point_t ;
         typedef  geom::segment_t<S,3> segment_t ;
         enum  { dimension = 3 };

         typedef  rectangle_details::iterator<S,3> iterator ;

         using rectangle_t<S, 2>::x;
         using rectangle_t<S, 2>::y;
         range_t z;

         rectangle_t()
         {}

         __forceinline rectangle_t( range_t x, range_t y, range_t z )
            : rectangle_t<S,2> ( x, y )
            , z ( z )
         {}

         __forceinline rectangle_t( rectangle_t<S,2> const& r, range_t z )
            : rectangle_t<S,2> ( r )
            , z ( z )
         {}

         __forceinline rectangle_t( point_t const& p1, point_t const& p2 )
            : rectangle_t<S,2> ( p1, p2 )
            , z ( p1.z, p2.z )
         {}

         __forceinline explicit rectangle_t( point_t const& p )
            : rectangle_t<S,2> ( p )
            , z ( p.z )
         {}

         __forceinline explicit rectangle_t( rectangle_t<S,2> const& r )
            : rectangle_t<S,2> ( r )
         {}

         __forceinline rectangle_t( rectangle_t const& r )
            : rectangle_t<S,2> ( r )
            , z ( r.z )
            {
            }

         template <class Scalar>
            __forceinline rectangle_t( rectangle_t<Scalar,3> const& r )
               : rectangle_t<S,2> ( r )
               , z ( r.z )
            {
            }

         template < typename FwdIter >
            __forceinline rectangle_t( FwdIter p, FwdIter q )
            {
                while (p != q)
                    *this |= *p++;
            }

         __forceinline point_t        corner        ( size_t i ) const { return rectangle_details::corner(*this, i); }

         __forceinline point_t        lo            () const { return point_t(x.lo(), y.lo(), z.lo()) ; }
         __forceinline point_t        hi            () const { return point_t(x.hi(), y.hi(), z.hi()) ; }

         __forceinline point_t        xyz           () const { return point_t(x.lo(), y.lo(), z.lo()) ; }
         __forceinline point_t        xYz           () const { return point_t(x.lo(), y.hi(), z.lo()) ; }
         __forceinline point_t        Xyz           () const { return point_t(x.hi(), y.lo(), z.lo()) ; }
         __forceinline point_t        XYz           () const { return point_t(x.hi(), y.hi(), z.lo()) ; }

         __forceinline point_t        xyZ           () const { return point_t(x.lo(), y.lo(), z.hi()) ; }
         __forceinline point_t        xYZ           () const { return point_t(x.lo(), y.hi(), z.hi()) ; }
         __forceinline point_t        XyZ           () const { return point_t(x.hi(), y.lo(), z.hi()) ; }
         __forceinline point_t        XYZ           () const { return point_t(x.hi(), y.hi(), z.hi()) ; }

         __forceinline point_t        size          () const                              { return rectangle_details::size(*this) ; }
         __forceinline point_t        center        () const                              { return rectangle_details::center(*this) ; }

         __forceinline bool           empty         () const                              { return rectangle_details::empty(*this) ; }

         __forceinline bool           contains      ( point_t const& p ) const            { return rectangle_details::contains(*this, p) ; }
         __forceinline bool           contains      ( rectangle_t const& r ) const        { return rectangle_details::contains(*this, r) ; }
         __forceinline bool           contains      ( segment_t const& s ) const          { return rectangle_details::contains(*this, s) ; }

         __forceinline bool           contains      ( point_t const& p, S eps ) const     { return rectangle_details::contains(*this, p, eps) ; }
         __forceinline bool           contains      ( rectangle_t const& r, S eps ) const { return rectangle_details::contains(*this, r, eps) ; }
         __forceinline bool           contains      ( segment_t const& s, S eps ) const   { return rectangle_details::contains(*this, s, eps) ; }

         __forceinline point_t        closest_point ( point_t const& p ) const            { return rectangle_details::closest_point(*this, p) ; }
         __forceinline point_t        closest_point ( rectangle_t const& r ) const        { return rectangle_details::closest_point(*this, r) ; }

         __forceinline point_t        furthest_point( point_t const& p ) const            { return rectangle_details::furthest_point(*this, p) ; }
         __forceinline point_t        furthest_point( rectangle_t const& r ) const        { return rectangle_details::furthest_point(*this, r) ; }

         __forceinline rectangle_t&   offset        ( point_t const& p )                  { return rectangle_details::offset(*this, p) ; }
         __forceinline rectangle_t&   offset        ( S t )                               { return rectangle_details::offset(*this, t) ; }

         __forceinline rectangle_t&   inflate       ( point_t const& p )                  { return rectangle_details::inflate(*this, p) ; }
         __forceinline rectangle_t&   inflate       ( S t )                               { return rectangle_details::inflate(*this, t) ; }

         __forceinline range_t      & operator []   ( size_t i )                          { return rectangle_details::at(*this, i) ; }
         __forceinline range_t const& operator []   ( size_t i ) const                    { return rectangle_details::at(*this, i) ; }

         template <class Scalar>
            point_t        operator()( geom::point_t<Scalar,3> const& t ) const           { return rectangle_details::lerp(*this, t) ; }

         template <class Scalar>
            point_t        map( geom::point_t<Scalar,3> const& x ) const                 { return rectangle_details::map(*this, x) ; }

         template < typename FwdIter >
            __forceinline static rectangle_t bounding ( FwdIter p, FwdIter q )           { return rectangle_details::bounding<S,3,FwdIter>(p,q) ; }

      } ;
   #pragma pack( pop )


   // ----------------------------------------------------------------------------------------------- creation
   template<class S, size_t D>
      __forceinline rectangle_t<S,D> rectangle_by_size ( point_t<S,D> const& lo, point_t<S,D> const& size )
      {
         return rectangle_t<S,D>(lo, lo + size) ;
      }

   template<class S, size_t D>
      __forceinline rectangle_t<S,D> rectangle_by_sphere ( point_t<S,D> const& center, S radius )
      {
         return rectangle_t<S,D>(center).inflate (radius) ;
      }

   template<class S, size_t D>
      __forceinline rectangle_t<S,D> rectangle_by_sphere ( sphere_t<S, D> const& sphere )
      {
         return rectangle_t<S,D>(sphere.center).inflate (sphere.radius) ;
      }

   template<class S, size_t D>
      __forceinline rectangle_t<S,D> rectangle_by_extents ( point_t<S,D> const& extents )
      {
         BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

         rectangle_t<S,D> ret ;
         for ( size_t n = 0 ; n != D ; n ++ )
            if ( extents[n] >= 1 )
               ret[n] = range_t<S>(0, extents[n] - 1) ;
         return ret ;
      }

   template<class S, size_t D>
      __forceinline rectangle_t<S,D> rectangle_by_extents ( point_t<S,D> const& lo, point_t<S,D> const& extents )
      {
         BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

         rectangle_t<S,D> ret ;
         for ( size_t n = 0 ; n != D ; n ++ )
            if ( extents[n] >= 1 )
               ret[n] = range_t<S>(lo[n], lo[n] + extents[n] - 1);
         return ret ;
      }

   template<class S, size_t D>
      __forceinline point_t<S,D> extents_by_size ( point_t<S,D> const& size )
      {
         BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

         point_t<S,D> ret ;
         for ( size_t n = 0 ; n != D ; n ++ )
            ret[n] = size[n] + 1 ;
         return ret ;
      }

   template<class S, size_t D>
      __forceinline point_t<S,D> size_by_extents ( point_t<S,D> const& extents )
      {
         BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

         point_t<S,D> ret ;
         for ( size_t n = 0 ; n != D ; n ++ )
            if ( extents[n] >= 1 )
               ret[n] = extents[n] - 1 ;
         return ret ;
      }

   template<class S, size_t D>
      __forceinline rectangle_t<S,D> halfopened_by_closed ( rectangle_t<S,D> const& rect )
      {
         BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

         rectangle_t<S,D> ret ;
         for ( size_t n = 0 ; n != D ; n ++ )
            ret[n] = halfopened_by_closed(rect[n]);
         return ret ;
      }

   template<class S, size_t D>
      __forceinline rectangle_t<S,D> closed_by_halfopened ( rectangle_t<S,D> const& rect )
      {
         BOOST_STATIC_ASSERT_MSG(meta::_is_integral<S>::value, "S_must_be_integral");

         rectangle_t<S,D> ret ;
         for ( size_t n = 0 ; n != D ; n ++ )
            ret[n] = closed_by_halfopened(rect[n]);
         return ret ;
      }

   // ----------------------------------------------------------------------------------------------- modify operators
   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator += ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] += p[n];
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator -= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] -= p[n];
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator *= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] *= p[n];
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator /= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] /= p[n];
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator *= ( rectangle_t<S,D>& r,  NO_DEDUCE(S) t )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] *= t;
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator /= ( rectangle_t<S,D>& r,  NO_DEDUCE(S) t )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] /= t;
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator |= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] |= p[n];
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator &= ( rectangle_t<S,D>& r,  NO_DEDUCE2(point_t<S,D>) const& p )
      {
         for ( size_t n = 0; n != D; ++n )
            r[n] &= p[n];
         return r ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator |= ( rectangle_t<S,D>& r1, NO_DEDUCE2(rectangle_t<S,D>) const& r2 )
      {
         for ( size_t n = 0; n != D; ++n )
            r1[n] |= r2[n];
         return r1 ;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator |= ( rectangle_t<S,D>& r, NO_DEDUCE2(segment_t<S,D>) const& s )
      {
         r |= s.P0();
         r |= s.P1();

         return r;
      }

   template <class S, size_t D>
      __forceinline rectangle_t<S,D>& operator &= ( rectangle_t<S,D>& r1, NO_DEDUCE2(rectangle_t<S,D>) const& r2 )
      {
         for ( size_t n = 0; n != D; ++n )
            r1[n] &= r2[n];
         return r1 ;
      }

   // ----------------------------------------------------------------------------------------------- operators
   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator + ( rectangle_t<S1,D> const& r, point_t<S2,D> const&  p )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr += p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator + ( point_t<S1,D> p, rectangle_t<S2,D> const& r )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr += p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator - ( rectangle_t<S1,D> const& r, point_t<S2,D> const&  p )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr -= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator * ( rectangle_t<S1,D> const& r, point_t<S2,D> const&  p )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r) ;
         return rr *= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator * ( point_t<S1,D> p, rectangle_t<S2,D> const& r )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr *= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator / ( rectangle_t<S1,D> const& r, point_t<S2,D> const&  p )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr /= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator * ( rectangle_t<S1,D> const& r, S2 t )
      {
         MAX_RECTANGLE(S1,S2,D) tmp(r);
         return tmp *= t ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator * ( S1 t, rectangle_t<S2,D> const& r )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr *= t ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator / ( rectangle_t<S1,D> const& r, S2 t )
      {
         MAX_RECTANGLE(S1,S2,D) tmp(r);
         return tmp /= t ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator | ( rectangle_t<S1,D> const& r, point_t<S2,D> const&  p )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr |= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator | ( point_t<S1,D> p, rectangle_t<S2,D> const& r)
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr |= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator & ( rectangle_t<S1,D> const& r, point_t<S2,D> const&  p )
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr &= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator & ( point_t<S1,D> p, rectangle_t<S2,D> const& r)
      {
          MAX_RECTANGLE(S1,S2,D) rr(r);
         return rr &= p ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator & ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2)
      {
         MAX_RECTANGLE(S1,S2,D) r(r1);
         return r &= r2 ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RECTANGLE(S1,S2,D) operator | ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2)
      {
          MAX_RECTANGLE(S1,S2,D) rr(r1);
         return rr |= r2 ;
      }

   // --------------------------------------------------------------------------------------------------------- operations
   template <class S1, class S2, size_t D>
      __forceinline bool has_intersection ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2 )
      {
         for ( size_t n = 0; n != D; ++n )
            if ( !has_intersection (r1[n], r2[n]) )
               return false ;
         return true ;
      }

   template <class S1, class S2>
      __forceinline bool has_intersection ( rectangle_t<S1,2> const& rect, point_t<S2,2> const& p0, point_t<S2,2> const& p1 )
      {
         // todo: optimize and make dimension independent, use segment_2 somehow

         if (  p0.x < rect.lo().x && p1.x < rect.lo().x
            || p0.x > rect.hi().x && p1.x > rect.hi().x
            || p0.y < rect.lo().y && p1.y < rect.lo().y
            || p0.y > rect.hi().y && p1.y > rect.hi().y )
            return false ;

         double nX = - (p1.y - p0.y) ;
         double nY = + (p1.x - p0.x) ;

         double dX0 = rect.lo().x - p0.x ;
         double dY0 = rect.lo().y - p0.y ;
         double dX1 = rect.hi().x - p0.x ;
         double dY1 = rect.hi().y - p0.y ;

         double d0 = dX0 * nX + dY0 * nY ;
         double d1 = dX1 * nX + dY0 * nY ;
         double d2 = dX1 * nX + dY1 * nY ;
         double d3 = dX0 * nX + dY1 * nY ;

         if ( d0 > 0 && d1 > 0 && d2 > 0 && d3 > 0 )
            return false ;

         if ( d0 < 0 && d1 < 0 && d2 < 0 && d3 < 0 )
            return false ;

         return true ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_TYPE(S1,S2)  distance ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p )
      {
         return distance (r.closest_point(p), p ) ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_TYPE(S1,S2)  distance ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r )
      {
         return distance(r, p) ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RANGE(S1,S2)  distance_range ( rectangle_t<S1,D> const& r, point_t<S2,D> const& p )
      {
         return MAX_RANGE(S1,S2)(distance(r.closest_point(p), p), distance(r.furthest_point(p), p) )  ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_RANGE(S1,S2)  distance_range ( point_t<S1,D> const& p, rectangle_t<S2,D> const& r )
      {
         return distance_range(r, p) ;
      }

   template <class S1, class S2, size_t D>
      __forceinline MAX_TYPE(S1,S2)  distance ( rectangle_t<S1,D> const& r1, rectangle_t<S2,D> const& r2 )
      {
         return distance ( r1.closest_point(r2), r2 ) ;
      }

   template <class S, size_t D>
      __forceinline point_t<S,D> rand ( rectangle_t<S,D> const& r )
      {
         point_t<S,D> ret ;
         for ( size_t n = 0; n != D; ++n )
            ret[n] = rand(r[n]) ;

         return ret ;
      }

   // --------------------------------------------------------------------------------------------------------- comparasion
   template <class S, size_t D>
      __forceinline bool eq ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2, NO_DEDUCE(S) eps )
      {
         for ( size_t n = 0; n != D; ++n )
            if ( !eq (r1[n], r2[n],eps) )
               return false ;
         return true ;
      }

   template <class S, size_t D>
      __forceinline bool operator == ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2 )
      {
         for ( size_t n = 0; n != D; ++n )
            if ( r1[n] != r2[n] )
               return false ;
         return true ;
      }

   template <class S, size_t D>
      __forceinline bool operator != ( rectangle_t<S,D> const& r1, rectangle_t<S,D> const& r2 )
      {
         return !(r1 == r2) ;
      }
} // namespace geom

#undef MAX_RECTANGLE
#undef MAX_POINT
#undef MAX_RANGE



