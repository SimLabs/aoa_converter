/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include <functional>
#include <vector>

#undef min
#undef max

// forward declarations
template < class T > struct mapped_vector;

namespace util
{
   // returns size of built-in array
   template < class T, size_t N >
      inline size_t array_size ( T (&) [N] )
   {
      return N;
   }

   // returns end of built-in array
   template < class T, size_t N >
      inline T * array_end ( T (& x)[N] )
   {
      return x + N;
   }

   template < class C, class T, size_t N >
      C make_container( const T (& x)[N] )
    {
        return C(x, x + N) ;
    }

    template < class T, size_t N >
    std::vector<T> vector ( const T (& x)[N] )
    {
        return std::vector<T>(x, x + N) ;
    }

    template <class associative_cont, class content_type>
    bool contains (associative_cont const& container, content_type const& value)
    {
        return container.find(value) != container.end();
    }

   template < class T, typename A >
      T * raw_ptr ( std::vector<T,A>& v )
      {
         return v.empty() ? NULL : &v[0];
      }

   template < class T, typename A >
      T const * raw_ptr ( std::vector<T,A> const& v )
      {
         return v.empty() ? NULL : &v[0];
      }

   template < class T >
      T const * raw_ptr ( mapped_vector<T> const& v )
      {
         return v.empty() ? NULL : &v[0];
      }

   // ------------------------------------------------ some useful functors for pair comparing
   // pair predicate using first
   template< template<class> class Pred >
      struct pair_fst_pred
   {
      template< template<class, class> class Pair, class A, class B >
         bool operator()( Pair<A, B> const& p1, Pair<A, B> const& p2 )  const
      {
         return Pred<A>()(p1.first, p2.first);
      }

      template< template<class, class> class Pair, class A, class B >
         bool operator()( A const& a, Pair<A, B> const& p ) const
      {
         return Pred<A>()(a, p.first);
      }

      template< template<class, class> class Pair, class A, class B >
         bool operator()( Pair<A, B> const& p, A const& a ) const
      {
         return Pred<A>()(p.first, a);
      }
   };

   // pair predicate using second
   template< template<class> class Pred >
      struct pair_snd_pred
   {
      template< template<class, class> class Pair, class A, class B >
         bool operator()( Pair<A, B> const& p1, Pair<A, B> const& p2 )  const
      {
         return Pred<B>()(p1.second, p2.second);
      }

      template< template<class, class> class Pair, class A, class B >
         bool operator()( B const& b, Pair<A, B> const& p ) const
      {
         return Pred<B>()(b, p.second);
      }

      template< template<class, class> class Pair, class A, class B >
         bool operator()( Pair<A, B> const& p, B const& b ) const
      {
         return Pred<B>()(p.second, b);
      }
   };

   typedef pair_fst_pred<std::less>     pair_fst_lt;
   typedef pair_fst_pred<std::greater>  pair_fst_gt;
   typedef pair_fst_pred<std::equal_to> pair_fst_eq;

   typedef pair_snd_pred<std::less>     pair_snd_lt;
   typedef pair_snd_pred<std::greater>  pair_snd_gt;
   typedef pair_snd_pred<std::equal_to> pair_snd_eq;

   // details for make_swap
   namespace details
   {
      template< class Pred >
         struct swap_predicate
      {
         swap_predicate( Pred const& pred ) : pred( pred ) {}

         template< class T1, class T2 >
            bool operator()( T1 const& p1, T2 const& p2 ) const
         {
            return pred(p2, p1);
         }

         Pred pred;
      };
   }

   // swaps binary predicate parameters
   template< class Pred >
      details::swap_predicate< Pred > make_swap( Pred const& pred )
   {
      return details::swap_predicate< Pred >(pred);
   }

   // some details for make_less/make_greater
   namespace details
   {
      template<class C, class T1, template <class> class Pred = std::less>
         struct member_comparer1
      {
         member_comparer1( T1 C::*m1 ) : m1(m1) {}

         template< class P >
            bool operator()( P const& p1, P const& p2 ) const
         {
            return operator()(static_cast<C const&>(*p1), static_cast<C const&>(*p2));
         }

         bool operator()( C const& c1, C const& c2 ) const
         {
            return Pred<T1>()(c1.*m1, c2.*m1);
         }

         T1 C::*m1;
      };

