/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
   
#ifdef CG_PRIMITIVES
#error "primitives_ext.h was included while CG_PRIMITIVES macro is defined"
#endif

namespace geom
{

// forwards
template < class, size_t >  struct point_t;

struct geo_point_2 ;
struct geo_point_3 ;

template<typename S>        struct color_t;
template<typename S>        struct colora_t;

template < class >          struct range_t;
template < class, size_t >  struct rectangle_t;

template < class >          struct cpr_t;

template < class >          struct frustum_perspective_t;

template < class >          struct transform_3t;
template < class >          struct transform_4t;

template < class >          struct matrix_4t;

template < class >          struct camera_t;

template < class >          struct frustum_clipper_t;

template < class >          struct rotation_3t;

// types
typedef     point_t< double,           2 >       point_2;
typedef     point_t< float,            2 >       point_2f;
typedef     point_t< char,             2 >       point_2c;
typedef     point_t< short,            2 >       point_2s;
typedef     point_t< int,              2 >       point_2i;
typedef     point_t< long,             2 >       point_2l;
typedef     point_t< unsigned char ,   2 >       point_2b;
typedef     point_t< unsigned char ,   2 >       point_2uc;
typedef     point_t< unsigned short,   2 >       point_2us;
typedef     point_t< unsigned int,     2 >       point_2ui;
typedef     point_t< unsigned long,    2 >       point_2ul;

typedef     point_t< double,           3 >       point_3;
typedef     point_t< float,            3 >       point_3f;
typedef     point_t< char,             3 >       point_3c;
typedef     point_t< short,            3 >       point_3s;
typedef     point_t< int,              3 >       point_3i;
typedef     point_t< long,             3 >       point_3l;
typedef     point_t< unsigned char ,   3 >       point_3b;
typedef     point_t< unsigned char ,   3 >       point_3uc;
typedef     point_t< unsigned short,   3 >       point_3us;
typedef     point_t< unsigned int,     3 >       point_3ui;
typedef     point_t< unsigned long,    3 >       point_3ul;

typedef     point_t< double,           4 >       point_4;
typedef     point_t< float,            4 >       point_4f;
typedef     point_t< char,             4 >       point_4c;
typedef     point_t< short,            4 >       point_4s;
typedef     point_t< int,              4 >       point_4i;
typedef     point_t< long,             4 >       point_4l;
typedef     point_t< unsigned char ,   4 >       point_4b;
typedef     point_t< unsigned char ,   4 >       point_4uc;
typedef     point_t< unsigned short,   4 >       point_4us;
typedef     point_t< unsigned int,     4 >       point_4ui;
typedef     point_t< unsigned long,    4 >       point_4ul;

typedef     color_t<unsigned char>               colorb ;
typedef     color_t<float>                       colorf ;

typedef     colora_t<unsigned char>              colorab ;
typedef     colora_t<float>                      coloraf ;

typedef     range_t< double         >           range_2;
typedef     range_t< float          >           range_2f;
typedef     range_t< int            >           range_2i;
typedef     range_t< unsigned char  >           range_2b;
typedef     range_t< unsigned short >           range_2us;
typedef     range_t< short          >           range_2s;
typedef     range_t<unsigned int    >           range_2ui;

typedef     rectangle_t< double,           2 >       rectangle_2;
typedef     rectangle_t< float,            2 >       rectangle_2f;
typedef     rectangle_t< short,            2 >       rectangle_2s;
typedef     rectangle_t< int,              2 >       rectangle_2i;
typedef     rectangle_t< long,             2 >       rectangle_2l;
typedef     rectangle_t< unsigned char ,   2 >       rectangle_2b;
typedef     rectangle_t< unsigned short,   2 >       rectangle_2us;
typedef     rectangle_t< unsigned,         2 >       rectangle_2ui;
typedef     rectangle_t< unsigned long,    2 >       rectangle_2ul;

typedef     rectangle_t< double,           3 >       rectangle_3;
typedef     rectangle_t< float,            3 >       rectangle_3f;
typedef     rectangle_t< short,            3 >       rectangle_3s;
typedef     rectangle_t< int,              3 >       rectangle_3i;
typedef     rectangle_t< long,             3 >       rectangle_3l;
typedef     rectangle_t< unsigned char ,   3 >       rectangle_3b;
typedef     rectangle_t< unsigned short,   3 >       rectangle_3us;
typedef     rectangle_t< unsigned,         3 >       rectangle_3ui;
typedef     rectangle_t< unsigned long,    3 >       rectangle_3ul;

typedef     rectangle_t< double,           4 >       rectangle_4;
typedef     rectangle_t< float,            4 >       rectangle_4f;
typedef     rectangle_t< short,            4 >       rectangle_4s;
typedef     rectangle_t< int,              4 >       rectangle_4i;
typedef     rectangle_t< long,             4 >       rectangle_4l;
typedef     rectangle_t< unsigned char ,   4 >       rectangle_4b;
typedef     rectangle_t< unsigned short,   4 >       rectangle_4us;
typedef     rectangle_t< unsigned,         4 >       rectangle_4ui;
typedef     rectangle_t< unsigned long,    4 >       rectangle_4ul;

typedef     cpr_t<float>                             cprf;
typedef     cpr_t<double>                            cpr;


typedef     frustum_perspective_t<double>           frustum_perspective;
typedef     frustum_perspective_t<float >           frustum_perspective_f;

typedef     transform_3t         <double>           transform_3 ;
typedef     transform_3t         <float >           transform_3f;

typedef     transform_4t         <double>           transform_4 ;
typedef     transform_4t         <float >           transform_4f;

typedef     matrix_4t            <double>           matrix_4;
typedef     matrix_4t            <float >           matrix_4f;

typedef     camera_t             <double>           camera;
typedef     camera_t             <float >           camera_f;

typedef     frustum_clipper_t    <float>            frustum_clipper;

typedef     rotation_3t          <float>            rotation_3f;
typedef     rotation_3t          <double>           rotation_3;

//////////////////////////////////////////////////////////////////////////
// functions

inline double feet2meter()      { return 0.3048; }
inline double meter2feet()      { return 1. / feet2meter(); }
inline double kt2mps()          { return 0.514444444444444; }

template <class type>   type    epsilon         () { return type(); }
template <> inline      double  epsilon<double> () { return 1e-10; }
template <> inline      float   epsilon<float > () { return 1e-6f; }

template <class type>   bool eq (type const& lhs, type const& rhs) { return lhs == rhs; }
inline                  bool eq (float       lhs, float       rhs) { return fabs(rhs - lhs) < epsilon<double>(); }
inline                  bool eq (double      lhs, double      rhs) { return fabs(rhs - lhs) < epsilon<float >(); }

template <class type>   bool eq_zero(type const& value) { return eq(value, type()); }


inline colorab color_red    ();
inline colorab color_green  ();
inline colorab color_orange ();
inline colorab color_random ();

}

