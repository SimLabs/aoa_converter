/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

struct connection_holder 
    : noncopyable
{
    typedef boost::signals2::connection         conn_t;
    typedef boost::signals2::scoped_connection  scoped_conn_t;

    typedef std::unique_ptr<scoped_conn_t> scoped_conn_ptr;
    typedef std::vector<scoped_conn_ptr>   scoped_conn_ptrs;

    connection_holder()
    {}

    connection_holder(connection_holder&& other) 
        : ptrs_(move(other.ptrs_))
    {}

    connection_holder& operator<<(conn_t const& conn)
    {
        ptrs_.emplace_back(scoped_conn_ptr(new scoped_conn_t(conn)));
        return *this;
    }

    void release()
    {
        ptrs_ = scoped_conn_ptrs();
    }

    bool empty() const 
    {
        return ptrs_.empty();
    }

private:
    scoped_conn_ptrs ptrs_;
};

template<class T>
struct connection_holder_map
{
    typedef boost::signals2::connection         conn_t;
    typedef boost::signals2::scoped_connection  scoped_conn_t;

    typedef std::unique_ptr<scoped_conn_t> scoped_conn_ptr;
    typedef std::map<T, scoped_conn_ptr>   scoped_conn_map;

    void insert(T const& s, conn_t const& conn)
    {
        std::swap(ptrs_[s], scoped_conn_ptr(new scoped_conn_t(conn)));    
    }

    void erase(T const& s)
    {
        ptrs_.erase(s);    
    }

    void release()
    {
        ptrs_ = scoped_conn_ptrs();
    }

private:
    scoped_conn_map ptrs_;
};


#define DECLARE_EVENT(name,arglist)                                                      \
    typedef boost::signals2::signal<void arglist>::slot_type name##_slot_type;           \
    boost::signals2::connection subscribe_##name( name##_slot_type const& slot ) const   \
    {                                                                                    \
        return name##_signal_.connect(slot);                                             \
    }                                                                                    \
                                                                                         \
    mutable boost::signals2::signal<void arglist> name##_signal_

#define DECLARE_VIRTUAL_EVENT(name,arglist)                                                    \
    typedef boost::signals2::signal<void arglist>::slot_type name##_slot_type;                 \
    virtual boost::signals2::connection subscribe_##name( name##_slot_type const& slot ) const \
    {                                                                                          \
        return name##_signal_.connect(slot);                                                   \
    }                                                                                          \
                                                                                               \
    mutable boost::signals2::signal<void arglist> name##_signal_

#define DECLARE_TEMPLATE_EVENT(name,arglist)                                             \
    typedef typename boost::signals2::signal<void arglist>::slot_type name##_slot_type;  \
    boost::signals2::connection subscribe_##name( name##_slot_type const& slot ) const   \
    {                                                                                    \
        return name##_signal_.connect(slot);                                             \
    }                                                                                    \
                                                                                         \
    mutable boost::signals2::signal<void arglist> name##_signal_

/*  DEPRECATED

    / * be careful, verify the module of to_track allocation still alive while signal is called. * /\
    / * Even if you unsubscribe!! (because of lazy unsubscription in boost::signals2) * / \
                                                                                        \
    template<class T>                                                                   \
    void subscribe_##name( name##_slot_type slot, std::shared_ptr<T> to_track)        \
    {                                                                                   \
        name##_signal_.connect(slot.track(to_track));                                   \
    }                                                                                   \
*/
    




