/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <boost/type_traits/is_convertible.hpp>

#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

#include "time_counter.h"

template<class predicate_t>
bool timed_predicate_wait(predicate_t const& pred, boost::posix_time::ptime const& abs_time)
{
    time_counter tc;
    auto start_time = boost::posix_time::microsec_clock::universal_time();

    while (start_time + tc.time() <= abs_time)
    {
        if (pred())
            return true;

        boost::interprocess::ipcdetail::thread_yield();
    }

    return false;
}


template<class scoped_lock_t, class condition_t>
bool timed_timed_wait(scoped_lock_t& l, condition_t& cond, boost::posix_time::ptime const& abs_time)
{
    using namespace boost::interprocess;

    BOOST_STATIC_ASSERT(
        (boost::is_convertible<typename scoped_lock_t::mutex_type&, named_mutex&>           ::value) ||
        (boost::is_convertible<typename scoped_lock_t::mutex_type&, named_upgradable_mutex&>::value));

    struct timed_lock
    {
        typedef boost::posix_time::ptime    ptime;
        typedef named_mutex                 mutex_type; // otherwise we've got an assert on named_upgradable_mutex usage. suppose, it's boost mistake

        timed_lock(scoped_lock_t& l, ptime const& abs_time)
            : lock_     (l)
            , abs_time_ (abs_time)
            , owns_     (l.owns())
        {
        }

        void lock()
        {
            Assert(!owns_);

            lock_.timed_lock(abs_time_);
            owns_ = lock_.owns();
        }

        void unlock()
        {
            Assert(owns_);

            lock_.unlock();
            owns_ = false;
        }

        bool owns() const
        {
            return owns_;
        }

        bool operator!() const
        {
            return !owns();
        }

        mutex_type* mutex()
        {
            return lock_.mutex();
        }

        void unspecified_bool(){}
        typedef void (timed_lock::*unspecified_bool_type)();

        operator unspecified_bool_type() const
        {
            return owns_ ? &timed_lock::unspecified_bool : unspecified_bool_type(0);
        }

    private:
        scoped_lock_t&  lock_;
        ptime           abs_time_;
        bool            owns_;
    };

    timed_lock lock(l, abs_time);
    return cond.timed_wait(lock, abs_time) && lock.owns();
}