namespace geom
{

// -------------------------------------------------------------- points

template < class Scalar >
struct point_t< Scalar, 2 >
{
    point_t( );
    point_t( Scalar x, Scalar y );

    Scalar   x;
    Scalar   y;
};

template < class Scalar >
struct point_t< Scalar, 3 >
    : point_t< Scalar, 2 >
{
    point_t( );
    point_t( Scalar x, Scalar y, Scalar z );

    Scalar   z;
};

template < class Scalar >
struct point_t< Scalar, 4 >
    : point_t< Scalar, 3 >
{
    point_t( );
    point_t( Scalar x, Scalar y, Scalar z, Scalar w );

    Scalar   w;
};

template<class Scalar>  bool operator==(point_t<Scalar, 2> const& lhs, point_t<Scalar, 2> const& rhs) { return eq(lhs.x, rhs.x) && eq(lhs.y, rhs.y); }
template<class Scalar>  bool operator==(point_t<Scalar, 3> const& lhs, point_t<Scalar, 3> const& rhs) { return eq(lhs.x, rhs.x) && eq(lhs.y, rhs.y) && eq(lhs.z, rhs.z); }
template<class Scalar>  bool operator==(point_t<Scalar, 4> const& lhs, point_t<Scalar, 4> const& rhs) { return eq(lhs.x, rhs.x) && eq(lhs.y, rhs.y) && eq(lhs.z, rhs.z) && eq(lhs.w, rhs.w); }

// ---------------------------------------------------------------- geo_points

struct geo_point_2
{
    double lat, lon ;

