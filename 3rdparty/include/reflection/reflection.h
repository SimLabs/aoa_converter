/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "reflection_aux.h"


template<typename processor, typename T>
void reflect(processor &proc, T const &object)
{
    single_entry_processor<processor> proc_wrapper(proc);
    reflect2(proc_wrapper, object, object);
}

#define REFL_STRUCT_BODY(...)                           \
void reflect2(processor& proc, __VA_ARGS__ const& lhs, __VA_ARGS__ const& rhs)\
{                                                       \
    typedef __VA_ARGS__ type;                           \
    type& lobj = const_cast<type&>(lhs);/*small hack*/  \
    type& robj = const_cast<type&>(rhs);/*small hack*/  \
                                                        \
    (void) proc; /*avoiding warning*/                   \
    (void) lobj; /*avoiding warning*/                   \
    (void) robj; /*avoiding warning*/                   


// for using outside structure or class
#define REFL_STRUCT(type)                               \
template<class processor>                               \
    REFL_STRUCT_BODY(type)

// for using inside structure or class - mostly for templates
#define REFL_INNER(type)                        \
template<class processor>                       \
    friend REFL_STRUCT_BODY(type)

#define REFL_ENTRY_NAMED(entry, name)           \
    proc(lobj.entry, robj.entry, name);

#define REFL_ENTRY_NAMED_WITH_TAG(entry, name, tag) \
    proc(lobj.entry, robj.entry, name, tag);

#define REFL_ENTRY(entry)                       \
    proc(lobj.entry, robj.entry, #entry);    

#define REFL_CHAIN(base)                        \
    reflect2(proc, (base&)lobj, (base&)robj);

#define REFL_AS_TYPE_NAMED(entry, type, name)   \
    ::refl::process_as_type<type>                 \
        (proc, lobj.entry, robj.entry, name);

#define REFL_AS_TYPE(entry, type)               \
    ::refl::process_as_type<type>                 \
    (proc, lobj.entry, robj.entry, #entry);

#define REFL_AS_TYPE_WITH_TAG(entry, type, tag)               \
    ::refl::process_as_type<type>                 \
    (proc, lobj.entry, robj.entry, #entry, tag);

#define REFL_ENTRY_TAG(entry, tag)              \
    proc(lobj.entry, robj.entry, #entry, tag);

#define REFL_END()                              \
}

#define ENUM_DECL(name) \
    inline auto const &enum_string_match_detail(name const*)       \
    {                                                              \
        typedef name enum_type;                                    \
        const static std::vector<std::pair<enum_type, std::string>> m = {

#define ENUM_DECL_INNER(name) friend ENUM_DECL(name)

#define ENUM_DECL_ENTRY(e) \
    { e, #e },

#define ENUM_DECL_ENTRY_S(e) \
    { enum_type::e, #e },

#define ENUM_DECL_END()     \
        };                  \
        return m;           \
    }

template<size_t SIZE>
struct reflect_helper
{

    template<std::size_t I = 0, class processor, class tuple_t>
    static typename std::enable_if<I == SIZE, void>::type reflect2_tuple(processor& /* proc */ , tuple_t& /* lhs */, tuple_t& /* rhs */)
    {
    }

    template<std::size_t I = 0, class processor, class tuple_t>
    static typename std::enable_if<I < SIZE, void>::type reflect2_tuple(processor& proc, tuple_t& lhs, tuple_t& rhs)
    {
        proc(std::get<I>(lhs), std::get<I>(rhs), boost::lexical_cast<std::string>(I).c_str());
        reflect2_tuple<I + 1>(proc, lhs, rhs);
    }
};

template<class processor, class ...Args>
void reflect2(processor& proc,  std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs)
{
    typedef std::tuple<Args...> type;
    type& lobj = const_cast<type&>(lhs);/*small hack*/ 
    type& robj = const_cast<type&>(rhs);/*small hack*/ 

    reflect_helper<std::tuple_size<type>::value>::reflect2_tuple(proc, lobj, robj);
}

template<class processor, class Type, size_t Size>
void reflect2(processor& proc, std::array<Type, Size> const& lhs, std::array<Type, Size> const& rhs)
{
    using type = std::array<Type, Size>;
    type& lobj = const_cast<type&>(lhs);/*small hack*/
    type& robj = const_cast<type&>(rhs);/*small hack*/

    for(size_t i = 0; i < Size; ++i)
        proc(lobj[i], robj[i], boost::lexical_cast<std::string>(i).c_str());
}

namespace refl
{
    template<class T>
    struct enable_reflect_traits
    {
        static const bool value = true;
    };
} // refl


//////////////////////////////////////////////////////////////////////////
// simple sample: 

// struct group
// {
//     string name;
// };
// 
// REFL_STRUCT(group)
//     REFL_ENTRY(name)
// REFL_END   ()
// 
// struct user 
// {
//     string          name;
//     vector<group>   groups;
// 
//     REFL_INNER(person)
//         REFL_ENTRY(name)
//         REFL_ENTRY(groups)
//     REFL_END  ()
// };
// 
