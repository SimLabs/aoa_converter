/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "geometry/primitives/cycle_val.h"

#include <list>

namespace filter
{
   template < typename T >
   struct diff1_t
   {
      typedef T type ;
   };

   template < typename T >
   struct diff2_t
   {
      typedef typename diff1_t< typename diff1_t< T >::type >::type type ;
   };

   template <>
   struct diff1_t< geom::degree180_value >
   {
      typedef double type ;
   };

//-- 1D signal filtering

    //  limits max |X| increment
    // cX - current X, dX - desired X
    template <typename vector, typename norm>
        typename diff1_t< vector >::type
        approach_linear ( vector const& cX, vector const& dX, norm deltaAbsX )
    {
        vector delta = ( dX - cX ) ;
        norm   deltaNorm = geom::norm( delta ) ;

        if ( deltaNorm > deltaAbsX )
           delta   *= deltaAbsX / deltaNorm ;

        return delta ;
    }

    // limits speed
    template <typename vector, typename norm>
        typename diff1_t< vector >::type
        approach_linear( vector const& cX, vector const& dX,
                         typename diff1_t< vector >::type const& vX, norm dt, norm smoothFactor = 1 )
    {
        return approach_linear <vector, norm> ( cX, dX, (typename diff1_t< vector >::type)( vX * dt / smoothFactor )) ;
    }

    // DX/dt = alpha * (desired - X)
    template <typename T> T approach_exp(T cX, T dX, T alpha)
    {
        T dLX = geom::abs(dX - cX) * alpha;
        return (cX < dX) ? (cX + dLX) : (cX - dLX);
    }

    // DX/dt = alpha * (desired - X)^gamma
    template <typename T> T approach_exp(T cX, T dX, T alpha, T gamma, T offset)
    {
        T dLX;
        T arg = geom::abs(dX - cX);
        if (arg < offset)
            dLX = alpha * offset * pow(arg / offset, gamma);
        else
            dLX = alpha * arg;

        return (cX < dX) ? (cX + dLX) : (cX - dLX);
    }

    // limits first and second derivative, with desired speed = 0
    template < class scalar >
      typename diff1_t< scalar >::type
      break_approach_speed
         (  scalar curPos, scalar desPos,
            typename diff1_t< scalar >::type curSpeed,
            typename diff1_t< scalar >::type maxSpeed,
            typename diff2_t< scalar >::type maxAccel,
            double dt, double smoothFactor = 1 )
   {
      Assert( geom::ge( smoothFactor, 1 )) ;

      typedef
         typename diff1_t< scalar >::type
         dscalar ;

      dscalar dpos    = desPos - curPos ;
      dscalar speed   = dpos / ( smoothFactor * dt ) ;

      dscalar maxBreakSpeed = geom::sqrt( dscalar(2) * maxAccel * geom::abs ( dpos ) ) ;
      dscalar    breakSpeed = maxBreakSpeed - maxAccel * dt / 2 ; // average speed during calc step

      if ( maxBreakSpeed - maxAccel * dt < 0 )
         return dpos / dt ;

      // limit acceleration
      speed = geom::bound<dscalar>( speed, curSpeed - maxAccel * dt, curSpeed + maxAccel * dt );

      // if for deceleration less speed is needed - use it
      if ( ( speed * dpos ) > 0 && breakSpeed < geom::norm( speed ))
         speed = geom::sign( speed ) * breakSpeed ;

//       if ( breakSpeed < geom::norm( speed ))
//          speed = geom::sign( dpos ) * breakSpeed ;
//       else
//          speed = geom::bound<dscalar>( speed, curSpeed - maxAccel * dt, curSpeed + maxAccel * dt );

      return geom::bound<dscalar>( speed, -maxSpeed, maxSpeed );
   }

