/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "stddefs.h"

// delete everywhere #define _HAS_ITERATOR_DEBUGGING  <value>
//                   #define _SECURE_SCL              <value>

//#ifdef _STD_BEGIN
//#   error "don't include any stl files before stl.h"
//#endif
//
//#ifdef _ITERATOR_DEBUG_LEVEL
//#   error "don't define _ITERATOR_DEBUG_LEVEL"
//#endif
//
//#ifdef _HAS_ITERATOR_DEBUGGING
//#   error "don't define _HAS_ITERATOR_DEBUGGING"
//#endif
//
//#ifdef _SECURE_SCL
//#   error "don't define _SECURE_SCL"
//#endif
//
//// define first iterator debug level in debug mode - secure_scl
//// has_iterator_debugging is 2nd debug level, which is too damn slow
//#ifdef _DEBUG
//#   define _SECURE_SCL 1
//#else
//#   define _SECURE_SCL 0
//#endif
