/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "common/dyn_lib.h"
#include "network/msg_base.h"

#ifdef PROCESS_STATE_LIB
# define PROCESS_STATE_API __HELPER_DL_EXPORT
#else
# define PROCESS_STATE_API __HELPER_DL_IMPORT
#endif


namespace process_state
{

typedef int32_t   proc_id_t;
typedef int32_t   thread_id_t;
typedef uint64_t  address_t;
typedef uint64_t  module_size_t;

typedef uint32_t  id_type;

/// milliseconds
const int PROC_STATE_STACKTRACE_WAIT_TIMEOUT_MS = 500;


struct stack_entry
{
    stack_entry() : line(0) { }
    std::string  file;       /// filename
    uint32_t     line;       /// line number
    std::string  function;   /// name of function or method
    address_t    address;    /// address of the function
};
struct module_entry
{
    std::string     basename;
    std::string     fullname;
    uint64_t        mtime;      /// modification time
    address_t       address;    /// address in memory
    module_size_t   size;
};

REFL_STRUCT(stack_entry)
    REFL_ENTRY(file)
    REFL_ENTRY(line)
    REFL_ENTRY(function)
    REFL_ENTRY(address)
REFL_END()

REFL_STRUCT(module_entry)
    REFL_ENTRY(basename)
    REFL_ENTRY(fullname)
    REFL_ENTRY(mtime)
    REFL_ENTRY(address)
    REFL_ENTRY(size)
REFL_END()

typedef std::vector<stack_entry>            thread_stack;
typedef std::shared_ptr<thread_stack>     thread_stack_ptr;

typedef std::pair<thread_id_t, process_state::thread_stack>  thread_id_stack_pair;
typedef std::shared_ptr<thread_id_stack_pair>              thread_id_stack_pair_ptr;

typedef std::vector<thread_id_stack_pair>   proc_stack;
typedef std::shared_ptr<proc_stack>       proc_stack_ptr;

typedef std::vector<module_entry>           modules_vect;
typedef std::shared_ptr<modules_vect>     modules_vect_ptr;

typedef std::shared_ptr<std::string>      string_ptr;

typedef uint8_t   cpu_usage_t; // in %
typedef uint32_t  cpu_freq_t;  // in MHz
typedef uint8_t   mem_usage_t; // in %
typedef uint64_t  mem_size_t;  // in MB

enum ps_msg_types
{
    m_peer_id        = 0,
    m_request_state     ,
    m_response_state    ,
    m_count
};

struct peer_id_msg
    : network::msg_id<m_peer_id>
{
    peer_id_msg()
        : peer_id(id_type())
    { }

    peer_id_msg(id_type peerid)
        : peer_id(peerid)
    { }

    id_type  peer_id;
};

struct req_state_msg
    : network::msg_id<m_request_state>
{
};

struct resp_state_msg
    : network::msg_id<m_response_state>
{
    resp_state_msg()
        : mem_used   (0)
        , mem_total  (0)
        , mem_private(0)
    { }

    mem_size_t    mem_used;
    mem_size_t    mem_total;
    mem_size_t    mem_private;
    proc_stack    stack;
    modules_vect  modules;
};

typedef std::shared_ptr<resp_state_msg>  resp_state_msg_ptr;

REFL_STRUCT(peer_id_msg)
    REFL_ENTRY(peer_id)
REFL_END()

REFL_STRUCT(req_state_msg)
REFL_END()

REFL_STRUCT(resp_state_msg)
    REFL_ENTRY(mem_used)
    REFL_ENTRY(mem_total)
    REFL_ENTRY(mem_private)
    REFL_ENTRY(stack)
    REFL_ENTRY(modules)
REFL_END()

} // namespace process_state

