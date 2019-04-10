#pragma once 
#include <type_traits>

#define BIND_PTR_MEM(FUNC, PTR_VALUE, ...) \
boost::bind(&std::remove_reference<decltype(*(PTR_VALUE))>::type::FUNC, (PTR_VALUE), __VA_ARGS__)
                                                                                                  
#define BIND_MEM(FUNC, ...) \
boost::bind(&std::remove_reference<decltype(*this)>::type::FUNC, this, __VA_ARGS__)