    geo_point_2 ()                         : lat ( 0     ), lon ( 0     ) {}
    geo_point_2 ( double lat, double lon ) : lat ( lat   ), lon ( lon   ) {}
} ;

struct geo_point_3 : geo_point_2
{
    double height ;

    geo_point_3 ()                                             : height ( 0 ) {}
    geo_point_3 ( double lat, double lon, double height = 0. ) : geo_point_2(lat, lon), height(height) {}
    geo_point_3 ( geo_point_2 const& pos, double height = 0. ) : geo_point_2(pos), height(height)      {}
} ;

// ---------------------------------------------------------------- polar_points

struct polar_point_2
{
    double range, course;

    polar_point_2(double range = 1, double course = 0) : range(range), course(course) {}
} ;


// ---------------------------------------------------------------- colors
template<typename S>
struct color_t
{
    S r, g, b;

    color_t() ;
    color_t( S r, S g, S b ) ;
};

template<typename S>
struct colora_t : color_t<S>
{
    S a;

    colora_t() ;
    colora_t( S r, S g, S b , S a = S(1)) ;
    colora_t( color_t<S> const&, S a ) ;
};

// ---------------------------------------------------------------- range
template <class S>
struct range_t
{
    S lo_, hi_ ;

    range_t() ;
    range_t( S t1, S t2 ) ;

    S lo() const { return lo_; }
    S hi() const { return hi_; }
};

// ---------------------------------------------------------------- rectangle
template <class S>
struct rectangle_t<S, 2>
{
    range_t<S> x;
    range_t<S> y;

    rectangle_t();
    rectangle_t( range_t<S> const& x, range_t<S> const& y );
};

template <class S>
struct rectangle_t<S, 3>
{
    range_t<S> z;

    rectangle_t();
    rectangle_t( range_t<S> const& x, range_t<S> const& y, range_t<S> const& z );
};

template <class S>
struct rectangle_t<S, 4>
{
    range_t<S> w;

    rectangle_t();
    rectangle_t( range_t<S> const& x, range_t<S> const& y, range_t<S> const& z, range_t<S> const& w );
};


// func impls
template <class scalar> scalar rand(scalar max) { return scalar(max * ::rand() / (scalar)RAND_MAX); }

inline colorab color_red    () { return colorab( 255, 0, 0 ) ; }
inline colorab color_green  () { return colorab( 0, 255, 0 ) ; }
inline colorab color_orange () { return colorab( 255, 127, 0 ) ; }
inline colorab color_random () { return colorab( rand<unsigned char>(255), rand<unsigned char>(255), rand<unsigned char>(255)); }

}

