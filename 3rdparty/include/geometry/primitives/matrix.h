/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "common/no_deduce.h"
#include <geometry/common.h>

#include "point.h"

#include "matrix_fwd.h"

namespace geom
{
   template<typename scalar, int N> struct matrix_t ;

   // matrix_t operations
   // point_t operator * ( matrix_t  const& m , point_t const& v  ) ;
   // point_t operator * ( point_t const& v , matrix_t  const& m  ) ; // treats v as row, col(row(v)*m) == !m*v
   // matrix_t  operator * ( matrix_t  const& m1, matrix_t  const& m2 ) ;
   // matrix_t  operator + ( matrix_t  const& m1, matrix_t  const& m2 ) ;
   // matrix_t  operator ! ( matrix_t  const& m ) ;
   // scalar determinant ( scalar m00, scalar m01, scalar m02, scalar m10, scalar m11, scalar m12, scalar m20, scalar m21, scalar m22 ) ;
   // scalar determinant ( scalar m00, scalar m01, scalar m10, scalar m11 ) ;
   // scalar determinant ( matrix_t const& m ) ;
}


namespace geom
{
   #pragma pack(push, 1)

   namespace matrix
   {
      struct undefined_tag    {};

      const undefined_tag    undefined = undefined_tag();
   }

   template<typename scalar, int N>
   struct matrix_t
   {
      matrix_t () ;
      matrix_t ( scalar diagonal ) ;
      matrix_t ( scalar const* rawdata ) ;
      matrix_t ( matrix::undefined_tag ) ;

      template<typename S>
         matrix_t ( matrix_t<S,N> const& other ) ;

      matrix_t ( matrix_t const& other ) ;
      matrix_t & operator = ( matrix_t const& other ) ;

      scalar const & operator () (size_t i, size_t j) const { return m_[i][j]; }
      scalar       & operator () (size_t i, size_t j)       { return m_[i][j]; }

      matrix_t& operator *= ( matrix_t const& m ) ;
      matrix_t& operator += ( matrix_t const& m ) ;

      point_t<scalar, N> get_col ( size_t i ) const ;
      void               put_col ( size_t i, point_t<scalar, N> const& col ) ;
      point_t<scalar, N> get_row ( size_t i ) const ;
      void               put_row ( size_t i, point_t<scalar, N> const& row ) ;

      matrix_t& transpose() ;

      bool is_zero() const ;
      bool is_identity() const ;

      scalar const* rawdata () const ;


   private :
      scalar m_[N][N];
   };

   #pragma pack(pop)
}


////////////////////////////////////////////////////////////////////////////////////////////
// implementation
namespace geom
{

   template<typename S, int N> __forceinline matrix_t<S,N>::matrix_t ()
   {
      memset(m_, 0, sizeof(m_));
   }

   template<typename S, int N> __forceinline matrix_t<S,N>::matrix_t ( S diag )
   {
      memset(m_, 0, sizeof(m_));
      for ( size_t i = 0 ; i != N ; ++i )
         m_[i][i] = diag ;
   }

   template<typename S, int N> __forceinline matrix_t<S,N>::matrix_t ( S const* rawdata )
   {
      memcpy(m_, rawdata, sizeof(m_));
   }

   template<typename S, int N> __forceinline matrix_t<S,N>::matrix_t ( matrix::undefined_tag )
   {
   }

   // conversion
   template<typename S, int N> template<typename _S> __forceinline matrix_t<S,N>::matrix_t ( matrix_t<_S,N> const& other )
   {
      for ( size_t i = 0 ; i < N ; ++i )
         for ( size_t j = 0 ; j < N ; ++j )
            m_[i][j] = (S)other(i,j) ;
   }

   // copy constructor
   template<typename S, int N> __forceinline matrix_t<S,N>::matrix_t ( matrix_t const& other )
   {
      memcpy(m_, other.m_, sizeof(m_));
   }

   // copy assignment
   template<typename S, int N> __forceinline matrix_t<S,N> & matrix_t<S,N>::operator = ( matrix_t<S,N> const& other )
   {
       if (this != &other)
           memcpy(m_, other.m_, sizeof(m_));
       return *this;
   }