      template<class C, class T1, class T2>
         struct member_comparer2
      {
         member_comparer2( T1 C::*m1, T2 C::*m2 ) : m1(m1), m2(m2) {}

         template< class P >
            bool operator()( P const& p1, P const& p2 ) const
         {
            return operator()(static_cast<C const&>(*p1), static_cast<C const&>(*p2));
         }

         bool operator()( C const& c1, C const& c2 ) const
         {
            return (c1.*m1) < (c2.*m1) || ((c1.*m1) == (c2.*m1) && (c1.*m2) < (c2.*m2));
         }

         T1 C::*m1;
         T2 C::*m2;
      };

      template<class C, class T1, template <class> class Pred>
         struct mem_fun_comparer1
      {
         mem_fun_comparer1( T1 (C::*m1)() const ) : m1(m1) {}

         template< class P >
            bool operator()( P const& p1, P const& p2 ) const
         {
            return operator()(static_cast<C const&>(*p1), static_cast<C const&>(*p2));
         }

         bool operator()( C const& c1, C const& c2 ) const
         {
            return Pred<T1>()((c1.*m1)(), (c2.*m1)());
         }

         T1 (C::*m1)() const;
      };

      template<class C, class T1, class T2>
         struct mem_fun_comparer2
      {
         mem_fun_comparer2( T1 (C::*m1)() const, T2 (C::*m2)() const ) : m1(m1), m2(m2) {}

         template< class P >
            bool operator()( P const& p1, P const& p2 ) const
         {
            return operator()(static_cast<C const&>(*p1), static_cast<C const&>(*p2));
         }

         bool operator()( C const& c1, C const& c2 ) const
         {
            return ((c1.*m1)() < (c2.*m1)()) || (((c1.*m1)() == (c2.*m1)()) && ((c1.*m2)() < (c2.*m2)()));
         }

         T1 (C::*m1)() const;
         T2 (C::*m2)() const;
      };

      template<class C, class T1>
         struct member_is1
      {
         member_is1( T1 C::*m1, T1 const& v ) : m1(m1), v(v) {}

         template< class P >
            bool operator()( P const& p ) const
         {
            return operator()(static_cast<C const&>(p));
         }

         bool operator()( C const& c ) const
         {
            return (c.*m1) == v;
         }

         T1 C::*m1;
         T1 v;
      };

      template<class C, class T1>
         struct mem_fun_is1
      {
         mem_fun_is1( T1 (C::*m1)() const, T1 const& v ) : m1(m1), v(v) {}

         template< class P >
            bool operator()( P const& p ) const
         {
            return operator()(static_cast<C const&>(p));
         }

         bool operator()( C const& c ) const
         {
            return (c.*m1)() == v;
         }

         T1 (C::*m1)() const;
         T1 v;
      };
   }

   /* -------------------------------------------------------------------------------------------------
    *   Functions make_less/make_greater creates less/greater predicates by one or two member pointers.
    *   For example:
    *   std::vector< point_2i > v;
    *   ...
    *
    *   //sorts points by y coordinate
    *   std::sort( v.begin(), v.end(), make_less(&point_2i::y));
    *   //sorts points by y coordinate and then by x
    *   std::sort( v.begin(), v.end(), make_less(&point_2i::y, &point_2i::x));
    */
   template<class C, class T1>
      details::member_comparer1<C, T1, std::less>
         make_less( T1 C::*m1 )
   {
      return details::member_comparer1<C, T1, std::less>(m1);
   }

   template<class C, class T1, class T2>
      details::member_comparer2<C, T1, T2>
         make_less( T1 C::*m1, T2 C::*m2 )
   {
      return details::member_comparer2<C, T1, T2>(m1, m2);
   }

   template<class C, class T1>
      details::member_comparer1<C, T1, std::greater>
         make_greater( T1 C::*m1 )
   {
      return details::member_comparer1<C, T1, std::greater>(m1);
   }

   template<class C, class T1, class T2>
      details::swap_predicate< details::member_comparer2<C, T1, T2> >
         make_greater( T1 C::*m1, T2 C::*m2 )
   {
      return make_swap( details::member_comparer2<C, T1, T2>(m1, m2) );
   }

   template<class C, class T1>
      details::mem_fun_comparer1<C, T1, std::less>
         make_less( T1 (C::*m1)() const )
   {
      return details::mem_fun_comparer1<C, T1, std::less>(m1);
   }

