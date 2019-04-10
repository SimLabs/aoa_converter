/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "async_services/net_common.h"
#include "common/process_state_defs.h"


namespace process_state
{

inline proc_id_t current_process_id();

/// Use with caution!! on windows calls SuspendThread on each thread (except the caller one)
/// this might lead to hanging the whole application
PROCESS_STATE_API  proc_stack_ptr      stacktrace       (proc_id_t process_id = current_process_id());
PROCESS_STATE_API  modules_vect_ptr    loaded_modules   (proc_id_t process_id = current_process_id());
PROCESS_STATE_API  string_ptr          to_string        (proc_stack const& stack);
PROCESS_STATE_API  string_ptr          to_string        (modules_vect const& modules);
PROCESS_STATE_API  string_ptr          to_string        (resp_state_msg const& msg_ptr);

/// Everything in bytes
PROCESS_STATE_API  mem_size_t   private_memory  (proc_id_t process_id = current_process_id());
PROCESS_STATE_API  mem_usage_t  memory_usage    ();
PROCESS_STATE_API  mem_size_t   phys_memory_used();

/// General system information
inline  mem_size_t  phys_memory   ();
inline  cpu_freq_t  cpu_frequency ();
inline  uint16_t    number_of_cpus();

struct PROCESS_STATE_API cpu_meter
    : boost::noncopyable
{
    cpu_meter(int inquiry_interval_ms = 1000);    // request interval for process times values (default 1 second)

    cpu_usage_t  cpu_usage(proc_id_t process_id = current_process_id());
    cpu_usage_t  cpu_load ();

    ~cpu_meter();
private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

/// Runs background thread which establishes TCP connection and interacts with server, sending self state to it.
/// It's up to clients to somehow provide distinctive ids.
struct state_thread;
PROCESS_STATE_API std::shared_ptr<state_thread> start_state_thread(network::endpoint const& connect_addr, id_type id = id_type());

typedef std::function<void (resp_state_msg const&)>  on_resp_f;

struct PROCESS_STATE_API state_server
    : boost::noncopyable
{
    state_server(network::endpoint const& bind_addr);
    ~state_server();

    void request_state(id_type peer, on_resp_f on_resp);

private:
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace process_state

namespace boost
{
namespace serialization
{

template <class Archive>
void serialize(Archive &ar, process_state::stack_entry& s_entry, const unsigned int)
{
    ar & s_entry.address;
    ar & s_entry.file;
    ar & s_entry.line;
    ar & s_entry.function;
    ar & s_entry.address;
}

} // namespace serialization
} // namespace boost

#include "process_state.hpp"
