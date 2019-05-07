/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/
#pragma once
#include <type_traits>
#include "logger/logger.hpp"
#include "cpp_utils/enum_to_string.h"

namespace refl
{
    enum processor_types 
    {
        pt_input,
        pt_output,
        pt_unknown
    };

    template<class processor>
    struct processor_type
    {
        static const size_t value = pt_unknown;
    };
}

//////////////////////////////////////////////////////////////////////////

template<typename inner_processor_type>
struct single_entry_processor
{
    single_entry_processor(inner_processor_type &inner_proc)
        : inner_proc_(inner_proc)
    {}

    template<typename T, typename... Args>
    void operator()(T &&entry, T &&entry2, char const *name, Args&&... args)
    {
        (void)entry2;
        Assert(&entry == &entry2);
        inner_proc_(std::forward<T>(entry), name, std::forward<Args>(args)...);
    }

private:
    inner_processor_type &inner_proc_;
};

namespace refl
{
    template <typename inner_processor_type>
    struct processor_type < single_entry_processor<inner_processor_type> >
    {
        static const size_t value = processor_type<inner_processor_type>::value;
    };
} // refl

namespace refl
{
    struct empty_tag_t{};
}




//////////////////////////////////////////////////////////////////////////


namespace refl
{

// general
template<class S, class T, class E = void>
struct uni_cast
{
    static T apply(S const& source)
    {
        return T(source);
    }
};

// boost::any
template<class T>
struct uni_cast<boost::any, T>
{
    static T apply(boost::any const& any_value)
    {
        if (auto ptr = boost::any_cast<T>(&any_value))
            return *ptr;
        
        return T();
    }
};

template<class T, class Source>
struct uni_cast<Source, T, typename std::enable_if<std::is_enum<T>::value>::type>
{
    static T apply(Source const& source)
    {
        auto res = cpp_utils::string_to_enum<T>(source);
        Verify(res);
        return *res;
    }
};

template<class S, class Target>
struct uni_cast<S, Target, typename std::enable_if<std::is_enum<S>::value>::type>
{
    static Target apply(S const& source)
    {
        return cpp_utils::enum_to_string(source);
    }
};

// boost::variant
template<class T, class ...Args >
struct uni_cast<boost::variant<Args...>, T>
{
    static T apply(boost::variant<Args...> const& variant_value)
    {
        if (auto ptr = boost::get<T>(&variant_value))
            return *ptr;

        return T();
    }
};


//////////////////////////////////////////////////////////////////////////

template<class S, class T>
void uni_cast_assign(T& target, S const& source)
{
    target = uni_cast<S, T>::apply(source);
}

template<class type_t, class proc_t, class entry_t, class tag_t>
void process_as_type(proc_t& proc, entry_t& entry, entry_t& entry2, const char* name, tag_t tag)
{
    (void)entry2;
    Assert(&entry == &entry2);
    type_t type_value;
    uni_cast_assign(type_value, entry);
    proc(type_value, type_value, name, tag);
    ::refl::uni_cast_assign(entry, type_value);
}

template<class type_t, class proc_t, class entry_t>
void process_as_type(proc_t& proc, entry_t& entry, entry_t& entry2, const char* name)
{
    (void)entry2;
    type_t type_value;
    uni_cast_assign(type_value, entry);
    if(&entry == &entry2)
    {
        proc(type_value, type_value, name);
        ::refl::uni_cast_assign(entry, type_value);
    } 
    else
    {
        type_t type_value2;
        uni_cast_assign(type_value2, entry2);
        proc(type_value, type_value2, name);
        ::refl::uni_cast_assign(entry, type_value);
        ::refl::uni_cast_assign(entry2, type_value2);
    }
}

//////////////////////////////////////////////////////////////////////////
// enum selector

/*
template<class proc_t, class entry_t>
void process(proc_t& proc, entry_t& entry, const char* name, 
             typename std::enable_if<!std::is_enum<entry_t>::value>::type* = 0)
{
    proc(entry, name);
}

template<class proc_t, class entry_t>
void process(proc_t& proc, entry_t& entry, const char* name, 
             typename std::enable_if<std::is_enum<entry_t>::value>::type* = 0)
{
    uint32_t value(entry);
    proc(value, name);
    entry = entry_t(value);
}

template<class proc_t, class entry_t, class tag_t>
void process(proc_t& proc, entry_t& entry, const char* name, tag_t tag,
             typename std::enable_if<!std::is_enum<entry_t>::value>::type* = 0)
{
    proc(entry, name, tag);
}

template<class proc_t, class entry_t, class tag_t>
void process(proc_t& proc, entry_t& entry, const char* name, tag_t tag,
             typename std::enable_if<std::is_enum<entry_t>::value>::type* = 0)
{
    uint32_t value(entry);
    proc(value, name, tag);
    entry = entry_t(value);
}

*/

//////////////////////////////////////////////////////////////////////////
// special feature for binary serialization 

template<class T>
struct bin_adaptor_impl
    : boost::noncopyable
{
    bin_adaptor_impl(T const& cref)
        : ref_(const_cast<T&>(cref))
    {
    }

    bin_adaptor_impl(bin_adaptor_impl && rhs) noexcept
        : ref_(rhs.ref)
    {}

    T& ref() const 
    {
        return ref_;
    }


private:
    T&  ref_;
};

//////////////////////////////////////////////////////////////////////////
// bin_adaptor 

template<class T>
bin_adaptor_impl<T> bin_adaptor(T const& t)
{
    return bin_adaptor_impl<T>(t);
}

} // refl

//////////////////////////////////////////////////////////////////////////
// common structures
// STL

// pair
namespace std
{

template<class t1, class t2, class processor>
void reflect2(processor& proc, std::pair<t1, t2> const& lhs, std::pair<t1, t2> const& rhs)
{
    typedef std::pair<t1, t2> type;

    type& lobj = const_cast<type&>(lhs);
    type& robj = const_cast<type&>(rhs);

    // workaround for std::map<...>::value_type
    proc(const_cast<typename std::remove_const<t1>::type&>(lobj.first ), const_cast<typename std::remove_const<t1>::type&>(robj.first ), "first" );
    proc(const_cast<typename std::remove_const<t2>::type&>(lobj.second), const_cast<typename std::remove_const<t2>::type&>(robj.second), "second");
}

} // std
