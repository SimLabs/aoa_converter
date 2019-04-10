/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace thread_ops
{

template<typename T>
class thread_aware_obj
    : boost::noncopyable
{
public:
    // Constructor isn't thread-aware
    thread_aware_obj() {}
    thread_aware_obj(const T  &object) : object_(object)                  {}
    thread_aware_obj(      T &&object) : object_(std::forward<T>(object)) {}

          T &get()       { check_thread(); return object_; }
    const T &get() const { check_thread(); return object_; }

    operator       T&  ()       { return  get(); }
    operator const T&  () const { return  get(); }
          T& operator* ()       { return  get(); }
    const T& operator* () const { return  get(); }
          T* operator->()       { return &get(); }
    const T* operator->() const { return &get(); }

    thread_aware_obj &operator=(const T  &object) { get() = object;                  return *this; }
    thread_aware_obj &operator=(      T &&object) { get() = std::forward<T>(object); return *this; }

private:
    void check_thread() const
    {
        const auto current_id = boost::this_thread::get_id();
        auto m = thread_id_.monitor();
        auto &id = m.get();
        if (id)
            Assert(*id == current_id);
        else
            id = current_id;
    }

private:
    mutable thread_safe_obj<boost::optional<boost::thread::id> > thread_id_;
    T object_;
};

}