   template<typename S, int N> __forceinline matrix_t<S,N>& matrix_t<S,N>::operator *= ( matrix_t<S,N> const& m )
   {
      *this = *this * m ;

      return *this ;
   }

   template<typename S, int N> __forceinline matrix_t<S,N>& matrix_t<S,N>::operator += ( matrix_t<S,N> const& m )
   {
      for ( size_t i = 0 ; i < N ; ++i )
         for ( size_t j = 0 ; j < N ; ++j )
            m_[ i ][ j ] += m.m_[ i ][ j ] ;

      return *this ;
   }

   template<typename S, int N> __forceinline point_t<S,N> matrix_t<S,N>::get_col ( size_t j ) const
   {
      point_t<S, N> r ;
      for ( size_t i = 0 ; i < N ; ++i )
         r[i] = m_[i][j] ;
      return r ;
   }

   template<typename S, int N> __forceinline void matrix_t<S,N>::put_col ( size_t j, point_t<S,N> const& col )
   {
      for ( size_t i = 0 ; i < N ; ++i )
         m_[i][j] = col[i] ;
   }

   template<typename S, int N> __forceinline point_t<S,N> matrix_t<S,N>::get_row ( size_t i ) const
   {
      point_t<S, N> r ;
      for ( size_t j = 0 ; j < N ; ++j )
         r[j] = m_[i][j] ;
      return r ;
   }

   template<typename S, int N> __forceinline void matrix_t<S,N>::put_row ( size_t j, point_t<S,N> const& row )
   {
      for ( size_t i = 0 ; i < N ; ++i )
         m_[j][i] = row[i] ;
   }

   template<typename S, int N>
   __forceinline matrix_t<S,N>& matrix_t<S,N>::transpose()
   {
      for ( size_t i = 0; i != N; ++i )
         for ( size_t j = i + 1; j < N; ++j )
            std::swap( m_[i][j], m_[j][i] );

      return *this ;
   }

   template<typename S, int N> __forceinline bool matrix_t<S,N>::is_zero() const
   {
       for ( unsigned i = 0; i != N; ++i )
           for ( unsigned j = 0; j < N; ++j )
               if (!eq_zero(m_[i][j], epsilon<float>()))
                   return false;
       return true;
   }

   template<typename S, int N> __forceinline bool matrix_t<S,N>::is_identity() const
   {
       for ( unsigned i = 0; i != N; ++i )
           for ( unsigned j = 0; j < N; ++j )
               if (!eq_zero(m_[i][j] - S(i == j), epsilon<float>()))
                   return false;
       return true;
   }

   template<typename S, int N> __forceinline S const* matrix_t<S,N>::rawdata () const
   {
      return &m_[0][0] ;
   }

   template<typename S> __forceinline point_t<S,2> operator * ( matrix_t<S,2> const& m, typename meta::no_deduce_f< point_t<S,2> > :: type const& v )
   {
      return point_t<S,2>( m(0,0) * v[0] + m(0,1) * v[1],
                           m(1,0) * v[0] + m(1,1) * v[1] );
   }

   template<typename S> __forceinline point_t<S,3> operator * ( matrix_t<S,3> const& m, typename meta::no_deduce_f< point_t<S,3> > :: type const& v )
   {
      return point_t<S,3>( m(0, 0) * v[0] + m(0, 1) * v[1] + m(0, 2) * v[2],
                           m(1, 0) * v[0] + m(1, 1) * v[1] + m(1, 2) * v[2],
                           m(2, 0) * v[0] + m(2, 1) * v[1] + m(2, 2) * v[2]);
   }

   template<typename S> __forceinline point_t<S,4> operator * ( matrix_t<S,4> const& m, typename meta::no_deduce_f< point_t<S,4> > :: type const& v )
   {
      return point_t<S,4>( m(0, 0) * v[0] + m(0, 1) * v[1] + m(0, 2) * v[2] + m(0, 3) * v[3],
                           m(1, 0) * v[0] + m(1, 1) * v[1] + m(1, 2) * v[2] + m(1, 3) * v[3],
                           m(2, 0) * v[0] + m(2, 1) * v[1] + m(2, 2) * v[2] + m(2, 3) * v[3],
                           m(3, 0) * v[0] + m(3, 1) * v[1] + m(3, 2) * v[2] + m(3, 3) * v[3] );
   }

