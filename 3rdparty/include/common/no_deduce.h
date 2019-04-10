/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once


namespace meta
{
   template < class T >
      struct no_deduce_f
   {
      typedef     T     type;
   };
}

#ifndef NO_DEDUCE
#  define   NO_DEDUCE(t)   typename meta::no_deduce_f< t >::type
#endif

#ifndef NO_DEDUCE2
#  define   NO_DEDUCE2(p1,p2)   typename meta::no_deduce_f< p1,p2 >::type
#endif
