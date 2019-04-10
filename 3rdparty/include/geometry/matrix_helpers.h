/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once


#include "primitives/matrix_fwd.h"
#include "primitives/turn_fwd.h"

namespace geom
{

//
// projection matrices
//

// orthographic projection matrix
template<typename scalar>
__forceinline matrix_t<scalar,4> matrix_ortho(
    const scalar left  , const scalar right,
    const scalar bottom, const scalar top,
    const scalar znear , const scalar zfar )
{
    const scalar
        sx = 1 / (right - left), tx = -sx * (right + left),
        sy = 1 / (top - bottom), ty = -sy * (bottom + top),
        sz = geom::eq(zfar, znear) ? 0 : (1 / (zfar - znear)),
        tz = -sz * (zfar + znear);

    const scalar data[4][4] =
    {
        {+2 * sx,       0,       0, tx},
        {      0, +2 * sy,       0, ty},
        {      0,       0, -2 * sz, tz},
        {      0,       0,       0,  1},
    };

    return matrix_t<scalar,4>((const scalar *)data);
}

// frustum projection matrix (near = 0, far = 1)
template<typename scalar>
__forceinline matrix_t<scalar,4> matrix_frustum(
    const scalar left  , const scalar right,
    const scalar bottom, const scalar top,
    const scalar znear , const scalar zfar )
{
    const scalar
        dxinv = 1 / (right - left),
        dyinv = 1 / (top - bottom),
        dzinv = 1 / (zfar - znear),
        a = (right + left) * dxinv,
        b = (bottom + top) * dyinv,
        c = -(zfar + znear) * dzinv,
        d = -2 * zfar * znear * dzinv,
        e = 2 * znear * dxinv,
        f = 2 * znear * dyinv;

    const scalar data[4][4] = 
    {
        { e,  0,  a,  0},
        { 0,  f,  b,  0},
        { 0,  0,  c,  d},
        { 0,  0, -1,  0},
    };

    return matrix_t<scalar,4>((const scalar *)data);
}

// frustum reversed projection matrix (near = 1, far = 0)
template<typename scalar>
__forceinline matrix_t<scalar,4> matrix_frustum_reversed(
    const scalar left  , const scalar right,
    const scalar bottom, const scalar top,
    const scalar znear , const scalar zfar )
{
    const scalar
        dxinv = 1 / (right - left),
        dyinv = 1 / (top - bottom),
        dzinv = 1 / (znear - zfar),
        a = (right + left) * dxinv,
        b = (bottom + top) * dyinv,
        c = -(zfar + znear) * dzinv,
        d = -2 * zfar * znear * dzinv,
        e = 2 * znear * dxinv,
        f = 2 * znear * dyinv;

    const scalar data[4][4] = 
    {
        { e,  0,  a,  0},
        { 0,  f,  b,  0},
        { 0,  0,  c,  d},
        { 0,  0, -1,  0},
    };

    return matrix_t<scalar,4>((const scalar *)data);
}

// eye projection matrix
template<typename scalar>
__forceinline matrix_t<scalar,4> matrix_eye_fov(
    const scalar tanH   , const scalar tanV,
    const scalar defTanH, const scalar defTanV,
    const scalar znear  , const scalar zfar )
{
    return matrix_frustum(
        znear * (-tanH + defTanH),
        znear * ( tanH + defTanH),
        znear * (-tanV + defTanV),
        znear * ( tanV + defTanV),
        znear, zfar);
}


//
// Viewing matrices
//

// world-to-camera matrix ((side,up,dir) must be right-handed, so direction looks backward)
template<typename scalar>
__forceinline matrix_t<scalar,4> matrix_camera(
    const point_t<scalar,3> & pos,
    const point_t<scalar,3> & dir,
    const point_t<scalar,3> & up,
    const point_t<scalar,3> & side )
{
    const scalar data[4][4] =
    {
        { side.x, side.y, side.z, -(pos * side) },
        {   up.x,   up.y,   up.z, -(pos *   up) },
        {  dir.x,  dir.y,  dir.z, -(pos *  dir) },
        {      0,      0,      0,             1 },
    };

    return matrix_t<scalar,4>((const scalar *)data);
}

// world-to-camera matrix (make fancy turn for Y to become -Z and visa versa)
template<typename scalar>
__forceinline matrix_t<scalar,4> matrix_camera(
    const point_t<scalar,3> & pos,
    const cpr_t<scalar> & orien )
{
    typedef matrix_t<scalar, 3> matrix_type;
    typedef point_t<scalar, 3> point_type;
    typedef rotation_t<scalar, 3> rotation_type;

    const matrix_type m = rotation_type(orien).matrix();

    const point_type dir  = -point_type(m.get_col(1));
    const point_type up   =  m.get_col(2);
    const point_type side =  m.get_col(0);

    return matrix_camera(pos, dir, up, side);
}

// look-at world-to-camera matrix
template<typename scalar>
__forceinline matrix_t<scalar,4> matrix_look_at(
    const point_t<scalar,3> & pos,
    const point_t<scalar,3> & look,
    const point_t<scalar,3> & upward = point_t<scalar,3>(0, 0, 1) )
{
    const point_t<scalar,3>
        dir = normalized(pos - look),
        side = normalized(upward ^ dir),
        up = dir ^ side;

    return matrix_camera(pos, dir, up, side);
}


//
// Texture coordinates matrices
//

template<typename scalar>
__forceinline matrix_t<scalar,4> xyzw_to_strq_remap_matrix( scalar w, scalar h )
{
    const geom::point_t<scalar, 3> scale(scalar(0.5) * w, scalar(0.5) * h, scalar(0.5));
    const geom::point_t<scalar, 3> bias(scale);
    const scalar data[4][4] =
    {
        { scale.x,       0,       0, bias.x },
        {       0, scale.y,       0, bias.y },
        {       0,       0, scale.z, bias.z },
        {       0,       0,       0,      1 },
    };

    return matrix_t<scalar,4>((const scalar *)data);
}

/*
template<typename scalar>
__forceinline matrix_t<scalar,4> xyzw_to_strq_remap_matrix( const geom::point_t<S, 2 > & leftBottom,
    const geom::point_t<S, 2 > & rightTop )
{
    BOOST_STATIC_ASSERT(!boost::is_integral<S>::value);

    const S width = rightTop.x - leftBottom.x;
    const S height = rightTop.y - leftBottom.y;

    const S xSum = rightTop.x + leftBottom.x;
    const S ySum = rightTop.y + leftBottom.y;

    S const data[4][4] =
    {
        { (S)0.5 * width,               0,      0, (S)0.5 * (xSum)},
        {              0, (S)0.5 * height,      0, (S)0.5 * (ySum)},
        {              0,               0, (S)0.5,          (S)0.5},
        {              0,               0,      0,               1},
    };

    return matrix_t<scalar,4>((const scalar *)data);
}
*/

}
