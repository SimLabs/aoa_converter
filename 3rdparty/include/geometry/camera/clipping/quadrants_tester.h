/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

/*************************************************************
 * Filename: QuadrantsTester.h
 * Purpose : 8 octants space subdivision
 *************************************************************/

#pragma once

#include "../frustum_fwd.h"
#include "clipping_fwd.h"

//
// Quadrants frustum test class
//

class QuadrantsTester
{

   enum
   {
      Quadrant_LeftBottomFront = 0,
      Quadrant_LeftTopFront,
      Quadrant_LeftBottomBack,
      Quadrant_LeftTopBack,
      Quadrant_RightBottomFront,
      Quadrant_RightTopFront,
      Quadrant_RightBottomBack,
      Quadrant_RightTopBack,
      Quadrant_Max
   };

   // get bit code of point
   inline int BitCode( const geom::point_3f &p_in )
   {
      return (int)(p_in.x > 0) * 4 + (int)(p_in.y > 0) * 2 + (int)(p_in.z > 0);
   }

public:

   // default constructor
   QuadrantsTester ( void )
   {
      offsets[Quadrant_LeftBottomFront] =  geom::point_3f(-0.5f, -0.5f, -0.5f);
      offsets[Quadrant_LeftTopFront] =     geom::point_3f(-0.5f, -0.5f,  0.5f);
      offsets[Quadrant_LeftBottomBack] =   geom::point_3f(-0.5f,  0.5f, -0.5f);
      offsets[Quadrant_LeftTopBack] =      geom::point_3f(-0.5f,  0.5f,  0.5f);
      offsets[Quadrant_RightBottomFront] = geom::point_3f( 0.5f, -0.5f, -0.5f);
      offsets[Quadrant_RightTopFront] =    geom::point_3f( 0.5f, -0.5f,  0.5f);
      offsets[Quadrant_RightBottomBack] =  geom::point_3f( 0.5f,  0.5f, -0.5f);
      offsets[Quadrant_RightTopBack] =     geom::point_3f( 0.5f,  0.5f,  0.5f);
      for (int i = 0; i < Quadrant_Max; i++)
         view_states[i] = false;
   }

   // update view state
   template< typename S >
   void UpdateState( geom::frustum_t<S> const * frustum )
   {
      if (frustum == NULL)
         return;

      boost::scoped_ptr<geom::frustum_t<S>> frustum_copy(frustum->clone());
      frustum_copy->camera().set_position(point_3());
      geom::frustum_clipper frustum_clipper(*frustum_copy.get());

      // setting view state
      for (int i = 0; i < Quadrant_Max; i++)
      {
         if (frustum_clipper.is_visible(geom::aabb_clip_data(offsets[i] * 10, point_3f(5.f, 5.f, 5.f))))
            view_states[i] = true;
         else
            view_states[i] = false;
      }
   }



   // get state of quadrant
   bool GetQuadrantState( int Quadrant )
   {
      Assert(Quadrant >= Quadrant_LeftBottomFront && Quadrant < Quadrant_Max);
      if (Quadrant < Quadrant_LeftBottomFront || Quadrant >= Quadrant_Max)
         return false;
      return view_states[Quadrant];
   }

   // get offset of quadrant
   geom::point_3f GetQuadrantOffset( int Quadrant, geom::point_3f QuadrantSize )
   {
      geom::point_3f out = geom::point_3f(0.f, 0.f, 0.f);

      Assert(Quadrant >= Quadrant_LeftBottomFront && Quadrant < Quadrant_Max);
      if (Quadrant < Quadrant_LeftBottomFront || Quadrant >= Quadrant_Max)
         return out;

      out = offsets[Quadrant] & QuadrantSize;
      return out;
   }

private:

   geom::point_3f offsets[8]; // vector offsets to quadrant centers
   bool view_states[8];     // quadrant's view state
};
