/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include <boost/thread.hpp>

#if defined(__GNUG__)
    #pragma GCC diagnostic push

    #pragma GCC diagnostic ignored "-fpermissive"
    #pragma GCC diagnostic ignored "-Wparentheses"
#endif

namespace thread_ops
{

template<class T>
struct data_monitor
    : boost::noncopyable
{
    typedef boost::recursive_mutex  mutex_t;

    explicit data_monitor(T& object, mutex_t& mut)
        : lock_  (mut   )
        , object_(object)
    {
    }
    
    data_monitor(data_monitor&& other)
        : lock_  (std::move(other.lock_))
        , object_(other.object_)
    {
    }

    data_monitor& operator=(data_monitor other)
    {
        swap(other);
        return *this;
    }
    
    T&      get  () const { return object_; } 
    operator T&  () const { return get()  ; }
    T& operator *() const { return get()  ; }
    T* operator->() const { return &object_;}
    
    // doesn't work for const monitor
    data_monitor& operator=(T const& obj){ object_ = obj;                   return *this; }
    data_monitor& operator=(T&& obj)     { object_ = std::forward<T>(obj);  return *this; }

    void swap(data_monitor& other)
    {
        using std::swap;
        swap(lock_  , other.lock_  );
        swap(object_, other.object_);
    }

private:
    mutex_t::scoped_lock lock_;
    T& object_;
};

template <class T>
struct thread_safe_obj
{
    typedef data_monitor<T const>           const_data_monitor;
    typedef data_monitor<T      >           data_monitor;
    typedef typename data_monitor::mutex_t  mutex_t;

    //thread_safe_obj() {}

    explicit thread_safe_obj(T const& object) : object_(object)                 {}
    explicit thread_safe_obj(T&&      object) : object_(std::forward<T>(object)){}
    template <class ... Args>
    explicit thread_safe_obj(Args && ... args)
        : object_(std::forward<Args&&>(args)...)
    {
    }

    const_data_monitor monitor() const { return const_data_monitor(object_, mut_); }
          data_monitor monitor()       { return       data_monitor(object_, mut_); }

private:
    mutable mutex_t mut_;
    T               object_;
};

// returns previous value 
template<class T>
T exchange(thread_safe_obj<T>& obj, T value)
{
    using namespace std;
    swap(obj.monitor(), value);
    
    return value;
}

} // thread_ops

#if defined(__GNUG__)
    #pragma GCC diagnostic pop
#endif
