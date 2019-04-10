/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <cstddef>
#include "stddefs.h"
#include <assert.h>

#define BOOST_ALL_DYN_LINK


#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

#ifdef BOOST_SP_DISABLE_THREADS
#error "don't use BOOST_SP_DISABLE_THREADS"
#endif

// delete everywhere #pragma warning(disable: 4996) -- 'function': was declared deprecated

namespace boost
{
   inline void assertion_failed(char const * expr, char const * function, char const * file, long line)
   {
       (void)expr;
       (void)function;
       (void)file;
       (void)line;

#ifndef NDEBUG
//      std::cout << "Boost assertion failed: " << expr << " at \""
//                << function << "\" in " << file << ":" << line << std::endl;
      assert(0 && expr);
#endif
   }

   inline void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line)
   {
       (void)expr;
       (void)msg;
       (void)function;
       (void)file;
       (void)line;

#ifndef NDEBUG
//       std::cout << "Boost assertion failed: " << expr << ". Msg: " << msg << ". At \""
//                 << function << "\" in " << file << ":" << line << std::endl;
       assert(0 && expr);
#endif
   }
}

#define BOOST_ENABLE_ASSERT_HANDLER
#define BOOST_FILESYSTEM_NO_DEPRECATED

/*
#if defined(_MSC_VER)
    #if defined(_M_IX86)
        #define BOOST_BIND_ENABLE_STDCALL
    #endif
    #define BOOST_MEM_FN_ENABLE_STDCALL
#endif
*/

#define BOOST_ASIO_HAS_MOVE

// intrusive_ptr adapters
namespace intrusive_ptr_details
{
   struct intrusive_counter
   {
      intrusive_counter() : count( 0 ) {}
      intrusive_counter( intrusive_counter const & ) : count( 0 ) {}
      intrusive_counter & operator=( intrusive_counter const & ) { return *this; }
      size_t count;
   };
}
#define DECLARE_INTRUSIVE_COUNTER_FWD(name)                                            \
   __forceinline void intrusive_ptr_add_ref( name const* p );                          \
   __forceinline void intrusive_ptr_release( name const* p );

#define DECLARE_INTRUSIVE_COUNTER(name)                                                                              \
   mutable intrusive_ptr_details::intrusive_counter _ptr_count_;                                                     \
   __forceinline friend void intrusive_ptr_add_ref( name const* p ) { ++p->_ptr_count_.count; }                      \
   __forceinline friend void intrusive_ptr_release( name const* p ) { if( --p->_ptr_count_.count == 0 ) delete p; }


#define DECLARE_THREAD_SAFE_INTRUSIVE_COUNTER(name)                                                                                                                 \
   mutable volatile intrusive_ptr_details::intrusive_counter _ptr_count_;                                                                                           \
   __forceinline friend void intrusive_ptr_add_ref( name const* p ) { InterlockedIncrement( reinterpret_cast<volatile long*>(&p->_ptr_count_.count) ); }                     \
   __forceinline friend void intrusive_ptr_release( name const* p ) { if( InterlockedDecrement( reinterpret_cast<volatile long*>(&p->_ptr_count_.count) ) == 0 ) delete p; }