   template<class C, class T1, class T2>
      details::mem_fun_comparer2<C, T1, T2>
         make_less( T1 (C::*m1)() const, T2 (C::*m2)() const)
   {
      return details::mem_fun_comparer2<C, T1, T2>(m1, m2);
   }

   template<class C, class T1>
      details::mem_fun_comparer1<C, T1, std::greater>
         make_greater( T1 (C::*m1)() )
   {
      return details::mem_fun_comparer1<C, T1, std::greater>(m1);
   }

   template<class C, class T1, class T2>
      details::swap_predicate< details::mem_fun_comparer2<C, T1, T2> >
         make_greater( T1 (C::*m1)(), T2 (C::*m2)() )
   {
      return make_swap( details::mem_fun_comparer2<C, T1, T2>(m1, m2) );
   }

   template<class C, class T1>
      details::mem_fun_comparer1<C, T1, std::equal_to>
         make_eq( T1 (C::*m1)() )
   {
      return details::mem_fun_comparer1<C, T1, std::equal_to>(m1);
   }

   template<class C, class T1>
      details::member_comparer1<C, T1, std::equal_to>
         make_eq( T1 C::*m1 )
   {
      return details::member_comparer1<C, T1, std::equal_to>(m1);
   }

   template<class C, class T1>
      details::mem_fun_is1<C, T1>
         make_is( T1 (C::*m1)(), T1 const& v )
   {
      return details::mem_fun_is1<C, T1>(m1, v);
   }

   template<class C, class T1>
      details::member_is1<C, T1>
         make_is( T1 C::*m1, T1 const& v )
   {
      return details::member_is1<C, T1>(m1, v);
   }

   template<class FwdIter, class Fn>
      Fn for_each_fst(FwdIter p, FwdIter q, Fn f)
   {
      for (; p != q; ++p)
         f(p->first);
      return f;
   }

   template<class FwdIter, class Fn>
      Fn for_each_snd(FwdIter p, FwdIter q, Fn f)
   {
      for (; p != q; ++p)
         f(p->second);
      return f;
   }

      template<class T>
      std::set<T> set_intersect(std::set<T> const& s1, std::set<T> const& s2)
      {
          std::set<T> isection;
          std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(isection, isection.end()));
          return std::move(isection);
      }


   /* -------------------------------------------------------------------------------------------------
    *   Functors for find_if over collection of pairs.
    */

   namespace details
   {
      template< class T >
         struct fst_is_type
      {
         fst_is_type( T const& t ) : t_(t) {}
         template< class P > bool operator()( P const& p ) const { return p.first == t_; }

      private:
         T const t_;
      };

      template< class T >
         struct snd_is_type
      {
         snd_is_type( T const& t ) : t_(t) {}
         template< class P > bool operator()( P const& p ) const { return p.second == t_; }

      private:
         T const t_;
      };
   }

   template< class T >
      details::fst_is_type<T> fst_is( T const& t )
   {
      return details::fst_is_type<T>(t);
   }

   template< class T >
      details::snd_is_type<T> snd_is( T const& t )
   {
      return details::snd_is_type<T>(t);
   }

   template <class Pair>
      struct select1st : public std::unary_function<Pair, typename Pair::first_type>
   {
      typename Pair::first_type const& operator()(Pair const& p) const { return p.first; }
   };

   template <class Pair>
      struct select2nd : public std::unary_function<Pair, typename Pair::second_type>
   {
      typename Pair::second_type const& operator()(Pair const& p) const { return p.second; }
   };

// for vectors
   template < class T, class IndicesOutIter >
      std::vector< T > flatten( std::vector< std::vector< T > > const & v, IndicesOutIter indices )
   {
      std::vector< T > res;
      for ( size_t i = 0, offset = 0; i < v.size(); ++i, ++indices )
      {
         std::vector< size_t > ind;
         ind.reserve(v[i].size());

         for ( size_t j = 0; j < v[i].size(); ++j )
         {
            res.push_back( v[i][j] );
            ind.push_back( offset + j );
         }

         *indices = ind;
         offset += v[i].size();
      }

      return res;
   }

   struct crtp_default_tag {};

   template < class Derived, class Tag = crtp_default_tag >
      struct crtp
   {
   protected:
      crtp() {}
      __forceinline Derived &         self()         { return static_cast< Derived & >( *this ); }
      __forceinline Derived const &   self() const   { return static_cast< Derived const & >( *this ); }
   };
}
