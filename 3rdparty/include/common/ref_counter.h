/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <boost/intrusive_ptr.hpp>

struct ref_counter
{
private:
    friend void intrusive_ptr_add_ref(ref_counter * p)
    {
        ++(p->counter_);
    }

    friend void intrusive_ptr_release(ref_counter * p)
    {
        if (--(p->counter_) == 0)
            p->dispose();
    }

private:
    virtual void dispose()
    {
        delete this;
    }

protected:
    // restrict following methods
    // XXX: VL and EZ decided to move from private to protected
    ref_counter( const ref_counter & ) : counter_(0) {}
    ref_counter & operator= ( const ref_counter & ) {}

protected:
    ref_counter() : counter_(0) {}
    virtual ~ref_counter() {}

private:
    unsigned counter_;
};

//namespace boost
//{
//     inline void intrusive_ptr_add_ref(ref_counter * p)
//     {
//         // increment reference count of object *p
//         ++(p->counter_);
//     }

//     inline void intrusive_ptr_release(ref_counter * p)
//     {
//         // decrement reference count, and delete object when reference count reaches 0
//         if (--(p->counter_) == 0)
//             delete p;
//     }
//}

// observed ref_ptr for intrusive singletons
struct intrusive_ptr_observed : public virtual ref_counter
{
protected:
    inline intrusive_ptr_observed() : observed_memory_place_(0) {}
    virtual ~intrusive_ptr_observed() { if (observed_memory_place_) *observed_memory_place_ = 0; }

public:
    inline void set_observer_ptr( void * p ) { observed_memory_place_ = (void **)p; }

private:
    void ** observed_memory_place_;
};

// and singleton helper
template <typename T> struct singleton
{
public:
    static boost::intrusive_ptr<T> instance()
    {
        static T * ptr = NULL;
        if (!ptr)
        {
            ptr = new T();
            ptr->set_observer_ptr(&ptr);
        }
        return ptr;
    }
};
