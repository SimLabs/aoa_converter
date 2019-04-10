/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "point.h"

#include "geometry/half.h"

namespace geom
{

#pragma push_macro ( "DECLARE_EQUAL" )
#pragma push_macro ( "DECLARE_LESS" )
#pragma push_macro ( "DECLARE_FLOATING" )

#define DECLARE_FLOATING(S) template <> struct is_floating_point_f<S>  { static const bool value = true; };
#define DECLARE_EQUAL(S)    template <> struct max_type_f <S, S> { typedef S type ; } ;
#define DECLARE_LESS(L,G)   template <> struct max_type_f <L, G> { typedef G type ; } ; template <> struct max_type_f <G, L> { typedef G type ; } ;

DECLARE_EQUAL    (half)
DECLARE_FLOATING (half)

DECLARE_LESS (half, float)
DECLARE_LESS (half, double)

typedef point_t<half, 2> point_2h;
typedef point_t<half, 3> point_3h;
typedef point_t<half, 4> point_4h;

#pragma pop_macro ( "DECLARE_FLOATING" )
#pragma pop_macro ( "DECLARE_LESS" )
#pragma pop_macro ( "DECLARE_EQUAL" )

}
