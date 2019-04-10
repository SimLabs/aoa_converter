/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "common/event.h"
#include "logger/logger.hpp"
#include "async_services/async_services.h"
#include "net_layer/decl_net_layer.h"

namespace net_layer
{
struct log_entry
{
    boost::asio::ip::address_v4 host;
    string app;
    logging::level level;
    uint64_t time;
    string source;
    string file;
    string log;
};

struct net_logger
{
    DECLARE_EVENT(new_log_entry, (log_entry const&));

protected:
    ~net_logger() {}
};


NET_LAYER_API logging::writer_ptr create_network_log_writer(std::string const& remote_endpoint, string const& app);

} // namespace logging
