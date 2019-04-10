/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#ifndef Q_MOC_RUN

#include "boost_config.h"

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <boost/type_traits.hpp>
#include <boost/format.hpp>

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4180) // see https://svn.boost.org/trac/boost/ticket/1097
#endif

#include <boost/bind.hpp>

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#include <boost/function.hpp>

#include <boost/next_prior.hpp>
#include <boost/noncopyable.hpp>
#include <boost/utility.hpp>

#include <boost/any.hpp>
#include <boost/variant.hpp>

#include <boost/ref.hpp>

#include <boost/circular_buffer.hpp>

#include <boost/assign.hpp>

#include <boost/ptr_container/ptr_container.hpp>

#include <boost/algorithm/string.hpp>
//#include <boost/tokenizer.hpp>

#include <boost/lexical_cast.hpp>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4100)
#endif 

#include <boost/signals2.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif 

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <boost/foreach.hpp>
#include <boost/none.hpp>

#include <boost/bimap.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <boost/math/special_functions.hpp>

//#include <boost/range/algorithm.hpp>
//#include <boost/range/adaptors.hpp>



using boost::noncopyable;

//using boost::scoped_ptr;
using boost::scoped_array;
using boost::intrusive_ptr;
using boost::shared_array;

using boost::optional;
using boost::in_place;

using boost::format;
using boost::wformat;

using boost::bind;

using boost::any;
using boost::any_cast;
using boost::bad_any_cast;

using boost::ref;
using boost::cref;

using boost::circular_buffer;

using boost::assign::list_of;
using boost::assign::map_list_of;

using boost::ptr_array;
using boost::ptr_deque;
using boost::ptr_list;
using boost::ptr_map;
using boost::ptr_set;
using boost::ptr_vector;
using boost::ptr_unordered_map;
using boost::ptr_unordered_set;
using boost::ptr_circular_buffer;

using boost::lexical_cast;

using std::dynamic_pointer_cast;
using std::static_pointer_cast;

using boost::signals2::scoped_connection;
using boost::signals2::connection;

using boost::unordered_map;
using boost::unordered_set;

using boost::none;

using boost::bimap;

#endif // Q_MOC_RUN