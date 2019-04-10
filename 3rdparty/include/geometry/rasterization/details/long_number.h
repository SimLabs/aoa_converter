/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#if defined _WIN32 || defined __CYGWIN__
#  pragma warning (push)
#  pragma warning (disable:4146)
#endif

// #include <gmp/gmp.h>

#if defined _WIN32 || defined __CYGWIN__
#  pragma warning (pop)
#endif

namespace geom {
namespace rasterization {
namespace details {

    // ToDo: be careful, possible loss of precision due to using double instead of GMP library (as it was previously)
    typedef double long_number;

//    struct long_number
//    {
//       explicit long_number(double x)
//       {
//          mpf_init_set_d(val_, x);
//       }
// 
//       explicit long_number(int x)
//       {
//          mpf_init_set_si(val_, x);
//       }
// 
//       long_number(long_number const & other)
//       {
//          mpf_init_set(val_, other.val_);
//       }
// 
//       long_number& operator = (long_number const & other)
//       {
//          mpf_set(val_, other.val_);
//          return *this;
//       }
// 
//       ~long_number()
//       {
//          mpf_clear(val_);
//       }
// 
//       long_number & operator += (long_number const & other)
//       {
//          mpf_add(val_, val_, other.val_);
//          return *this;
//       }
// 
//       long_number & operator -= (long_number const & other)
//       {
//          mpf_sub(val_, val_, other.val_);
//          return *this;
//       }
// 
//       long_number & operator *= (long_number const & other)
//       {
//          mpf_mul(val_, val_, other.val_);
//          return *this;
//       }
// 
//       long_number & operator *= (int x)
//       {
//          if (x < 0)
//          {
//             mpf_mul_ui(val_, val_, -x);
//             mpf_neg(val_, val_);
//          }
//          else
//             mpf_mul_ui(val_, val_, x);
//          return *this;
//       }
// 
//       long_number & operator /= (long_number const & other)
//       {
//          mpf_div(val_, val_, other.val_);
//          return *this;
//       }
// 
//       long_number & operator /= (int x)
//       {
//          if (x < 0)
//          {
//             mpf_div_ui(val_, val_, -x);
//             mpf_neg(val_, val_);
//          }
//          else
//             mpf_div_ui(val_, val_, x);
//          return *this;
//       }
// 
//       double to_double() const
//       {
//          return mpf_get_d(val_);
//       }
// 
//       int to_int() const
//       {
//          return mpf_get_si(val_);
//       }
// 
//       bool is_integer() const
//       {
//          return mpf_integer_p(val_) != 0;
//       }
// 
//       friend int cmp(long_number const & a, long_number const & b);
// 
//       int cmp(double x) const
//       {
//          return mpf_cmp_d(val_, x);
//       }
// 
//    private:
//       mpf_t val_;
//    };
// 
//    inline int cmp(long_number const & a, long_number const & b)
//    {
//       return mpf_cmp(a.val_, b.val_);
//    }
// 
//    inline long_number operator + (long_number const & a, long_number const & b)
//    {
//       long_number tmp = a;
//       tmp += b;
//       return tmp;
//    }

inline int cmp(long_number const& a, long_number const& b)
{
    if (a < b)
        return -1;
    else if (a > b)
        return 1;
    
    return 0;
}

}
}
}