    template < class scalar >
      typename diff1_t< scalar >::type
      goal_approach_speed
         (  scalar curPos, scalar desPos, scalar goalPos,
            typename diff1_t< scalar >::type curSpeed,
            typename diff1_t< scalar >::type maxSpeed,
            typename diff2_t< scalar >::type maxAccel,
            double dt, double smoothFactor = 1 )
   {
      Assert( geom::ge( smoothFactor, 1 )) ;

      typedef
         typename diff1_t< scalar >::type
         dscalar ;

      dscalar dpos    = desPos - curPos ;
      dscalar goal_dpos    = goalPos - curPos ;
      dscalar speed   = dpos / ( smoothFactor * dt ) ;

      dscalar maxBreakSpeed = geom::sqrt( dscalar(2) * maxAccel * geom::abs ( goal_dpos ) ) ;
      dscalar    breakSpeed = maxBreakSpeed - maxAccel * dt / 2 ; // average speed during calc step

      if ( maxBreakSpeed - maxAccel * dt < 0 )
         return goal_dpos / dt ;

      // limit acceleration
      speed = geom::bound<dscalar>( speed, curSpeed - maxAccel * dt, curSpeed + maxAccel * dt );

      // if for deceleration less speed is needed - use it
      if ( ( speed * goal_dpos ) > 0 && breakSpeed < geom::norm( speed ))
         speed = geom::sign( speed ) * breakSpeed ;

      return geom::bound<dscalar>( speed, -maxSpeed, maxSpeed );
   }

      // limits first and second derivative, with desired speed != 0
    template < typename vector, typename norm >
       typename diff1_t< vector >::type
         approach_speed
         (  vector const& curPos, vector const& desPos,
            typename diff1_t< vector >::type const& curV,
            norm maxVNorm, norm maxANorm, double dt, double smoothFactor = 1. )
    {
       Assert( !geom::eq_zero( dt )) ;

       typedef typename diff1_t < vector >::type dvector ;

       //-- max velocity constraint
       dvector desV = ( desPos - curPos ) / ( dt * smoothFactor ) ;

       norm desVNorm = geom::norm( desV ) ;
       if ( desVNorm > maxVNorm )
          desV *= maxVNorm / desVNorm ;

       //-- max accel constraint
       dvector newV = geom::clamp_d( 0., geom::norm( desV - curV ), curV, desV ) ( maxANorm * dt ) ;

       //-- new velocity
       return newV ;
    }

    // limits first and second derivative of angle value, with desired speed != 0
    inline point_3 approach_ang_speed( const quaternion & cur_orien, const quaternion & des_orien, const point_3 cur_omega, double max_ang_speed, double max_ang_acc, double dt, double smooth_factor = 1. )
    {
       Assert( !geom::eq_zero( dt )) ;

       const point_3 des_rot = get_rotate_quaternion(cur_orien, des_orien).rot_axis().omega();
       point_3 des_omega = des_rot / (dt * smooth_factor);

       //-- max velocity constraint
       double des_omega_norm = norm(des_omega);
       if ( des_omega_norm > max_ang_speed )
          des_omega *= max_ang_speed / des_omega_norm;

       //-- max accel constraint
       const point_3 new_omega = geom::clamp_d(0., norm(des_omega - cur_omega), cur_omega, des_omega)(max_ang_acc * dt);

       return new_omega;
    }

    // filter with memory
    template <typename T >
        struct frame_filter
    {
        frame_filter (size_t frameSize, T const& def = T())
            : m_Next  ( 0 )
            , m_Values( frameSize, def )
        {
        }

        size_t size  () { return m_Values.size(); }
        void   clear () { m_Values.resize(0) ; }
        void   push  (T const& v)
        {
            m_Values[m_Next] = v;
            m_Next = (m_Next + 1) % m_Values.size();
        }

        template<class Processor>
           Processor visit( Processor proc )
        {
            for (size_t i = m_Next; i != m_Values.size(); ++i)
               proc(m_Values[i]);

            for (size_t i = 0; i != m_Next; ++i)
               proc(m_Values[i]);

            return proc;
        }

    private:
        typedef std::vector<T> values_type;

        size_t       m_Next     ;
        values_type  m_Values   ;
    };

    template<typename T>
       struct average_processor
    {
       average_processor( size_t size, T const& start = T() )
          : m_Size(size)
          , m_Sum (start)
       {}

       void operator()( T const& t ) { m_Sum += t; }
       T    operator()() const { return m_Sum / m_Size; }

    private:
       size_t  m_Size;
       T       m_Sum;
    };


} // end of namespace 'filter'

