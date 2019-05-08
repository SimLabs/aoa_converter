/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "alloc.h"

#include <limits>


//
// STL-compatible private-heap based single-element allocator
//

#ifndef DISABLE_CUSTOM_ALLOCATOR
template<typename T>
struct private_heap_allocator
    //: type_size_base<T>
{
public: // typedefs
    typedef T                   value_type;
    typedef value_type *        pointer;
    typedef const value_type *  const_pointer;
    typedef value_type &        reference;
    typedef const value_type &  const_reference;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;

public: // convert an allocator<T> to allocator<U>
    template<typename U> struct rebind { typedef private_heap_allocator<U> other; };

public: // ctors
    __forceinline private_heap_allocator() {}
    __forceinline private_heap_allocator( const private_heap_allocator & /*other*/ ) {}

    template<typename U> 
    __forceinline private_heap_allocator( const private_heap_allocator<U> & ) {}

    __forceinline private_heap_allocator(private_heap_allocator&& /*other*/) {}

    template<typename U>
    __forceinline private_heap_allocator(private_heap_allocator<U>&&) {}


    __forceinline private_heap_allocator &operator=(private_heap_allocator&& /*other*/) {return *this;}


public: // address
    __forceinline pointer address( reference r ) const { return &r; }
    __forceinline const_pointer address( const_reference r ) { return &r; }

public: // memory allocation
    __forceinline pointer allocate( size_type count, typename std::allocator<void>::const_pointer hint = nullptr )
    {
        (void)hint;
        Assert(count == 1);
        return (pointer)heap()->allocate();
    }
    __forceinline void deallocate( pointer p, size_type count )
    {
        Assert(count == 1);
        heap()->deallocate(p);
    }

public: // max_size
    __forceinline size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

public: // objects construction/destruction
    template<class U, class Args>
    __forceinline void construct( U * p, Args && args ) { new(p) U(std::forward<Args>(args)); }

    template<class U>
    __forceinline void destroy( U * p ) { (void)p; p->~U(); }

private:
    static pool_alloc_t* heap()
    {
        static pool_alloc_ptr heap_obj(get_pool_allocator(sizeof(T)));
        return heap_obj.get();
    }
};
#else
template<class T>
struct private_heap_allocator : std::allocator<T> {};
#endif

template<class T1, class T2>
bool operator==(const private_heap_allocator<T1> &, const private_heap_allocator<T2> &)
{
    return true;
}
template<class T1, class T2>
bool operator!=(const private_heap_allocator<T1> &, const private_heap_allocator<T2> &)
{
    return false;
}

// helpers for set
template <typename T, typename Pred = std::less<T>>
    struct ph_set
{
    typedef std::set     <T, Pred, private_heap_allocator<T>>   set_t;
    typedef std::multiset<T, Pred, private_heap_allocator<T>>   multiset_t;
};

// helpers for map
template <typename K, typename T, typename Pred = std::less<K>>
    struct ph_map
{
    typedef std::map     <K, T, Pred, private_heap_allocator<std::pair<const K, T>>>   map_t;
    typedef std::multimap<K, T, Pred, private_heap_allocator<std::pair<const K, T>>>   multimap_t;
};
// helpers for list
template <typename T>
    struct ph_list
{
    typedef std::list<T, private_heap_allocator<T>> list_t;
};
  
template <typename T, typename Pred = std::less<T>>
using ph_set_t = typename ph_set<T, Pred>::set_t;

template <typename T, typename Pred = std::less<T>>
using ph_multiset_t = typename ph_set<T, Pred>::multiset_t;

template <typename T, typename Pred = std::less<T>>
using ph_map_t = typename ph_map<T, Pred>::map_t;

template <typename T, typename Pred = std::less<T>>
using ph_multimap_t = typename ph_map<T, Pred>::multimap_t;

template <typename T>
using ph_list_t = typename ph_list<T>::list_t;



