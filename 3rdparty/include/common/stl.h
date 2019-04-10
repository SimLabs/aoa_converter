/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "stl_config.h"

#ifdef _WIN32
// define VLD_CHECK to turn leak checking on!
//
// see http://vld.codeplex.com/
#   if !defined(NDEBUG) && defined(VLD_CHECK)
#       include <vld.h>
#   endif

#endif

#include <assert.h>

#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include <array>

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <numeric>

#include <functional>
#include <iterator>
#include <limits>
#include <locale>
#include <memory>
#include <stdexcept>
#include <utility>
#ifndef _WIN32
#   include <bits/move.h>
#endif

using std::deque;
using std::list;
using std::map;
using std::multimap;
using std::queue;
using std::set;
using std::multiset;
using std::stack;
using std::string;
using std::wstring;
using std::vector;
using std::array;
using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;
using std::make_unique;
using std::enable_shared_from_this;

using std::ofstream;
using std::ifstream;

using std::exception;
using std::runtime_error;

using std::pair;
using std::make_pair;

using std::move;
using std::forward;

typedef string const& str_cref;

namespace std
{

template<class T>
bool operator < (weak_ptr<T> const& a, weak_ptr<T> const& b)
{
    return a.owner_before(b);
}

}