   template<typename S> __forceinline point_t<S,2> operator * ( typename meta::no_deduce_f< point_t<S,2> > :: type const& v, matrix_t<S,2> const& m )
   {
      return point_t<S,2>( m(0,0) * v[0] + m(1,0) * v[1],
                           m(0,1) * v[0] + m(1,1) * v[1] );
   }

   template<typename S> __forceinline point_t<S,3> operator * ( typename meta::no_deduce_f< point_t<S,3> > :: type const& v, matrix_t<S,3> const& m )
   {
      return point_t<S,3>( m(0, 0) * v[0] + m(1, 0) * v[1] + m(2, 0) * v[2],
                           m(0, 1) * v[0] + m(1, 1) * v[1] + m(2, 1) * v[2],
                           m(0, 2) * v[0] + m(1, 2) * v[1] + m(2, 2) * v[2]);
   }

   template<typename S> __forceinline point_t<S,4> operator * ( typename meta::no_deduce_f< point_t<S,4> > :: type const& v, matrix_t<S,4> const& m )
   {
      return point_t<S,4>( m(0, 0) * v[0] + m(1, 0) * v[1] + m(2, 0) * v[2] + m(3, 0) * v[3],
                           m(0, 1) * v[0] + m(1, 1) * v[1] + m(2, 1) * v[2] + m(3, 1) * v[3],
                           m(0, 2) * v[0] + m(1, 2) * v[1] + m(2, 2) * v[2] + m(3, 2) * v[3],
                           m(0, 3) * v[0] + m(1, 3) * v[1] + m(2, 3) * v[2] + m(3, 3) * v[3] );
   }

   template<typename S, int N> __forceinline matrix_t<S,N> operator * ( matrix_t<S,N> const& m1, matrix_t<S,N> const& m2 )
   {
      matrix_t<S,N> r ;

      for ( size_t i = 0; i != N; i++ )
         for ( size_t j = 0; j != N; j++ )
            for ( size_t k = 0; k != N; k++ )
               r(i,j) += m1(i,k) * m2(k,j);

      return r;
   }

   // performance specializations
   template<typename S> __forceinline matrix_t<S,3> operator * ( matrix_t<S,3> const& m1, matrix_t<S,3> const& m2 )
   {
      matrix_t<S,3> r(matrix::undefined) ;

      for ( size_t i = 0; i < 3; i++ )
         for ( size_t j = 0; j < 3; j++ )
            r(i,j) = m1(i,0) * m2(0,j) + m1(i,1) * m2(1,j) + m1(i,2) * m2(2,j);

      return r ;
   }

   template<typename S> __forceinline matrix_t<S,3> operator + ( matrix_t<S,3> const& m1, matrix_t<S,3> const& m2 )
   {
      return matrix_t<S,3>( m1 ) += m2 ;
   }

   template<typename S> __forceinline matrix_t<S,4> operator * ( matrix_t<S,4> const& m1, matrix_t<S,4> const& m2 )
   {
      matrix_t<S,4> r(matrix::undefined) ;

      for ( size_t i = 0; i < 4; i++ )
         for ( size_t j = 0; j < 4; j++ )
            r(i,j) = m1(i,0) * m2(0,j) + m1(i,1) * m2(1,j) + m1(i,2) * m2(2,j) + m1(i,3) * m2(3,j);

      return r ;
   }

   template<typename S, int N> __forceinline matrix_t<S,N> operator ! ( matrix_t<S,N> const& m )
   {
      matrix_t<S,N> r ;

      for ( size_t i = 0; i != N; i++ )
         for ( size_t j = 0; j != N; j++ )
            r(i,j) = m(j,i) ;

      return r ;
   }

   // determinant
   template<typename S> __forceinline S determinant ( S m00, S m01,
                                                      S m10, S m11 )
   {
      return m00*m11 - m01*m10 ;
   }

