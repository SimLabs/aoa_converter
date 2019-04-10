/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <limits>
#include <boost/static_assert.hpp>

#include <boost/date_time.hpp>
#include <boost/random.hpp>

#ifdef _WIN32
    #include "common/performance_counter.h"
#endif

#pragma push_macro("RANDGEN_ASSERT")
#undef RANDGEN_ASSERT
#ifdef _DEBUG
#  define RANDGEN_ASSERT(x) void((x) || (__debugbreak(), 0))
#else
#  define RANDGEN_ASSERT(x) void(sizeof(x))
#endif


namespace randgen_impl
{
   template <typename Int>
   struct uniform_fastint
   {
      BOOST_STATIC_ASSERT(std::numeric_limits<Int>::is_integer);

      typedef double real_type;
      typedef boost::uniform_real<real_type> realdist_t;

      typedef realdist_t::input_type input_type;
      typedef Int result_type;

      uniform_fastint(Int min_arg, Int max_arg):
         min_(min_arg),
         max_(max_arg),
         realdist_(real_type(min_arg), real_type(max_arg) + 1)
      {
         RANDGEN_ASSERT(min_arg <= max_arg);
      }

      result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const {return min_;}
      result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const {return max_;}

      void reset() {}

      template <typename Engine>
      result_type operator () (Engine& eng)
      {
         result_type r;

         // it can happen, that realdist_(eng) will return _exactly_ max_ + 1;
         // r will be equal max_ + 1 too. therefore, workaround is required
         while ((r = static_cast<result_type>(realdist_(eng))) == max_ + 1)
            ;

         RANDGEN_ASSERT(r >= min_ && r <= max_);
         return r;
      }

   private:
      result_type const min_;
      result_type const max_;

      realdist_t realdist_;
   };


   template <typename Numeric, bool IsIntegerType>
   struct uniform_dist_helper;

   template <typename Int>
   struct uniform_dist_helper<Int, true>
   {
      typedef uniform_fastint<Int> type;
   };

   template <typename Real>
   struct uniform_dist_helper<Real, false>
   {
      typedef boost::uniform_real<Real> type;
   };


   template <typename Numeric>
   struct uniform_dist
   {
      typedef typename uniform_dist_helper<Numeric, std::numeric_limits<Numeric>::is_integer>::type type;
   };


   // this exists to not trigger runtime debug check "cast to smaller type causes loss of data"
   template <typename T>
   static inline T rdtsc_casted_to()
   {
       return T(PerformanceCounter::get_counter());
   }

   template <typename RandGen>
   struct stl_randgen_model
   {
      stl_randgen_model(RandGen& rg): rg_(rg) {}

      unsigned operator () (unsigned N)
      {
         RANDGEN_ASSERT(N > 0);
         return rg_(N - 1);
      }

   private:
      RandGen& rg_;
   };
}


struct randgen_seed_tag {};


template <typename BaseGen = boost::mt19937>
struct randgen
{
   typedef BaseGen basegen_t;

   randgen() {}

   randgen(randgen_seed_tag)
   {
      seed();
   }

   randgen(typename basegen_t::result_type s)
   {
      seed(s);
   }

public:
   basegen_t& basegen()
   {
      return basegen_;
   }

   basegen_t const& basegen() const
   {
      return basegen_;
   }

public:
   bool flip(double true_probability = 0.5)
   {
      typedef boost::bernoulli_distribution<double>        dist_t;
      typedef boost::variate_generator<basegen_t&, dist_t> gen_t;

      RANDGEN_ASSERT(true_probability >= 0 && true_probability <= 1);

      dist_t dist(true_probability);
      gen_t  gen(basegen_, dist);

      return gen();
   }

public:
   typename basegen_t::result_type operator () ()
   {
      return basegen_();
   }

   template <typename Numeric>
   Numeric operator () (Numeric max_arg)
   {
      RANDGEN_ASSERT(max_arg >= 0);

      Numeric const r = (*this)(static_cast<Numeric>(0), max_arg);

      RANDGEN_ASSERT(r >= 0 && r <= max_arg);
      return r;
   }

   template <typename Numeric>
   Numeric operator () (Numeric min_arg, Numeric max_arg)
   {
      typedef typename randgen_impl::uniform_dist<Numeric>::type    dist_t;
      typedef typename boost::variate_generator<basegen_t&, dist_t> gen_t;

      RANDGEN_ASSERT(min_arg <= max_arg);

      dist_t dist(min_arg, max_arg);
      gen_t  gen(basegen_, dist);

      Numeric const r = gen();

      RANDGEN_ASSERT(r >= min_arg && r <= max_arg);
      return r;
   }

public:
   void seed()
   {
      seed(randgen_impl::rdtsc_casted_to<typename basegen_t::result_type>());
   }

   void seed(typename basegen_t::result_type s)
   {
      basegen_.seed(s);
   }

private:
   basegen_t basegen_;
};


// helper for use in e.g. std::random_shuffle

template <typename RandGen>
inline randgen_impl::stl_randgen_model<RandGen> make_stl_randgen(RandGen& rg)
{
   return randgen_impl::stl_randgen_model<RandGen>(rg);
}


//
// Fast random generator
//

class simplerandgen
{

public:

    simplerandgen() : seed_(0) {}
    simplerandgen(unsigned seed) : seed_(seed) {}

    void reseed(unsigned seed) const { seed_ = seed; }

    // floating point randoms
    __forceinline float random_unit() const
    {
        static const float g_Mul = 1.0f / 65535.f;
        return g_Mul * random_16bit();
    }
    __forceinline float random_unit_signed() const
    {
        static const float g_Mul = 2.0f / 65535.f;
        return g_Mul * random_16bit() - 1.0f;
    }
    __forceinline float random_dev(float val, float dev) const
    {
        return val + random_unit_signed() * dev;
    }
    __forceinline float random_range(float min_val, float max_val) const
    {
        return min_val + random_unit() * (max_val - min_val);
    }

    // integer randoms
    __forceinline unsigned random_8bit() const 
    {
        seed_iterate();
        return (seed_ >> 16) & 0xFF;
    }
    __forceinline unsigned random_16bit() const
    {
        seed_iterate();
        return (seed_ >> 8) & 0xFFFF;
    }
    __forceinline unsigned random_32bit() const
    {
        seed_iterate();
        return seed_;
    }

    // iterate seed
    __forceinline void seed_iterate() const
    {
        seed_ = (214013 * seed_ + 2531011);
    }

private:

    mutable unsigned seed_;
};


#pragma pop_macro("RANDGEN_ASSERT")
