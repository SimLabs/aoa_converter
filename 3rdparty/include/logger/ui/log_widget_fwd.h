/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "common/dyn_lib.h"

namespace logging

{
struct log_widget;

typedef std::shared_ptr<log_widget> log_widget_ptr;

}

#ifdef LOG_WIDGET_LIB
# define LOGWIDGET_API __HELPER_DL_EXPORT
#else
# define LOGWIDGET_API __HELPER_DL_IMPORT
#endif