   template<typename S> __forceinline S determinant ( S m00, S m01, S m02,
                                                     S m10, S m11, S m12,
                                                     S m20, S m21, S m22 )
   {
      return m00*m11*m22 + m01*m12*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 - m02*m11*m20;
   }

   template<typename S> __forceinline S determinant ( S m00, S m01, S m02, S m03,
                                                      S m10, S m11, S m12, S m13,
                                                      S m20, S m21, S m22, S m23,
                                                      S m30, S m31, S m32, S m33 )
   {
      return  m00*m11*m22*m33 + m00*m12*m23*m31 + m00*m13*m21*m32
             +m01*m10*m23*m32 + m01*m12*m20*m33 + m01*m13*m22*m30
             +m02*m10*m21*m33 + m02*m11*m23*m30 + m02*m13*m20*m31
             +m03*m10*m22*m31 + m03*m11*m20*m32 + m03*m12*m21*m30
             -m00*m11*m23*m32 - m00*m12*m21*m33 - m00*m13*m22*m31
             -m01*m10*m22*m33 - m01*m12*m23*m30 - m01*m13*m20*m32
             -m02*m10*m23*m31 - m02*m11*m20*m33 - m02*m13*m21*m30
             -m03*m10*m21*m32 - m03*m11*m22*m30 - m03*m12*m20*m31 ;
   }

   template<typename S, size_t N> S determinant ( matrix_t<S,N> const& m ) ;

   template<typename S> __forceinline S determinant ( matrix_t<S,2> const& m )
   {
      return determinant ( m(0,0), m(0,1),
                           m(1,0), m(1,1) ) ;
   }

   template<typename S> __forceinline S determinant ( matrix_t<S,3> const& m )
   {
      return determinant ( m(0,0), m(0,1), m(0,2),
                           m(1,0), m(1,1), m(1,2),
                           m(2,0), m(2,1), m(2,2) ) ;
   }

   template<typename S> __forceinline S determinant ( matrix_t<S,4> const& m )
   {
      return determinant ( m(0,0), m(0,1), m(0,2), m(0,3),
                           m(1,0), m(1,1), m(1,2), m(1,3),
                           m(2,0), m(2,1), m(2,2), m(2,3),
                           m(3,0), m(3,1), m(3,2), m(3,3) ) ;
   }

   // inverse
   template<typename S, size_t N> bool inverse ( matrix_t<S,N> const& m, matrix_t<S,N> & res ) ;

   template<typename S, size_t N> bool inverse ( matrix_t<S,N> & m )
   {
      return inverse(m, m);
   }

   template<typename S> __forceinline bool inverse ( matrix_t<S,2> const& m, matrix_t<S,2> & res )
   {
      S det = determinant(m);
      if (geom::eq_zero(det))
         return false;

      S detInv = (S)(1. / det);

      S m00 = m(0,0), m01 = m(0,1),
        m10 = m(1,0), m11 = m(1,1);

      res(0,0) =  m11 * detInv;
      res(0,1) = -m01 * detInv;
      res(1,0) = -m10 * detInv;
      res(1,1) =  m00 * detInv;

      return true;
   }

   template<typename S> __forceinline bool inverse ( matrix_t<S,3> const& m, matrix_t<S,3> & res )
   {
      S det = determinant(m);
      if (geom::eq_zero(det))
         return false;

      S detInv = (S)(1. / det);

      S m00 = m(0,0), m01 = m(0,1), m02 = m(0,2),
        m10 = m(1,0), m11 = m(1,1), m12 = m(1,2),
        m20 = m(2,0), m21 = m(2,1), m22 = m(2,2);

      res(0,0) = (m11*m22 - m12*m21) * detInv;
      res(0,1) = (m02*m21 - m01*m22) * detInv;
      res(0,2) = (m01*m12 - m02*m11) * detInv;

      res(1,0) = (m12*m20 - m10*m22) * detInv;
      res(1,1) = (m00*m22 - m02*m20) * detInv;
      res(1,2) = (m02*m10 - m00*m12) * detInv;

      res(2,0) = (m10*m21 - m11*m20) * detInv;
      res(2,1) = (m01*m20 - m00*m21) * detInv;
      res(2,2) = (m00*m11 - m01*m10) * detInv;

      return true;
   }

