/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/primitives/point_fwd.h"
#include "geometry/scalar_traits.h"


namespace util {

   namespace detail {

      template<typename scalar, size_t idx>
      struct reset_sign_bit_func_t
      {
         static __forceinline void apply( scalar * data )
         {
            *(((unsigned char *)(data + idx)) + sizeof(scalar) - 1) &= 0x7F;
         }
      };

      template<typename scalar, size_t idx>
      struct reset_func_t
      {
         static __forceinline void apply( scalar * data )
         {
            *(data + idx) = 0;
         }
      };

      template<typename scalar, size_t idx>
      struct clamp_zero_func_t
      {
         static __forceinline void apply( scalar * data )
         {
            if (*(((unsigned char *)(data + idx)) + sizeof(scalar) - 1) & 0x80)
               reset(data + idx);//static_transform_impl_t<DWORD, sizeof(scalar) / sizeof(DWORD), reset_func_t>::apply((DWORD *)(data + idx));
            // static_transform_impl_t - poor code with doubles, todo
         }

         static __forceinline void reset( float * data )
         {
            *(uint32_t *)data = 0;
         }

         static __forceinline void reset( double * data )
         {
            *(uint64_t *)data = 0;
         }
      };


      template<typename scalar, size_t size, template<typename,size_t> class predicate>
      struct static_transform_impl_t
      {
         static __forceinline void apply( scalar * data )
         {
            predicate<scalar, size - 1>::apply(data);
            static_transform_impl_t<scalar, size - 1, predicate>::apply(data);
         }
      };

      template<typename scalar, template<typename,size_t> class predicate>
      struct static_transform_impl_t<scalar, 0, predicate>
      {
         static __forceinline void apply( scalar * /*data*/ )
         {
         }
      };
   }


   template<typename scalar, size_t size>
   __forceinline void vector_reset_sign_bit( scalar (&data)[size] )
   {
      detail::static_transform_impl_t<scalar, size, detail::reset_sign_bit_func_t>::apply(data);
   }

   template<typename scalar, size_t dim>
   __forceinline void vector_reset_sign_bit( geom::point_t<scalar,dim> & data )
   {
      detail::static_transform_impl_t<scalar, dim, detail::reset_sign_bit_func_t>::apply(&data.x);
   }

   template<typename scalar>
   __forceinline void scalar_clamp_zero( scalar & data )
   {
      detail::clamp_zero_func_t<scalar, 0>::apply(&data);
   }

   template<typename scalar, size_t size>
   __forceinline void vector_clamp_zero( scalar (&data)[size] )
   {
      detail::static_transform_impl_t<scalar, size, detail::clamp_zero_func_t>::apply(data);
   }

   template<typename scalar, size_t dim>
   __forceinline void vector_clamp_zero( geom::point_t<scalar,dim> & data )
   {
      detail::static_transform_impl_t<scalar, dim, detail::clamp_zero_func_t>::apply(&data.x);
   }
}
