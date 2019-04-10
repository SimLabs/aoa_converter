/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "serialization/dict_ops.h"
                
namespace dict
{
    namespace type_traits
    {
        template<class T>
        using use_reflection = 
            typename std::enable_if
            <
                !is_leaf_type<T>::value && 
                !is_container<T>::value && 
                refl::enable_reflect_traits<T>::value
            >::type;
    } // type_traits

    namespace details
    {
        // processors 
        struct write_processor
        {
            explicit write_processor(dict_t& dict)
                : bd_(dict)
            {
            }

            template<class type>
            void operator()(type const& object, char const* key, ...) // just forwarding to outer functions for reflection
            {
                using namespace dict;
                write(bd_, object, key);
            }

        private:
            dict_t& bd_;
        };

        //////////////////////////////////////////////////////////////////////////

        struct read_processor
        {
            explicit read_processor(dict_t const& dict, bool log_on_absence = true)
                : bd_(dict)
                , log_on_absence_(log_on_absence)
            {
            }

            template<class type>
            void operator()(type& object, char const* key, ...) // just forwarding to outer functions for reflection
            {
                using namespace dict;
                read(bd_, object, key, log_on_absence_);
            }

        private:
            dict_t const&   bd_;
            bool            log_on_absence_;
        };

    } // details 


    //////////////////////////////////////////////////////////////////////////
    // general functions 
    //- structs
    template<class type>
    void write(dict_t& dict, type const& object, type_traits::use_reflection<type>* = 0)
    {
        details::write_processor wp(dict);
        reflect(wp, object);
    }

    template<class type>
    void read(dict_t const& dict, type& object, bool log_on_absence = true, type_traits::use_reflection<type>* = 0)
    {
        details::read_processor rp(dict, log_on_absence);
        reflect(rp, object);
    }

} // dict

namespace refl
{
    template<> struct processor_type < dict::details::read_processor  > { static const size_t value = pt_input; };
    template<> struct processor_type < dict::details::write_processor > { static const size_t value = pt_output; };
} // refl
