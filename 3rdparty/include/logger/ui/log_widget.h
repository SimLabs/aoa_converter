/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "log_widget_fwd.h"
#include "logger/ui/logger_ui.h"
#include "common/event.h"
#include "application/widget.h"
#include "application/widget_with_layout.h"

class QSettings;

namespace logging
{

struct log_widget
    : app::widget
    , app::widget_with_layout
{
    virtual void set_logger(net_layer::net_logger *logger) = 0;
    virtual void set_standalone_mode(bool standalone) = 0;
    virtual void set_follow_mode(bool follow) = 0;
    virtual void clear() = 0;

    virtual void restore_layout(QSettings const& settings) override = 0;
    virtual void save_layout   (QSettings      & settings) override = 0;

    virtual ~log_widget() {}

    DECLARE_EVENT(error_received, ()) ;
} ;


LOGWIDGET_API log_widget_ptr create_log_widget();

} // end of namespace logging

