/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include "common/dyn_lib.h"

#ifdef ALLOC_LIB
# define ALLOC_API __HELPER_DL_EXPORT
#else
# define ALLOC_API __HELPER_DL_IMPORT
#endif


// single-element pool (set, heap, map, list, ...)
struct pool_alloc_t
{
    virtual ~pool_alloc_t() {}

    virtual void * allocate() = 0;
    virtual void deallocate( void * ) = 0;
};
typedef std::shared_ptr<pool_alloc_t> pool_alloc_ptr;
ALLOC_API pool_alloc_ptr get_pool_allocator( size_t item_size );