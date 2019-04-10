/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace cpp_utils
{

template<typename comparator_type>
struct reflect_compare_processor
{
    typedef typename std::remove_const<comparator_type>::type non_const_comparator_t;

    reflect_compare_processor(comparator_type &comparator)
        : comparator_(comparator)
        , is_equal_(true)
    {}

    template<typename T>
    void operator()(T const &lhs, T const &rhs, char const * name, ...)
    {
        if (!is_equal_)
            return;

        is_equal_ = comparator_(lhs, rhs);

        if (!is_equal_)
        {
            //sad to use const cast
            const_cast<non_const_comparator_t&>(comparator_).append_path(string(name, name + strlen(name)));
        }
    }
    
    bool is_equal() const
    {
        return is_equal_;
    }

private:
    comparator_type &comparator_;
    bool is_equal_;
};

template<typename comparator_type, typename T>
inline bool apply_comp_refl(comparator_type &c, T const &lhs, T const &rhs)
{
    cpp_utils::reflect_compare_processor<comparator_type> proc(c);          
    reflect2(proc, lhs, rhs);                                               
    return proc.is_equal();                                                 
}

} // namespace cpp_utils

#define ENABLE_REFL_COMP(type) \
    template<typename comparator_type>                                           \
    friend bool apply_comp(comparator_type &c, type const &lhs, type const &rhs) \
    {                                                                            \
        cpp_utils::reflect_compare_processor<comparator_type> proc(c);           \
         reflect2(proc, lhs, rhs);                                               \
         return proc.is_equal();                                                 \
    }


