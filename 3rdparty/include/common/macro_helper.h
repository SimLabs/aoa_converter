/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#ifdef _WIN32

#  define MACRO_FALSE_CONDITION (0, 0)

#elif __linux__

#  define MACRO_FALSE_CONDITION (0)

#else
#  error "Unsupported platform"
#endif
