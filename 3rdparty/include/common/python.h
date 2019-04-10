/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#ifdef _DEBUG
#   define Py_DEBUG
#   define BOOST_DEBUG_PYTHON
#   define BOOST_LINKING_PYTHON
#endif

#pragma warning(push)
#pragma warning(disable:4100) // unused formal parameter
#pragma warning(disable:4244) // conversion from 'Py_ssize_t' to 'unsigned int', possible loss of data
#pragma warning(disable:4273) // 'round' : inconsistent dll linkage
#   include "boost/python.hpp"
#pragma warning(pop)

namespace py = boost::python;

#ifdef _DEBUG
#   undef Py_DEBUG
#   undef BOOST_DEBUG_PYTHON
#   undef BOOST_LINKING_PYTHON
#endif
