/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/common.h"

#include "point.h"
#include "matrix.h"
#include "rotation.h"
#include "quaternion.h"

#include "transform_fwd.h"

#include <utility>

// transform is Translate-Rotate-Scale

// Ix * Sx | Jx * Sy | Kx * Sz | Tx
// Iy * Sx | Jy * Sy | Ky * Sz | Ty
// Iz * Sx | Jz * Sy | Kz * Sz | Tz
// 0       | 0       | 0       | 1

namespace geom
{
   template< typename S, size_t N > struct transform_t;

   template< typename S, size_t N > struct translation_t;
   template< typename S, size_t N > struct vector_t;
   template< typename S, size_t N > struct normal_t;
   template< typename S, size_t N > struct scale_t;

   template< typename S, size_t N > translation_t<S, N> const & as_translation( point_t<S, N> const & p );
   template< typename S, size_t N > scale_t      <S, N> const & as_scale      ( point_t<S, N> const & p );
   template< typename S, size_t N > normal_t     <S, N> const & as_normal     ( point_t<S, N> const & p );
   template< typename S, size_t N > vector_t     <S, N> const & as_vector     ( point_t<S, N> const & p );

   enum scale_state
   {
      ss_unscaled     = 0,
      ss_equal_scaled = 1,
      ss_scaled       = 3,

      P_UNKNOWN      = ss_scaled,
      ss_autodetect   = -1
   };

   template< typename S, size_t N > scale_state get_scale_state( scale_t<S, N> const& p );

   // operations
   // transform_t operator * (transform_t   const&, transform_t    const& ) ;

   // -- post translation
   // transform_t operator * (transform_t   const&,  rotation_t    const& ) ;
   // transform_t operator * (transform_t   const&,  scale_t       const& ) ;
   // transform_t operator * (transform_t   const&,  translation_t const& ) ;

   // -- pre translation
   // transform_t operator * (rotation_t    const&,  transform_t   const& ) ;
   // transform_t operator * (scale_t       const&,  transform_t   const& ) ;
   // transform_t operator * (translation_t const&,  transform_t   const& ) ;

   // -- treating
   // point_t     operator * (transform_t   const&,  point_t       const& ) ;
   // normal_t    operator * (transform_t   const&,  normal_t      const& ) ;
   // vector_t    operator * (transform_t   const&,  vector_t      const& ) ;

   // -- back treating
   // point_t     operator * (point_t       const&,  transform_t   const& ) ;
   // normal_t    operator * (normal_t      const&,  transform_t   const& ) ;
   // vector_t    operator * (vector_t      const&,  transform_t   const& ) ;

   // interpolation
   // transform_t blend( const transform_t &tr1, const transform_t &tr2, double t ) ;
   // transform_t slerp( const transform_t &tr1, const transform_t &tr2, double t ) ;
}


namespace geom
{
#pragma pack(push, 1)

   // point aliases
   template< typename S, size_t N > struct translation_t : point_t<S, N> {};
   template< typename S, size_t N > struct vector_t      : point_t<S, N> {};
   template< typename S, size_t N > struct normal_t      : point_t<S, N> { normal_t () { this->operator[](N-1) = 1; } };

   template< typename S, size_t N > struct scale_t       : point_t<S, N>
   {
      __forceinline scale_t( S s = 1 )
         : point_t<S, N>()
      {
         for (size_t l = 0; l != N; ++l)
            this->operator[](l) = s;
      }

      template< typename _S >
      __forceinline scale_t( scale_t<_S, N> const & other )
         : point_t<S, N>(other)
      {
      }
   };

   template< typename scalar, size_t N >
   struct transform_t
   {
      typedef point_t      <scalar, N-1> point_t;
      typedef matrix_t     <scalar, N>   matrix_t;
      typedef rotation_t   <scalar, N-1> rotation_t;
      typedef translation_t<scalar, N-1> translation_t;
      typedef scale_t      <scalar, N-1> scale_t;
      typedef normal_t     <scalar, N-1> normal_t;
      typedef vector_t     <scalar, N-1> vector_t;

      transform_t();
      transform_t( translation_t const & t, rotation_t const & r = rotation_t(), scale_t const & s = scale_t() );
      transform_t( matrix_t const & direct_matrix, scale_state state = P_UNKNOWN );
      transform_t( matrix_t const & direct_matrix, matrix_t const & inverse_matrix, scale_state state = P_UNKNOWN );

