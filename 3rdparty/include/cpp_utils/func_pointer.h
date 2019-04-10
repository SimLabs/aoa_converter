/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace cpp_utils
{

template<typename Sygnature> class func_pointer;

template<class R>                                                                       struct func_pointer<R()>                            { typedef R(*type)(); };
template<class R, class T1>                                                             struct func_pointer<R(T1)>                          { typedef R(*type)(T1); };
template<class R, class T1, class T2>                                                   struct func_pointer<R(T1, T2)>                      { typedef R(*type)(T1, T2); };
template<class R, class T1, class T2, class T3>                                         struct func_pointer<R(T1, T2, T3)>                  { typedef R(*type)(T1, T2, T3); };
template<class R, class T1, class T2, class T3, class T4>                               struct func_pointer<R(T1, T2, T3, T4)>              { typedef R(*type)(T1, T2, T3, T4); };
template<class R, class T1, class T2, class T3, class T4, class T5>                     struct func_pointer<R(T1, T2, T3, T4, T5)>          { typedef R(*type)(T1, T2, T3, T4, T5); };
template<class R, class T1, class T2, class T3, class T4, class T5, class T6>           struct func_pointer<R(T1, T2, T3, T4, T5, T6)>      { typedef R(*type)(T1, T2, T3, T4, T5, T6); };
template<class R, class T1, class T2, class T3, class T4, class T5, class T6, class T7> struct func_pointer<R(T1, T2, T3, T4, T5, T6, T7)>  { typedef R(*type)(T1, T2, T3, T4, T5, T6, T7); };


} // cpp_utils