namespace geom
{

// -------------------------------------------------------------- points

template < class Scalar >
point_t< Scalar, 2 > :: point_t( )
    : x( 0 )
    , y( 0 )
{}

template < class Scalar >
point_t< Scalar, 2 > :: point_t( Scalar x, Scalar y )
    : x( x )
    , y( y )
{}

template < class Scalar >
point_t< Scalar, 3 > :: point_t( )
    : z( 0 )
{}

template < class Scalar >
point_t< Scalar, 3 > :: point_t( Scalar x, Scalar y, Scalar z )
    : point_t< Scalar, 2 >( x, y )
    , z( z )
{}

template < class Scalar >
point_t< Scalar, 4 > :: point_t( )
    : w( 0 )
{}

template < class Scalar >
point_t< Scalar, 4 > :: point_t( Scalar x, Scalar y, Scalar z, Scalar w )
    : point_t< Scalar, 3 >( x, y, z )
    , w( w )
{}

// ---------------------------------------------------------------- colors
template<typename S>
color_t<S>::color_t ()
    : r( 0 )
    , g( 0 )
    , b( 0 )
{
}

template<typename S>
color_t<S>::color_t ( S r, S g, S b )
    : r( r )
    , g( g )
    , b( b )
{
}

template<typename S>
colora_t<S>::colora_t ()
    : a( 0 )
{
}

template<typename S>
colora_t<S>::colora_t ( S r, S g, S b, S a)
    : color_t<S>( r, g, b )
    , a( a )
{
}

template<typename S>
colora_t<S>::colora_t(color_t<S> const& other, S a) 
    : color_t<S>( other )
    , a         ( a )
{
}

// ---------------------------------------------------------------- range
template <class S>
range_t<S>::range_t()
    : lo_(1)
    , hi_(0)
{}

template <class S>
range_t<S>::range_t( S lo, S hi )
    : lo_( lo )
    , hi_( hi )
{}

// ---------------------------------------------------------------- rectangle
template <class S>
rectangle_t<S, 2>::rectangle_t()
{}

template <class S>
rectangle_t<S, 2>::rectangle_t( range_t<S> const& x, range_t<S>  const& y )
    : x ( x )
    , y ( y )
{}

template <class S>
rectangle_t<S, 3>::rectangle_t()
{}

template <class S>
rectangle_t<S, 3>::rectangle_t( range_t<S> const& x, range_t<S>  const& y, range_t<S> const& z )
    : rectangle_t<S, 2>(x, y)
    , z ( z )
{}

template <class S>
rectangle_t<S, 4>::rectangle_t()
{}

template <class S>
rectangle_t<S, 4>::rectangle_t( range_t<S> const& x, range_t<S> const& y, range_t<S> const& z, range_t<S> const& w )
    : rectangle_t<S, 3>(x, y, z)
    , w ( w )
{}

template<typename scalar>
struct cpr_t
{
    scalar course, pitch, roll ;

    cpr_t( scalar c = 0, scalar p = 0, scalar r = 0) ;

    template<typename _scalar>
    cpr_t( cpr_t<_scalar> const& other ) ;
} ;

} // geom

//
using geom::point_2;
using geom::point_2f;
using geom::point_2c;
using geom::point_2s;
using geom::point_2i;
using geom::point_2l;
using geom::point_2b;
using geom::point_2uc;
using geom::point_2us;
using geom::point_2ui;
using geom::point_2ul;

//
using geom::point_3;
using geom::point_3f;
using geom::point_3c;
using geom::point_3s;
using geom::point_3i;
using geom::point_3l;
using geom::point_3b;
using geom::point_3uc;
using geom::point_3us;
using geom::point_3ui;
using geom::point_3ul;

//
using geom::point_4;
using geom::point_4f;
using geom::point_4c;
using geom::point_4s;
using geom::point_4i;
using geom::point_4l;
using geom::point_4b;
using geom::point_4uc;
using geom::point_4us;
using geom::point_4ui;
using geom::point_4ul;

//
using geom::polar_point_2 ;

//
using geom::cpr;
using geom::cprf;

//
using geom::matrix_4;
using geom::matrix_4f;

//
using geom::transform_4 ;
using geom::transform_4f ;

//
using geom::geo_point_2 ;
using geom::geo_point_3 ;

//
using geom::range_2f;
using geom::range_2i;
using geom::range_2;
using geom::range_2ui;

//
using geom::rectangle_2f;
using geom::rectangle_2i;
using geom::rectangle_2ui;
using geom::rectangle_2;
using geom::rectangle_3f;
using geom::rectangle_3i;
using geom::rectangle_3ui;
using geom::rectangle_3;

//
using geom::colorf;
using geom::colorb;
using geom::coloraf;
using geom::colorab;