   template<typename S> bool inverse ( matrix_t<S,4> const& m, matrix_t<S,4> & res )
   {
      S det = determinant(m);
      if (geom::eq_zero(det))
         return false;

      S detInv = (S)(1. / det);

      S m00 = m(0,0), m01 = m(0,1), m02 = m(0,2), m03 = m(0,3),
        m10 = m(1,0), m11 = m(1,1), m12 = m(1,2), m13 = m(1,3),
        m20 = m(2,0), m21 = m(2,1), m22 = m(2,2), m23 = m(2,3),
        m30 = m(3,0), m31 = m(3,1), m32 = m(3,2), m33 = m(3,3);

      res(0,0) = (m11*m22*m33 + m12*m23*m31 + m13*m21*m32
                 -m11*m23*m32 - m12*m21*m33 - m13*m22*m31) * detInv;

      res(0,1) = (m01*m23*m32 + m02*m21*m33 + m03*m22*m31
                 -m01*m22*m33 - m02*m23*m31 - m03*m21*m32) * detInv;

      res(0,2) = (m01*m12*m33 + m02*m13*m31 + m03*m11*m32
                 -m01*m13*m32 - m02*m11*m33 - m03*m12*m31) * detInv;

      res(0,3) = (m01*m13*m22 + m02*m11*m23 + m03*m12*m21
                 -m01*m12*m23 - m02*m13*m21 - m03*m11*m22) * detInv;

      res(1,0) = (m10*m23*m32 + m12*m20*m33 + m13*m22*m30
                 -m10*m22*m33 - m12*m23*m30 - m13*m20*m32) * detInv;

      res(1,1) = (m00*m22*m33 + m02*m23*m30 + m03*m20*m32
                 -m00*m23*m32 - m02*m20*m33 - m03*m22*m30) * detInv;

      res(1,2) = (m00*m13*m32 + m02*m10*m33 + m03*m12*m30
                 -m00*m12*m33 - m02*m13*m30 - m03*m10*m32) * detInv;

      res(1,3) = (m00*m12*m23 + m02*m13*m20 + m03*m10*m22
                 -m00*m13*m22 - m02*m10*m23 - m03*m12*m20) * detInv;

      res(2,0) = (m10*m21*m33 + m11*m23*m30 + m13*m20*m31
                 -m10*m23*m31 - m11*m20*m33 - m13*m21*m30) * detInv;

      res(2,1) = (m00*m23*m31 + m01*m20*m33 + m03*m21*m30
                 -m00*m21*m33 - m01*m23*m30 - m03*m20*m31) * detInv;

      res(2,2) = (m00*m11*m33 + m01*m13*m30 + m03*m10*m31
                 -m00*m13*m31 - m01*m10*m33 - m03*m11*m30) * detInv;

      res(2,3) = (m00*m13*m21 + m01*m10*m23 + m03*m11*m20
                 -m00*m11*m23 - m01*m13*m20 - m03*m10*m21) * detInv;

      res(3,0) = (m10*m22*m31 + m11*m20*m32 + m12*m21*m30
                 -m10*m21*m32 - m11*m22*m30 - m12*m20*m31) * detInv;

      res(3,1) = (m00*m21*m32 + m01*m22*m30 + m02*m20*m31
                 -m00*m22*m31 - m01*m20*m32 - m02*m21*m30) * detInv;

      res(3,2) = (m00*m12*m31 + m01*m10*m32 + m02*m11*m30
                 -m00*m11*m32 - m01*m12*m30 - m02*m10*m31) * detInv;

      res(3,3) = (m00*m11*m22 + m01*m12*m20 + m02*m10*m21
                 -m00*m12*m21 - m01*m10*m22 - m02*m11*m20) * detInv;


      return true;
   }


   // inverse symmetric matrix
   template<typename S, size_t N> bool inverse_symmetric ( matrix_t<S,N> const& m, matrix_t<S,N> & res ) ;

   template<typename S, size_t N> bool inverse_symmetric ( matrix_t<S,N> & m )
   {
      return inverse_symmetric(m, m);
   }

