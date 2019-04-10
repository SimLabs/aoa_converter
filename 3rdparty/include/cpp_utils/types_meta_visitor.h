/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace cpp_utils
{
namespace types
{
    template<class t, class n = void>
    struct type_t
    {
        typedef t type;
        typedef n next;
    };

    template<class T>
    struct begin
    {
        typedef typename T type;
    };
    template<class T>
    struct end
    {
        typedef typename void type;
    };

    template<class T>
    struct next
    {
        typedef typename T::next type;
    };

    template<class A, class B>
    struct joint_view
    {
        typedef typename A::type type;
    };

    template<class B>
    struct joint_view < void, B >
    {
        typedef typename B::type type;
    };

    template<class A, class B>
    struct next < joint_view<A, B> >
    {
        typedef joint_view<typename next<A>::type, B> type;
    };

    template<class B_next>
    struct joint_view_next
    {
        typedef joint_view<void, B_next> type;
    };

    template<>
    struct joint_view_next < void >
    {
        typedef void type;
    };

    template<class B>
    struct next < joint_view< void, B> >
    {
        typedef typename joint_view_next<typename next<B>::type>::type type;
    };

    template<class it, class end, class visitor>
    struct visit_types_impl_t
    {
        bool operator()(visitor& v)
        {
            typedef typename it::type type;
            if (!v((type*)NULL))
                return true;
            else
                return visit_types_impl_t<next<it>::type, end, visitor>()(v);
        }
    };

    template<class end, class visitor>
    struct visit_types_impl_t < end, end, visitor >
    {
        bool operator()(visitor&)
        {
            return false;
        }
    };

    template<class types, class visitor>
    bool visit(visitor& v)
    {
        return visit_types_impl_t
            <
            begin<types>::type,
            end<types>::type,
            visitor
            > ()(v);
    }
}
}
