/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/primitives/segment.h"
#include "geometry/primitives/rectangle.h"

namespace geom {

   enum intersection_type { disjoint=0, intersect, overlap };

   template < class Pt, class Scalar >
      inline intersection_type generic_intersection(
         segment_t< Scalar, 2 > const &A,
         segment_t< Scalar, 2 > const &B,
         Pt *out1,
         Pt *out2,
         Scalar eps = epsilon<Scalar>() )
   {
      typedef point_t< Scalar, 2 > point_type;

      point_type u = direction(A);
      point_type v = direction(B);
      point_type w = A.P0() - B.P0();

      Scalar eps_prim = 2 * eps; // u, v, w

      Scalar D = u ^ v;

      Scalar eps_D = eps_prim * ( abs(u.x) + abs(u.y) + abs(v.x) + abs(v.y) );
      Scalar eps_vw = eps_prim * ( abs(v.x) + abs(v.y) + abs(w.x) + abs(w.y) );
      Scalar eps_uw = eps_prim * ( abs(u.x) + abs(u.y) + abs(w.x) + abs(w.y) );

      if ( eq_zero( D, eps_D ) )     // if parallel
      {
         if ( !eq_zero( u ^ w, eps_uw ) || !eq_zero( v ^ w, eps_vw ) )  // not on one line
            return disjoint;

         // on one line
         Scalar lu = adjust( u * u, 2 * eps_prim * ( abs(u.x) + abs(u.y) ) );
         Scalar lv = adjust( v * v, 2 * eps_prim * ( abs(v.x) + abs(v.y) ) );

         if (lu == 0 && lv == 0) // both is points
         {
            // 
            if ( !eq_zero( norm_sqr(w), 2 * eps_prim * ( abs(w.x) + abs(w.y) ) ) )
               return disjoint;

            // intersected in the point
            if ( out1 != NULL )
            {
               out1->x = A.P0().x;
               out1->y = A.P0().y;
            }

            return intersect;
         }

         if (lu == 0)   // A is point
         {
            Scalar eps_contains = ( eps_vw * ( v * v ) + 2 * eps_prim * ( abs(v.x) + abs(v.y) ) * abs(v * w) ) / sqr(v * v) * geom::max( abs(B.P0().x) + abs(B.P1().x), abs(B.P0().y) + abs(B.P1().y) ) ;

            if ( !B.contains( A.P0(), eps_contains ) ) // not belong B
               return disjoint;

            // belong
            if ( out1 != NULL )
            {
               out1->x = A.P0().x;
               out1->y = A.P0().y;
            }

            return intersect;
         }

         if (lv == 0)  // B is point
         {
            Scalar eps_contains = ( eps_uw * ( u * u ) + 2 * eps_prim * ( abs(u.x) + abs(u.y) ) * abs(u * -w) ) / sqr(u * u) * geom::max( abs(A.P0().x) + abs(A.P1().x), abs(A.P0().y) + abs(A.P1().y) ) ;

            if ( !A.contains(B.P0(), eps_contains) )  // not belong A
               return disjoint;

            if ( out1 != NULL )
            {
               out1->x = B.P0().x;
               out1->y = B.P0().y;
            }

            return intersect;
         }

         // A and B not points, and lie on one line

         // t0 - projection coeff of B ends on A
         Scalar t0 = A(B.P0());
         Scalar t1 = A(B.P1());

         point_type w_prim = B.P1() - A.P0();

         Scalar eps_t0 = ( eps_uw * ( u * u ) + 2 * eps_prim * ( abs(u.x) + abs(u.y) ) * abs(u * -w) ) / sqr(u * u) * geom::max( abs(A.P0().x) + abs(A.P1().x), abs(A.P0().y) + abs(A.P1().y) ) ;
         Scalar eps_uw_prim = eps_prim * ( abs(u.x) + abs(u.y) + abs(w_prim.x) + abs(w_prim.y) );
         Scalar eps_t1 = ( eps_uw_prim * ( u * u ) + 2 * eps_prim * ( abs(u.x) + abs(u.y) ) * abs(u * w_prim) ) / sqr(u * u) * geom::max( abs(A.P0().x) + abs(A.P1().x), abs(A.P0().y) + abs(A.P1().y) ) ;

         sort2(t0,t1,t0,t1);  // make t0 < t1

         // (!) intersection point - A.P0()
         if (eq_zero(t1,eps_t1))
         {
            if ( out1 != NULL )
            {
               out1->x = A.P0().x;
               out1->y = A.P0().y;
            }
            return intersect;
         }

         // (!) intersection point - A.P1()
         if (eq(t0, 1, eps_t0))
         {
            if ( out1 != NULL )
            {
               out1->x = A.P1().x;
               out1->y = A.P1().y;
            }

            return intersect;
         }

         if (t1 < 0 || t0 > 1) // not intersected
            return disjoint;

         //*out1 = t0 < 0 ? A.P0() : A(t0);
         if ( out1 != NULL )
         {
            if ( t0 < 0 )
            {
               out1->x = A.P0().x;
               out1->y = A.P0().y;
            }
            else
            {
               out1->x = A(t0).x;
               out1->y = A(t0).y;
            }
         }

         //*out2 = t1 > 1 ? A.P1() : A(t1);
         if ( out2 != NULL )
         {
            if ( t1 > 1 )
            {
               out2->x = A.P1().x;
               out2->y = A.P1().y;
            }
            else
            {
               out2->x = A(t1).x;
               out2->y = A(t1).y;
            }
         }

         return overlap;
      }

      // intersection parameter for A
      Scalar tA = (v ^ w) / D;
      Scalar eps_tA = ( eps_vw * abs(D) + eps_D * abs(v^w) ) / sqr(D);

      if ( !between01eps ( tA, eps_tA ) )
         //      if (!between01(tA))
         return disjoint;

      Scalar tB = (u ^ w) / D;
      Scalar eps_tB = ( eps_uw * abs(D) + eps_D * abs(u^w) ) / sqr(D);

      if ( !between01eps ( tB, eps_tB ) )
         //      if (!between01(tB))
         return disjoint;

      //*out1 = A(tA);
      if ( out1 != NULL )
      {
         out1->x = A(tA).x;
         out1->y = A(tA).y;
      }

      return intersect;

   }

   template < class Scalar >
      inline intersection_type generic_intersection( segment_t< Scalar, 2 > const &A,
      segment_t< Scalar, 2 > const &B,
      Scalar eps = epsilon<Scalar>() * 1e-5 )
   {
      return generic_intersection<geom::point_t< Scalar, 2 >, Scalar> ( A, B, NULL, NULL, eps );
   }


   inline bool has_intersection(segment_2 const &s1, segment_2 const &s2)
   {
      point_2 r;
      return disjoint != generic_intersection(s1,s2,&r,&r);
   }

   inline bool is_crossing_exact ( point_2 const & A, point_2 const & B,
      point_2 const & C, point_2 const & D )
   {
      return has_intersection(segment_2(A, B), segment_2(C, D));
   }

   inline double distance(segment_2 const &S1, segment_2 const &S2)
   {
      geom::intersection_type ipt_type = geom::generic_intersection <geom::point_2> ( S1, S2, NULL, NULL );

      if ( ipt_type == geom::disjoint )
         return geom::min ( distance(S2, S1.P0()), distance(S2, S1.P1()), distance(S1, S2.P0()), distance(S1, S2.P1()) );

      return 0;
   }
}