   template<typename S> __forceinline bool inverse_symmetric ( matrix_t<S,2> const& m, matrix_t<S,2> & res )
   {
      S det = determinant(m);
      if (geom::eq_zero(det))
         return false;

      S detInv = (S)(1. / det);

      S m00 = m(0,0), m01 = m(0,1), m11 = m(1,1);

      res(0,0) = m11 * detInv;
      res(1,1) = m00 * detInv;
      res(1,0) = res(0,1) = -m01 * detInv;

      return true;
   }

   template<typename S> __forceinline bool inverse_symmetric ( matrix_t<S,3> const& m, matrix_t<S,3> & res )
   {
      S det = determinant(m);
      if (geom::eq_zero(det))
         return false;

      S detInv = (S)(1. / det);

      S m00 = m(0,0), m01 = m(0,1), m02 = m(0,2),
        m10 = m(1,0), m11 = m(1,1), m12 = m(1,2),
        m20 = m(2,0), m21 = m(2,1), m22 = m(2,2);

      res(0,0) = (m11*m22 - m12*m21) * detInv;
      res(1,1) = (m00*m22 - m02*m20) * detInv;
      res(2,2) = (m00*m11 - m01*m10) * detInv;
      res(1,0) = res(0,1) = (m02*m21 - m01*m22) * detInv;
      res(2,0) = res(0,2) = (m01*m12 - m02*m11) * detInv;
      res(2,1) = res(1,2) = (m02*m10 - m00*m12) * detInv;

      return true;
   }

   template<typename S> bool inverse_symmetric ( matrix_t<S,4> const& m, matrix_t<S,4> & res )
   {
      S det = determinant(m);
      if (geom::eq_zero(det))
         return false;

      S detInv = (S)(1. / det);

      S m00 = m(0,0), m01 = m(0,1), m02 = m(0,2), m03 = m(0,3),
        m10 = m(1,0), m11 = m(1,1), m12 = m(1,2), m13 = m(1,3),
        m20 = m(2,0), m21 = m(2,1), m22 = m(2,2), m23 = m(2,3),
        m30 = m(3,0), m31 = m(3,1), m32 = m(3,2), m33 = m(3,3);

      res(0,0) = (m11*m22*m33 + m12*m23*m31 + m13*m21*m32
                 -m11*m23*m32 - m12*m21*m33 - m13*m22*m31) * detInv;

      res(1,1) = (m00*m22*m33 + m02*m23*m30 + m03*m20*m32
                 -m00*m23*m32 - m02*m20*m33 - m03*m22*m30) * detInv;

      res(2,2) = (m00*m11*m33 + m01*m13*m30 + m03*m10*m31
                 -m00*m13*m31 - m01*m10*m33 - m03*m11*m30) * detInv;

      res(3,3) = (m00*m11*m22 + m01*m12*m20 + m02*m10*m21
                 -m00*m12*m21 - m01*m10*m22 - m02*m11*m20) * detInv;

      res(0,1) = res(1,0) = (m01*m23*m32 + m02*m21*m33 + m03*m22*m31
                            -m01*m22*m33 - m02*m23*m31 - m03*m21*m32) * detInv;

      res(0,2) = res(2,0) = (m01*m12*m33 + m02*m13*m31 + m03*m11*m32
                            -m01*m13*m32 - m02*m11*m33 - m03*m12*m31) * detInv;

      res(0,3) = res(3,0) = (m01*m13*m22 + m02*m11*m23 + m03*m12*m21
                            -m01*m12*m23 - m02*m13*m21 - m03*m11*m22) * detInv;

      res(1,2) = res(2,1) = (m00*m13*m32 + m02*m10*m33 + m03*m12*m30
                            -m00*m12*m33 - m02*m13*m30 - m03*m10*m32) * detInv;

      res(1,3) = res(3,1) = (m00*m12*m23 + m02*m13*m20 + m03*m10*m22
                            -m00*m13*m22 - m02*m10*m23 - m03*m12*m20) * detInv;

      res(2,3) = res(3,2) = (m00*m13*m21 + m01*m10*m23 + m03*m11*m20
                            -m00*m11*m23 - m01*m13*m20 - m03*m10*m21) * detInv;

      return true;
   }

}

