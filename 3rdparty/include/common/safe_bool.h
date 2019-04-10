/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once


// "operator bool()" is dangerous
// e.g.:
//
//    A a1, a2;   // "A" has operator bool
//    int i = a1;
//    a1 << 1;
//    if (a1 == a2) {}
//    if (a1  < a2) {}
//
// all of the above would compile just fine :-(
//
// if you want to be a good man, you should instead of
//
//    struct A
//    {
//       operator bool () const {return ...}
//    };
//
// write
//
//    struct A
//    {
//       SAFE_BOOL_OPERATOR(...)
//    };
//


#define SAFE_BOOL_OPERATOR(true_cond)                                         \
   struct _safe_bool_helper                                                   \
   {                                                                          \
      void safe_bool_function() const {}                                      \
   };                                                                         \
                                                                              \
   typedef void (_safe_bool_helper::*safe_bool_type) () const;                \
                                                                              \
   operator safe_bool_type() const                                            \
   {                                                                          \
   return (true_cond)                                                         \
         ? &_safe_bool_helper::safe_bool_function                             \
         : safe_bool_type(NULL);                                              \
   }


#define SAFE_BOOL_OPERATOR_WITH_EQ(type, true_cond)                           \
   SAFE_BOOL_OPERATOR(true_cond)                                              \
private:                                                                      \
   bool operator == (type const&);                                            \
   bool operator != (type const&);                                            \
public:
