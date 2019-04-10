/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <boost/asio/io_service.hpp>
#include "common/qt_dispatch.h"
#include "common/dyn_lib.h"

#ifdef CONCURRENT_LIB
#   define CONCURRENT_API __HELPER_DL_EXPORT
#else
#   define CONCURRENT_API __HELPER_DL_IMPORT
#endif

namespace concurrent
{

namespace details
{

using namespace boost::asio;
using boost::thread;

CONCURRENT_API io_service& get_io_service();

template <class result_type>
void callback_wrap(std::function<void (result_type)> const& callback_f, result_type res, optional<std::weak_ptr<void>> wp)
{
    if (wp && !wp->expired())
        callback_f(res);
}

template <class result_type>
void process_task(
    std::function<result_type ()>     const& task_f,
    std::function<void (result_type)> const& callback_f,
    optional<std::weak_ptr<void>>            wp)
{
    result_type res = task_f();
    qt_dispatch::post(boost::bind(&callback_wrap<result_type>, callback_f, res, wp));
}

} // namespace details


struct CONCURRENT_API calculator
{
    calculator ();
    ~calculator();

private:
    struct impl;
    impl*  pimpl_;
};

template <class result_type>
void post_task(
    std::function<result_type ()>     const& task_f,
    std::function<void (result_type)> const& callback_f,
    std::weak_ptr<void>                      wp = std::weak_ptr<void>())
{
    callback_f(task_f());
    /*details::get_io_service().post(boost::bind(&details::process_task<result_type>, task_f, callback_f,
        (wp.expired() ? boost::none : optional<weak_ptr<void>>(wp))));*/
}

} // namespace concurrent
