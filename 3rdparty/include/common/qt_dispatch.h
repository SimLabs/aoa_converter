/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <QtCore>

namespace qt_dispatch
{

const size_t event_id = QEvent::User + 1;
typedef std::function<void()> fwd_f;

struct forwarder_t;
typedef std::shared_ptr<forwarder_t> forwarder_ptr;

struct event_t
    : QEvent
{
    event_t(fwd_f const& func, forwarder_ptr ptr = forwarder_ptr());

    fwd_f         func;
    forwarder_ptr forwarder;
};

struct forwarder_t
    : QObject
{
    forwarder_t(QThread* thread = 0);

    Q_OBJECT
    inline virtual bool event(QEvent* e);
};

// posting to MAIN thread
void post(fwd_f const&);

// posting to ANY thread
struct thread_queue
{
    thread_queue();
    void post(fwd_f const&);

private:
    forwarder_ptr ptr_;
};


/// ///////////////////////////////////
/// impl

inline event_t::event_t(fwd_f const& func, forwarder_ptr ptr)
    : QEvent   (QEvent::Type(event_id))
    , func     (func)
    , forwarder(ptr ? ptr : forwarder_ptr(new forwarder_t(nullptr)))
{
    forwarder->moveToThread(QCoreApplication::instance()->thread());
}

inline bool forwarder_t::event(QEvent* e)
{
    if (e->type() == event_id)
    {
        event_t& event = static_cast<event_t&>(*e);
        event.func();

        return true;
    }

    return false;
}

inline forwarder_t::forwarder_t(QThread* thread)
{
    if (thread)
        moveToThread(thread);
}

//
inline void post(fwd_f const& func)
{
    event_t* event = new event_t(func);
    QCoreApplication::postEvent(event->forwarder.get(), event);
}

//
inline thread_queue::thread_queue()
    : ptr_(std::make_shared<forwarder_t>(QThread::currentThread()))
{
}

inline void thread_queue::post(fwd_f const& func)
{
    event_t* event = new event_t(func, ptr_);
    QCoreApplication::postEvent(event->forwarder.get(), event);
}

} // qt_dispatch
