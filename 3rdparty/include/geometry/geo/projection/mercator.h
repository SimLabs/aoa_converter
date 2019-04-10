/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/primitives/geo_point.h"
#include "geometry/primitives/glb_point.h"

// interface
namespace geom
{
   namespace merkator_wgs84
   {
      geo_point_2 toGeo ( const glb_point_2 & pos, double base_lat = 0, double eps = 0.001 );
      glb_point_2 toGlb ( const geo_point_2 & pos, double base_lat = 0 );

      geo_point_2 toGeoSphere ( const glb_point_2 & pos, double base_lat = 0 );
      glb_point_2 toGlbSphere ( const geo_point_2 & pos, double base_lat = 0 );

      // note. "toGeo::eps" in meters, 10 cycles max. eps=0.001 may take 7 cycles
   }
}

///////////////////////////////////////////////////////////////////////////////////////
// implementation
namespace geom
{
   namespace merkator
   {
      inline double melat ( double lat )
      {
         return log(tan(lat/2 + pi/4));
      }

      inline double latit ( double mlt )
      {
         return (atan(exp(mlt)) - pi/4) * 2;
      }

      inline double melat ( double lat, double E )
      {
         return log(tan(pi/4 + lat/2) * pow ((1 - E*sin(lat))/(1 + E*sin(lat)), E/2)) ;
      }

      inline double latit ( double mlt, double E, double eps )
      {
          double eps_sq = fabs(latit(mlt) - latit(mlt+eps));

		  double ts = exp ( -mlt);
		  double phi = pi / 2 - 2 * atan(ts);
		  for (int i = 0; i < 10; i++)
          {
			  double con = E * sin (phi);
              double old = phi;
			  phi = pi / 2 - 2 * atan (ts * pow((1.0 - con) / (1.0 + con), 0.5 * E));
              if ( eq(phi, old, eps_sq) )
                  break;
		  }
		  return phi;

      }
   } // merkator

   namespace wgs84
   {
      const double Rbig   = 6378137 ;
      const double Finv   = 298.257223563;
      const double F      = 1.0 / Finv;
      const double Rsmall = Rbig * (1.0 - F);
      const double E      = sqrt((2.0 - F) * F);
   } // wgs84

   namespace merkator_wgs84
   {
      inline geo_point_2 toGeo ( const glb_point_2 & pos, double base_lat, double eps )
      {
         double factor = wgs84::Rbig*cos(grad2rad(base_lat));

         return geo_point_2( rad2grad(merkator::latit(pos.Y/factor, wgs84::E, eps/factor)), rad2grad(pos.X/factor) ) ;
      }

      inline glb_point_2 toGlb ( const geo_point_2 & pos, double base_lat )
      {
         double factor = wgs84::Rbig*cos(grad2rad(base_lat));

         return glb_point_2(grad2rad(pos.lon)*factor, merkator::melat(grad2rad(pos.lat), wgs84::E)*factor ) ;
      }

      inline geo_point_2 toGeoSphere ( const glb_point_2 & pos, double base_lat )
      {
         double factor = wgs84::Rbig*cos(grad2rad(base_lat));

         return geo_point_2(rad2grad(merkator::latit(pos.Y/factor)), rad2grad(pos.X/factor) ) ;
      }

      inline glb_point_2 toGlbSphere ( const geo_point_2 & pos, double base_lat )
      {
         double factor = wgs84::Rbig*cos(grad2rad(base_lat));

         return glb_point_2(grad2rad(pos.lon)*factor, merkator::melat(grad2rad(pos.lat))*factor ) ;
      }
   } // merkator_wgs84
} // geom
