/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

//////////////////////////////////////////////////////////////////////////
// provides ability to call functions (function pointers and member function pointers), 
// passing more parameters than needed, but not less

namespace pack_call
{

// arguments pack structures
template<class t1>
struct arg_pack_1 { enum {arg_num = 1};     t1 arg1; };

template<class t1, class t2>
struct arg_pack_2 { enum {arg_num = 2};     t1 arg1; t2 arg2; };

template<class t1, class t2, class t3>
struct arg_pack_3 { enum {arg_num = 3};     t1 arg1; t2 arg2; t3 arg3; };

template<class t1, class t2, class t3, class t4>
struct arg_pack_4 { enum {arg_num = 4};     t1 arg1; t2 arg2; t3 arg3; t4 arg4; };

// template<class t1, class t2, class t3, class t4, class t5>
// struct arg_pack_5 { enum {arg_num = 5};     t1 arg1; t2 arg2; t3 arg3; t4 arg4; t5 arg5; };


// pack functions 
template<class t1>
arg_pack_1<t1>                  pack(t1 arg1)                                       { arg_pack_1<t1>                p = {arg1};                         return p; }

template<class t1, class t2>
arg_pack_2<t1, t2>              pack(t1 arg1, t2 arg2)                              { arg_pack_2<t1, t2>            p = {arg1, arg2};                   return p; }

template<class t1, class t2, class t3>
arg_pack_3<t1, t2, t3>          pack(t1 arg1, t2 arg2, t3 arg3)                     { arg_pack_3<t1, t2, t3>        p = {arg1, arg2, arg3};             return p; }

template<class t1, class t2, class t3, class t4>
arg_pack_4<t1, t2, t3, t4>      pack(t1 arg1, t2 arg2, t3 arg3, t4 arg4)            { arg_pack_4<t1, t2, t3, t4>    p = {arg1, arg2, arg3, arg4};       return p; }

// template<class t1, class t2, class t3, class t4, class t5>
// arg_pack_5<t1, t2, t3, t4, t5>  pack(t1 arg1, t2 arg2, t3 arg3, t4 arg4, t5 arg5)   { arg_pack_5<t1, t2, t3, t4, t5>p = {arg1, arg2, arg3, arg4, arg5}; return p; }


// aux
#define CHECK_ENOUGH_PARAMS(num)  static_assert(pack::arg_num >= (num), "not enough actual parameters passed")

// calls for function pointers
template<class r, class t1, class pack>
void call(r(*func)(t1), pack const& p)                  { CHECK_ENOUGH_PARAMS(1); func(p.arg1); }

template<class r, class t1, class t2, class pack>
void call(r(*func)(t1, t2), pack const& p)              { CHECK_ENOUGH_PARAMS(2); func(p.arg1, p.arg2); }

template<class r, class t1, class t2, class t3, class pack>
void call(r(*func)(t1, t2, t3), pack const& p)          { CHECK_ENOUGH_PARAMS(3); func(p.arg1, p.arg2, p.arg3); }

template<class r, class t1, class t2, class t3, class t4, class pack>
void call(r(*func)(t1, t2, t3, t4), pack const& p)      { CHECK_ENOUGH_PARAMS(4); func(p.arg1, p.arg2, p.arg3, p.arg4); }

// template<class r, class t1, class t2, class t3, class t4, class t5, class pack>
// void call(r(*func)(t1, t2, t3, t4, t5), pack const& p)  { CHECK_ENOUGH_PARAMS(5); func(p.arg1, p.arg2, p.arg3, p.arg4, p.arg5); }


// determining member function type
template<class pack, class functor>
void call(functor& func, pack const& p)  { call(&functor::operator(), func, p); }

// calls for member function pointers
template<class functor, class r, class t1, class pack>
void call(r(functor::*)(t1), functor& func, pack const& p)                   { CHECK_ENOUGH_PARAMS(1); func(p.arg1); }

template<class functor, class r, class t1, class t2, class pack>
void call(r(functor::*)(t1, t2), functor& func, pack const& p)               { CHECK_ENOUGH_PARAMS(2); func(p.arg1, p.arg2); }

template<class functor, class r, class t1, class t2, class t3, class pack>
void call(r(functor::*)(t1, t2, t3), functor& func, pack const& p)           { CHECK_ENOUGH_PARAMS(3); func(p.arg1, p.arg2, p.arg3); }

template<class functor, class r, class t1, class t2, class t3, class t4, class pack>
void call(r(functor::*)(t1, t2, t3, t4), functor& func, pack const& p)       { CHECK_ENOUGH_PARAMS(4); func(p.arg1, p.arg2, p.arg3, p.arg4); }

// template<class functor, class r, class t1, class t2, class t3, class t4, class t5, class pack>
// void call(r(functor::*)(t1, t2, t3, t4, t5), functor& func, pack const& p)   { CHECK_ENOUGH_PARAMS(5); func(p.arg1, p.arg2, p.arg3, p.arg4, p.arg5); }

} // pack_call

#define CALL_ARGS_1(fn, arg1)                           pack_call::call((fn), pack_call::pack((arg1)))
#define CALL_ARGS_2(fn, arg1, arg2)                     pack_call::call((fn), pack_call::pack((arg1), (arg2)))
#define CALL_ARGS_3(fn, arg1, arg2, arg3)               pack_call::call((fn), pack_call::pack((arg1), (arg2), (arg3)))
#define CALL_ARGS_4(fn, arg1, arg2, arg3, arg4)         pack_call::call((fn), pack_call::pack((arg1), (arg2), (arg3), (arg4)))
/*#define CALL_ARGS_5(fn, arg1, arg2, arg3, arg4, arg5)   pack_call::call((fn), pack_call::pack((arg1), (arg2), (arg3), (arg4), (arg5)))*/
