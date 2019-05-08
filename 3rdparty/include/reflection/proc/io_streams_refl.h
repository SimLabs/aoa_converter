/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

#include <type_traits>

#include "reflection/reflection.h"
#include "reflection/reflection_ext.h"
#include "serialization/io_streams_ops.h"

namespace binary
{
    namespace type_traits
    {
        template<class T>
        using use_reflection = 
            typename std::enable_if
            <
                !is_primitive<T>::value && 
                !is_container<T>::value && 
                ::refl::enable_reflect_traits<T>::value
            >::type;

    } // type_traits

    //////////////////////////////////////////////////////////////////////////
    // reflection processors
    namespace details
    {
        struct write_processor
        {
            explicit write_processor(output_stream& os)
                : os(os)
            {
            }

            template<class type>
            void operator()(type const& object, char const *, ...) // just forwarding to outer functions for reflection
            {
                using namespace binary;
                write(os, object);
            }

            output_stream& os;
        };


        struct read_processor
        {
            explicit read_processor(input_stream& is)
                : is(is)
            {
            }

            template<class type>
            void operator()(type& object, char const *, ...) // just forwarding to outer functions for reflection
            {
                using namespace binary;
                read(is, object);
            }

            input_stream& is;
        };
    } // details 

    template<class type>
    void write(output_stream& os, type const& object, type_traits::use_reflection<type>* = 0)
    {
        details::write_processor wp(os);
        reflect(wp, object);
    }

    template<class type>
    void read(input_stream& is, type& object, type_traits::use_reflection<type>* = 0)
    {
        details::read_processor rp(is);
        reflect(rp, object);
    }
} // binary

namespace refl
{
    template<> struct processor_type < binary::details::read_processor  > { static const size_t value = pt_input; };
    template<> struct processor_type < binary::details::write_processor > { static const size_t value = pt_output; };
} // refl

                   