      transform_t( transform_t const & other );
      transform_t( transform_t && other );
      transform_t & operator = ( transform_t const & other );

      template< typename _S >
      transform_t( transform_t<_S, N> const & other );

      matrix_t    const & direct_matrix () const;
      matrix_t    const & inverse_matrix() const;
      scale_state         state         () const;

      rotation_t       rotation         () const;
      scale_t          scale            () const;
      translation_t    translation      () const;

      transform_t &    transform        ( transform_t const & t, bool post = true );
      transform_t &    rotate           ( rotation_t  const & r, bool post = true );
      transform_t &    scale            ( point_t     const & s, bool post = true );
      transform_t &    translate        ( point_t     const & t, bool post = true );

      transform_t &    reset_transform  ();
      transform_t &    reset_rotation   ();
      transform_t &    reset_scale      ();
      transform_t &    reset_translation();

      transform_t &    operator *=      ( transform_t   const & t );
      transform_t &    operator *=      ( rotation_t    const & r );
      transform_t &    operator *=      ( scale_t       const & s );
      transform_t &    operator *=      ( translation_t const & t );

      transform_t      treat_transform  ( transform_t const & t, bool post = true ) const;
      point_t          treat_point      ( point_t     const & p, bool post = true ) const;
      normal_t         treat_normal     ( point_t     const & n, bool post = true ) const;
      vector_t         treat_vector     ( point_t     const & v, bool post = true ) const;

      transform_t &    invert           ();
      transform_t      inverted         () const;

   private:
      void validate_inverse_matrix() const;

   private:
      unsigned         state_;
      matrix_t         direct_;

      // inverse matrix should actually be boost::optional
      // due to lower performance of non inline boost::optional routines (i.e. reset())
      // a simpler solution was used
      mutable matrix_t inverse_;
      mutable bool     inverse_valid_;

      template< typename, size_t > friend struct transform_t;
   };

#pragma pack(pop)
}

namespace geom
{
   template< typename S, size_t N > __forceinline translation_t<S, N> const & as_translation ( point_t<S, N> const & p ) { return static_cast<translation_t<S, N> const &>(p); }
   template< typename S, size_t N > __forceinline scale_t      <S, N> const & as_scale       ( point_t<S, N> const & p ) { return static_cast<scale_t      <S, N> const &>(p); }
   template< typename S, size_t N > __forceinline normal_t     <S, N> const & as_normal      ( point_t<S, N> const & p ) { return static_cast<normal_t     <S, N> const &>(p); }
   template< typename S, size_t N > __forceinline vector_t     <S, N> const & as_vector      ( point_t<S, N> const & p ) { return static_cast<vector_t     <S, N> const &>(p); }

