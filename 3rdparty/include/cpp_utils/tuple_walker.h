/*
    * Copyright (C) 2015 SimLabs LLC - All rights reserved.
    * Unauthorized copying of this file or its part is strictly prohibited.
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once


namespace cpp_utils
{

namespace details
{

    template<typename Tuple, typename Visitor>
    struct tuple_walker_t
    {
        template<size_t index>
        static void walk(Tuple const &t, Visitor &visitor)
        {
            visitor(std::get<index>(t));
            walk<index + 1>(t, visitor);
        }

        template<size_t index>
        static void walk(Tuple const &t, Visitor const &visitor)
        {
            visitor(std::get<index>(t));
            walk<index + 1>(t, visitor);
        }

        template<>
        static void walk<std::tuple_size<Tuple>::value>(Tuple const &, Visitor const &)
        {
        }
    };

} //namespace details

template<typename Tuple, typename Visitor>
void walk_tuple(Tuple const &t, Visitor &visitor)
{
    details::tuple_walker_t<Tuple, Visitor>::walk<0>(t, visitor);
}

} // namespace cpp_utils