   template< typename S, size_t N >
   __forceinline scale_state get_scale_state( scale_t<S, N> const & s )
   {
      for (size_t i = 1; i < N; ++i)
         if (!eq(s[0], s[i]))
            return ss_scaled;

      return eq(s[0], (S)1) ? ss_unscaled : ss_equal_scaled;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>::transform_t()
      : state_        (ss_unscaled)
      , direct_       (matrix_t(1))
      , inverse_      (matrix_t(1))
      , inverse_valid_(true)
   {
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>::transform_t( translation_t const & t, rotation_t const & r, scale_t const & s )
      : state_        (get_scale_state(s))
      , direct_       (matrix::undefined)
      , inverse_      (matrix::undefined)
      , inverse_valid_(false)
   {
      for (size_t i = 0 ; i < N - 1; ++i)
         for (size_t j = 0 ; j < N - 1; ++j)
            direct_(i, j) = r.matrix()(i, j) * s[j];

      for (size_t i = 0 ; i < N - 1; ++i)
      {
         direct_(i, N - 1) = t[i];
         direct_(N - 1, i) = 0;
      }

      direct_(N - 1, N - 1) = 1;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>::transform_t( matrix_t const & direct_matrix, scale_state state )
      : state_        (state == ss_autodetect ? P_UNKNOWN : state)
      , direct_       (direct_matrix)
      , inverse_      (matrix::undefined)
      , inverse_valid_(false)
   {
      if (state_ == ss_autodetect)
         state_ = get_scale_state(scale());
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>::transform_t( matrix_t const & direct_matrix, matrix_t const & inverse_matrix,
      scale_state state = P_UNKNOWN )
      : state_  (state == ss_autodetect ? P_UNKNOWN : state)
      , direct_       (direct_matrix)
      , inverse_      (inverse_matrix)
      , inverse_valid_(true)
   {
      if (state == ss_autodetect)
         state_ = get_scale_state(scale());
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>::transform_t ( transform_t const & other )
      : state_        (other.state_)
      , direct_       (other.direct_)
      , inverse_      (matrix::undefined)
      , inverse_valid_(other.inverse_valid_)
   {
      if (inverse_valid_)
         inverse_ = other.inverse_;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>::transform_t ( transform_t && other )
       : state_        (std::move(other.state_))
       , direct_       (std::move(other.direct_))
       , inverse_      (matrix::undefined)
       , inverse_valid_(std::move(other.inverse_valid_))
   {
       if (inverse_valid_)
           inverse_ = std::move(other.inverse_);
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::operator = ( transform_t const & other )
   {
       if (this != &other)
       {
           state_ = other.state_;
           direct_ = other.direct_;
           inverse_valid_ = other.inverse_valid_;
           if (inverse_valid_)
               inverse_ = other.inverse_;
       }
       return *this;
   }


   template< typename S, size_t N >
   template< typename _S >
   __forceinline transform_t<S, N>::transform_t ( transform_t<_S, N> const & other )
      : state_        (other.state_)
      , direct_       (other.direct_)
      , inverse_      (matrix::undefined)
      , inverse_valid_(other.inverse_valid_)
   {
      if (inverse_valid_)
         inverse_ = other.inverse_;
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::matrix_t const & transform_t<S, N>::direct_matrix() const
   {
      return direct_;
   }

   template< typename S, size_t N >
   __forceinline scale_state transform_t<S, N>::state() const
   {
      return scale_state(state_);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::rotation_t transform_t<S, N>::rotation() const
   {
      point_t const sc = scale();

      typename rotation_t::matrix_t m;
      for (size_t i = 0; i < N - 1; ++i)
         for (size_t j = 0; j < N - 1; ++j)
            m(i, j) = direct_(i, j) / sc[j];

      return rotation_t(m);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::scale_t transform_t<S, N>::scale() const
   {
      if (state_ == ss_equal_scaled)
      {
         S s = 0;
         for (size_t i = 0; i < N - 1; ++i)
            s += sqr(direct_(i, 0));
         return scale_t(geom::sqrt(s));
      }
      if (state_ == ss_scaled)
      {
         point_t p;
         for (size_t i = 0; i < N - 1; ++i)
         {
            S s = 0;
            for (size_t j = 0 ; j < N - 1; ++j)
               s += sqr(direct_(j, i));
            p[i] = geom::sqrt(s);
         }
         return as_scale(p);
      }
      return scale_t();
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::translation_t transform_t<S, N>::translation() const
   {
      point_t t;
      for (size_t i = 0; i < N - 1; ++i)
         t[i] = direct_(i, N - 1);

      return as_translation(t);
   }

   template< typename S, size_t N >
   __forceinline void multiply_transform_matrix( matrix_t<S, N> & direct, matrix_t<S, N> const & other, bool post )
   {
      if (post)
         direct *= other;
      else
         direct = other * direct;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>& transform_t<S, N>::transform( transform_t<S, N> const & t, bool post )
   {
      inverse_valid_ = false;

      multiply_transform_matrix(direct_, t.direct_matrix(), post);

      state_ = state_ | t.state();

      return *this;
   }

   // performance specialization
   template< typename S >
   void multiply_transform_matrix( matrix_t<S, 4> & direct, matrix_t<S, 4> const & other, bool post )
   {
      if (post)
      {
         S temp[3];
         for (size_t i = 0; i < 3; ++i)
         {
            memcpy(temp, &direct(i, 0), sizeof(temp));
            for (size_t j = 0; j < 3; ++j)
               direct(i, j) = temp[0] * other(0, j) + temp[1] * other(1, j) + temp[2] * other(2, j);
            direct(i, 3) += temp[0] * other(0, 3) + temp[1] * other(1, 3) + temp[2] * other(2, 3);
         }
      }
      else
      {
         matrix_t<S, 4> const temp(direct);
         for (size_t i = 0; i < 3; ++i)
         {
            for (size_t j = 0; j < 3; ++j)
               direct(i,j) = other(i,0) * temp(0,j) + other(i,1) * temp(1,j) + other(i,2) * temp(2,j);
            direct(i,3) = other(i,0) * temp(0,3) + other(i,1) * temp(1,3) + other(i,2) * temp(2,3) + other(i,3);
         }
      }
   }

   // performance specializations
   template< typename S >
   void rotate_transform_matrix( matrix_t<S, 4> & direct, matrix_t<S, 3> const & rot_m, bool post )
   {
      if (post)
      {
         S temp[3];
         for (size_t i = 0; i < 3; ++i)
         {
            memcpy(temp, &direct(i, 0), sizeof(temp));
            for (size_t j = 0; j < 3; ++j)
               direct(i,j) = temp[0] * rot_m(0,j) + temp[1] * rot_m(1,j) + temp[2] * rot_m(2,j);
         }
      }
      else
      {
         matrix_t<S, 4> const temp(direct);
         for ( size_t i = 0; i < 3; i++ )
            for ( size_t j = 0; j < 4; j++ )
               direct(i,j) = rot_m(i,0) * temp(0,j) + rot_m(i,1) * temp(1,j) + rot_m(i,2) * temp(2,j);
      }
   }

   template<typename S>
   void rotate_transform_matrix ( matrix_t<S,3>& direct, matrix_t<S,2> const& rot_m, bool post )
   {
      if (post)
      {
         S temp[2];
         for (size_t i = 0; i != 2; ++i)
         {
            memcpy(temp, &direct(i, 0), sizeof(temp));
            for ( size_t j = 0; j != 2; ++j )
               direct(i,j) = temp[0] * rot_m(0,j) + temp[1] * rot_m(1,j);
         }
      }
      else
      {
         matrix_t<S, 3> const temp(direct);
         for (size_t i = 0; i != 2; ++i)
            for (size_t j = 0; j != 3; ++j)
               direct(i,j) = rot_m(i,0) * temp(0,j) + rot_m(i,1) * temp(1,j);
      }
   }

   // performance specializations
   template<typename S>
   void rotate_transform_matrix ( matrix_t<S,3>& direct, matrix_t<S,3> const& rot_m, bool post )
   {
      matrix_t<S, 3> const temp(direct);
      if (post)
         for (size_t i = 0; i < 2; ++i)
            for (size_t j = 0; j < 2; ++j)
               direct(i,j) = temp(i,0) * rot_m(0,j) + temp(i,1) * rot_m(1,j);
      else
         for ( size_t i = 0; i < 2; i++ )
            for ( size_t j = 0; j < 3; j++ )
               direct(i,j) = rot_m(i,0) * temp(0,j) + rot_m(i,1) * temp(1,j);
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::rotate ( rotation_t const & r, bool post )
   {
      inverse_valid_ = false;

      rotate_transform_matrix(direct_, r.matrix(), post);

      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>& transform_t<S, N>::scale( point_t const & s, bool post )
   {
      inverse_valid_ = false;

      if (post)
      {
         for (size_t i = 0; i < N - 1; ++i)
            for (size_t j = 0; j < N - 1; ++j)
               direct_(i, j) *= s[j];
      }
      else
      {
         for (size_t i = 0 ; i < N - 1; ++i)
            for (size_t j = 0; j < N; ++j)
               direct_(i, j) *= s[i];
      }

      state_ = state_ | get_scale_state(as_scale(s));

      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::translate( point_t const & t, bool post )
   {
      inverse_valid_ = false;

      if (post)
      {
         for (size_t i = 0 ; i < N - 1; ++i)
            for (size_t k = 0 ; k < N - 1; ++k)
               direct_(i, N - 1) += direct_(i, k) * t[k];
      }
      else
      {
         for (size_t i = 0 ; i < N - 1; ++i)
            direct_(i, N - 1) += t[i];
      }

      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::reset_transform()
   {
      direct_        = matrix_t(1);
      inverse_       = matrix_t(1);
      inverse_valid_ = true;
      state_         = ss_unscaled;

      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>& transform_t<S, N>::reset_rotation()
   {
      scale_t const s = scale();

      inverse_valid_ = false;

      for (size_t i = 0; i < N - 1; ++i)
         for (size_t j = 0; j < N - 1; ++j)
            direct_(i, j) = i == j ? s[i] : S(0);

      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::reset_scale()
   {
      scale_t const s = scale();

      inverse_valid_ = false;

      for (size_t i = 0 ; i < N - 1; ++i)
         for (size_t j = 0 ; j < N - 1; ++j)
            direct_(i, j) /= s[j];

      state_ = ss_unscaled;
      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::reset_translation()
   {
      inverse_valid_ = false;

      for (size_t i = 0 ; i < N - 1; ++i)
         direct_(i, N - 1) = 0;

      return *this;
   }

   // modification and assignment
   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::operator *=( transform_t<S, N> const & t )
   {
      transform(t);
      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>& transform_t<S, N>::operator *=( rotation_t const& t )
   {
      rotate(t);
      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>& transform_t<S, N>::operator *=( scale_t const& s )
   {
      scale(s);
      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N>& transform_t<S, N>::operator *=( translation_t const& t )
   {
      translate(t);
      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> transform_t<S, N>::treat_transform( transform_t<S, N> const & t, bool post ) const
   {
      return transform_t<S, N>(*this).transform(t, post);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::point_t transform_t<S, N>::treat_point( point_t const & p, bool post ) const
   {
      matrix_t const & m = post ? direct_ : inverse_matrix();

      point_t ret;
      for (size_t i = 0; i < N - 1; ++i)
      {
         ret[i] = m(i, N - 1);
         for (size_t j = 0 ; j < N - 1; ++j)
            ret[i] += m(i, j) * p[j];
      }

      return ret;
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::vector_t transform_t<S, N>::treat_vector( point_t const & v, bool post ) const
   {
      matrix_t const & m = post ? direct_ : inverse_matrix();

      point_t ret;
      for (size_t i = 0; i < N - 1; ++i)
         for (size_t j = 0; j < N - 1; ++j)
            ret[i] += m(i, j) * v[j];

      return as_vector(ret);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::normal_t transform_t<S, N>::treat_normal( point_t const & n, bool post ) const
   {
      if (state_ == ss_unscaled)
         return as_normal(static_cast<point_t>(treat_vector(n, post)));

      if (state_ == ss_equal_scaled)
         return as_normal(normalized(static_cast<point_t>(treat_vector(n, post))));

      matrix_t const & m = post ? inverse_matrix() : direct_;

      point_t ret;
      for (size_t i = 0; i < N - 1; ++i)
         for (size_t j = 0; j < N - 1; ++j)
            ret[i] += m(j, i) * n[j];

      return as_normal(normalized(ret));
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> & transform_t<S, N>::invert()
   {
      validate_inverse_matrix();

      std::swap(direct_, inverse_);

      return *this;
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> transform_t<S, N>::inverted() const
   {
      validate_inverse_matrix();

      return transform_t(*this).invert();
   }

   template< typename S, size_t N >
   __forceinline void transform_t<S, N>::validate_inverse_matrix() const
   {
      if (!inverse_valid_)
      {
         invert_transform_matrix(direct_, inverse_, state_);
         inverse_valid_ = true;
      }
   }

   template< typename S, size_t N > void invert_transform_matrix( matrix_t<S, N> const & m, matrix_t<S, N> & r, scale_state state );

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::matrix_t const & transform_t<S, N>::inverse_matrix() const
   {
      validate_inverse_matrix();

      return inverse_;
   }

   // inversion specialization
   template< typename S >
   void invert_transform_matrix( matrix_t<S, 4> const & direct, matrix_t<S, 4> & inverse, unsigned state )
   {
      if (state == ss_scaled)
      {
         inverse(0,0) = direct(1,1) * direct(2,2) - direct(2,1) * direct(1,2);
         inverse(1,0) = direct(2,0) * direct(1,2) - direct(1,0) * direct(2,2);
         inverse(2,0) = direct(1,0) * direct(2,1) - direct(2,0) * direct(1,1);
         inverse(0,1) = direct(2,1) * direct(0,2) - direct(0,1) * direct(2,2);
         inverse(1,1) = direct(0,0) * direct(2,2) - direct(2,0) * direct(0,2);
         inverse(2,1) = direct(2,0) * direct(0,1) - direct(0,0) * direct(2,1);
         inverse(0,2) = direct(0,1) * direct(1,2) - direct(1,1) * direct(0,2);
         inverse(1,2) = direct(1,0) * direct(0,2) - direct(0,0) * direct(1,2);
         inverse(2,2) = direct(0,0) * direct(1,1) - direct(1,0) * direct(0,1);
         inverse(0,3) = - determinant ( direct(0,1), direct(0,2), direct(0,3),
                                        direct(1,1), direct(1,2), direct(1,3),
                                        direct(2,1), direct(2,2), direct(2,3) );
         inverse(1,3) =   determinant ( direct(0,0), direct(0,2), direct(0,3),
                                        direct(1,0), direct(1,2), direct(1,3),
                                        direct(2,0), direct(2,2), direct(2,3) );
         inverse(2,3) = - determinant ( direct(0,0), direct(0,1), direct(0,3),
                                        direct(1,0), direct(1,1), direct(1,3),
                                        direct(2,0), direct(2,1), direct(2,3) );

         S det = determinant(direct(0,0), direct(1,0), direct(2,0),
                             direct(0,1), direct(1,1), direct(2,1),
                             direct(0,2), direct(1,2), direct(2,2) );

         inverse(3,0) = 0;
         inverse(3,1) = 0;
         inverse(3,2) = 0;
         inverse(3,3) = 1;

         S inv_det = 1 / det;
         for (size_t i = 0; i < 3; ++i)
            for (size_t j = 0; j < 4; ++j)
               inverse(i,j) *= inv_det;
      }
      else
      {
         if (state == ss_unscaled)
         {
            for (size_t i = 0; i < 3; ++i)
               for (size_t j = 0; j < 3; ++j)
                  inverse(i, j) = direct(j, i);
         }
         else
         {
            // state_ == ss_equal_scaled
            S ss = 1 / (sqr(direct(0,0)) + sqr(direct(1,0)) + sqr(direct(2,0)));

            for (size_t i = 0; i < 3; ++i)
               for (size_t j = 0; j < 3; ++j)
                  inverse(i, j) = direct(j, i) * ss;
         }
         // calculate Translation
         for (size_t i = 0; i < 3; ++i)
         {
            inverse(3, i) = 0;
            inverse(i, 3) = -(inverse(i, 0) * direct(0, 3) + inverse(i, 1) * direct(1, 3) + inverse(i, 2) * direct(2, 3));
         }
         inverse(3, 3) = 1;
      }
   }

   template<typename S>
   void invert_transform_matrix ( matrix_t<S,3> const& direct, matrix_t<S,3>& inverted, unsigned state )
   {
      if ( state == ss_scaled )
      {
         inverted(0,0) =  direct(1,1);
         inverted(1,0) = -direct(1,0);
         inverted(0,1) = -direct(0,1);
         inverted(1,1) =  direct(0,0);
         inverted(0,2) =   determinant ( direct(0,1), direct(0,2),
            direct(1,1), direct(1,2) );
         inverted(1,2) = - determinant ( direct(0,0), direct(0,2),
            direct(1,0), direct(1,2) );

         S det =    determinant ( direct(0,0), direct(1,0),
            direct(0,1), direct(1,1) );

         inverted(2,0) = 0;
         inverted(2,1) = 0;
         inverted(2,2) = 1;

         S inv_det = 1 / det;
         for ( size_t i = 0 ; i < 2 ; i ++ )
            for ( size_t j = 0 ; j < 3 ; j ++ )
               inverted(i,j) *= inv_det;
      }
      else
      {
         if ( state == ss_unscaled )
         {
            for ( size_t i = 0 ; i < 2 ; i++ )
               for ( size_t j = 0 ; j < 2 ; j++ )
                  inverted(i,j) = direct(j,i);
         }
         else
         {
            // state_ == ss_equal_scaled
            S ss = 1 / (sqr(direct(0,0)) + sqr(direct(1,0)));

            for ( size_t i = 0 ; i < 2 ; i++ )
               for ( size_t j = 0 ; j < 2 ; j++ )
                  inverted(i,j) = direct(j,i) * ss ;
         }
         // calculate Translation
         for ( size_t i = 0 ; i < 2 ; i++ )
         {
            inverted(2,i) = 0;
            inverted(i,2) = -(inverted(i,0) * direct(0,2) + inverted(i,1) * direct(1,2) );
         }
         inverted(2,2) = 1;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   // operations

   template< typename S, size_t N >
   __forceinline transform_t<S, N> operator*( transform_t<S, N> const & t1, transform_t<S, N> const & t2 )
   {
      return t1.treat_transform(t2);
   }

   // Post
   template< typename S, size_t N >
   __forceinline transform_t<S, N> operator*( transform_t<S, N> const & t, typename transform_t<S, N>::rotation_t const & r )
   {
      return transform_t<S, N>(t).rotate(r, true);
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> operator*( transform_t<S, N> const & t, typename transform_t<S, N>::scale_t const & s )
   {
      return transform_t<S, N>(t).scale(s, true);
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> operator*( transform_t<S, N> const & tr, typename transform_t<S, N>::translation_t const & t )
   {
      return transform_t<S, N>(tr).translate(t, true);
   }

   // Pre
   template< typename S, size_t N >
   __forceinline transform_t<S, N> operator*( typename transform_t<S, N>::rotation_t const & r, transform_t<S, N> const & t )
   {
      return transform_t<S, N>(t).rotate(r, false);
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> operator*( typename transform_t<S, N>::scale_t const & s, transform_t<S, N> const & t )
   {
      return transform_t<S, N>(t).scale(s, false);
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> operator*( typename transform_t<S, N>::translation_t const & t, transform_t<S, N> const & tr )
   {
      return transform_t<S, N>(tr).translate(t, false);
   }

   // treating
   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::point_t operator*( transform_t<S, N> const & t, typename transform_t<S, N>::point_t const & p )
   {
      return t.treat_point(p, true);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::vector_t operator*( transform_t<S, N> const & t, typename transform_t<S, N>::vector_t const & v )
   {
      return t.treat_vector(v, true);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::normal_t operator*( transform_t<S, N> const & t, typename transform_t<S, N>::normal_t const & n )
   {
      return t.treat_normal(n, true);
   }

   // back treating
   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::point_t operator*( typename transform_t<S, N>::point_t const & p, transform_t<S, N> const & t )
   {
      return t.treat_point(p, false);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::vector_t operator*( typename transform_t<S, N>::vector_t const & v, transform_t<S, N> const & t )
   {
      return t.treat_vector(v, false);
   }

   template< typename S, size_t N >
   __forceinline typename transform_t<S, N>::normal_t operator*( typename transform_t<S, N>::normal_t const & n, transform_t<S, N> const & t )
   {
      return t.treat_normal(n, false);
   }

   // interpolation
   template<class S>
   __forceinline transform_t<S, 4> blend( transform_t<S, 4> const & tr1, transform_t<S, 4> const & tr2, S t )
   {
      typedef typename transform_t<S, 4>::point_t point_t ;
      typedef quaternion_t<S>                     quaternion_t ;

      point_t      const t1 = tr1.translation();
      point_t      const s1 = tr1.scale();
      quaternion_t const q1 = tr1.rotation().quaternion() ;

      point_t      const t2 = tr2.translation();
      point_t      const s2 = tr2.scale();
      quaternion_t const q2 = tr2.rotation().quaternion() ;

      return transform_t<S, 4>(translation(blend(t1, t2, t)), blend(q1, q2, t).cpr(), scale(blend(s1, s2, t)));
   }

   template< class S >
   __forceinline transform_t<S, 3> blend( transform_t<S, 3> const & tr1, transform_t<S, 3> const & tr2, S t )
   {
      typedef typename transform_t<S, 3>::point_t point_t;

      point_t const t1 = tr1.translation();
      point_t const s1 = tr1.scale();
      S       const a1 = tr1.rotation().angle();

      point_t const t2 = tr2.translation();
      point_t const s2 = tr2.scale();
      S       const a2 = tr2.rotation().angle();

      S const a = norm180(blend(a1, a1 + norm180(a2 - a1), t)); // m.b. add geom::blend_angles function ???

      return transform_t<S, 3>(translation(blend(t1, t2, t)), a, scale(blend(s1, s2, t)));
   }

   template< typename S, size_t N >
   __forceinline transform_t<S, N> slerp( transform_t<S, N> const & tr1, transform_t<S, N> const & tr2, double t )
   {
      return blend(tr1, tr2, 1 - t);
   }
}

