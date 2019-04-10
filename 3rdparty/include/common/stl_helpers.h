#pragma once

#include "cpp_utils/type_traits_aux.h"

namespace stl_helpers
{

template<typename Map>
typename Map::mapped_type *map_item(Map &m, typename Map::key_type const &key)
{
    auto it = m.find(key);
    return it != m.end() ? &(it->second) : nullptr;
}

template<typename Map>
typename Map::mapped_type const *map_item(Map const &m, typename Map::key_type const &key)
{
    return const_cast<typename Map::mapped_type const *>(map_item(const_cast<Map &>(m), key));
}

template<typename Container, typename Pred>
typename Container::value_type const *find_item_if(Container const &container, Pred const &pred)
{
    auto it = std::find_if(std::begin(container), std::end(container), pred);
    if (it == std::end(container))
        return nullptr;

    return &(*it);
}

template<size_t Index>
struct index_getter
{
    template<typename T>
    auto operator()(T const &t) const -> decltype(std::get<Index>(t))
    {
        return std::get<Index>(t);
    }
};

typedef index_getter<0> pair_first_getter;
typedef index_getter<1> pair_second_getter;

template<typename Container>
typename Container::value_type const *iter_value(Container const &container, typename Container::const_iterator const &it)
{
    return it == end(container) ? nullptr : &(*it);
}

template<typename Container>
typename Container::value_type *iter_value(Container &container, typename Container::iterator const &it)
{
    return it == end(container) ? nullptr : &(*it);
}

template<typename Dst, typename Src>
optional<Dst> map_optional(optional<Src> const &src, std::function<Dst(Src const&)> f)
{
    if (!src)
        return none;

    return boost::in_place(f(*src));
}

template<typename It>
vector<pair<typename std::iterator_traits<It>::value_type const &, typename std::iterator_traits<It>::value_type const &>>
make_pairs_vector(It begin, It end)
{
    typedef typename std::iterator_traits<It>::value_type value_type;
    typedef pair<value_type const &, value_type const &> pair_type;

    if (begin == end || next(begin) == end)
        return {};

    vector<pair_type> pairs;
    pairs.reserve(std::distance(begin, end) - 1);

    auto prev = begin;
    for (auto it = next(begin); it != end; ++it, ++prev)
        pairs.emplace_back(*prev, *it);

    return pairs;
}

template<typename Container>
vector<pair<typename Container::value_type const &, typename Container::value_type const &>>
make_pairs_vector(Container const &container)
{
    return make_pairs_vector(std::begin(container), std::end(container));
}

template<typename Range>
struct range_wrapper_t
{
    typedef typename Range::const_iterator it_t;
    typedef typename Range::value_type value_type;

    range_wrapper_t(Range const &range)
        : begin_(std::begin(range))
        , end_(std::end(range))
    {}

    template<typename Container>
    operator Container() const
    {
        return convert<Container>();
    }

private:
    template<typename Container>
    Container convert(ENABLE_IF_MACRO(cpp_utils::has_cbegin_cend<Container>::value)) const
    {
        return Container(begin_, end_);
    }

    template<typename Container>
    Container convert(ENABLE_IF_MACRO(!cpp_utils::has_cbegin_cend<Container>::value)) const
    {
        static_assert(false, "Can't convert range to a non-container type");
    }
private:
    it_t begin_, end_;
};

template<typename Range>
range_wrapper_t<Range> container_from_range(Range const &range)
{
    return range_wrapper_t<Range>(range);
}

template<typename T>
struct to_vector_of{};

struct to_vector{};
struct to_container{};

namespace detail
{
    template<typename T>
    struct inner_remove_cv
    {
        typedef T type;
    };

    template<typename T>
    struct my_remove_cv;

    template<typename T1, typename T2>
    struct inner_remove_cv<std::pair<T1, T2>>
    {
        typedef std::pair<
            typename my_remove_cv<T1>::type, 
            typename my_remove_cv<T2>::type 
        > type;
    };
    
    template<typename T>
    struct my_remove_cv
    {
        typedef typename inner_remove_cv<typename std::remove_cv<T>::type>::type type;
    };
    
    
    template<typename Range>
    using range_value_type = typename my_remove_cv<typename std::iterator_traits<typename Range::iterator>::value_type>::type;
}

template<typename Range>                                                            
vector<detail::range_value_type<Range>> operator| (Range const &range, to_vector)
{
    return vector<detail::range_value_type<Range>>(std::begin(range), std::end(range));
}

template<typename Range, typename T>                                                            
vector<T> operator| (Range const &range, to_vector_of<T>)
{
    return vector<T>(std::begin(range), std::end(range));
}
template<typename Range>
auto operator| (Range const &range, to_container) -> range_wrapper_t<Range>
{
    return range_wrapper_t<Range>(range);
}

template<typename Index>
struct indexer_t
{
    template<typename T>
    std::pair<Index, T const&> operator()(T const &v) const
    {
        return make_pair(index_++, v);
    }

private:
    mutable Index index_ = 0;
};

template<typename Index = uint32_t>
indexer_t<Index> indexer()
{
    return indexer_t<Index>();
}
    
template<typename Container, typename F>
auto transformed_container(Container const &container, F const &f) -> decltype(container_from_range(container | boost::adaptors::transformed(f)))
{
    return container_from_range(container | boost::adaptors::transformed(f));
}

template<typename Container1, typename Container2>
auto joined_containers(Container1 const &container1, Container2 const &container2)
    -> decltype(container_from_range(boost::range::join(container1, container2)))
{
    return container_from_range(boost::range::join(container1, container2));
}

template<typename Container1, typename Container2, typename Container3>
auto joined_containers(Container1 const &container1, Container2 const &container2, Container2 const &container3)
    -> decltype(container_from_range(boost::range::join(boost::range::join(container1, container2), container3)))
{
    return container_from_range(boost::range::join(boost::range::join(container1, container2), container3));
}

template<typename T>
optional<T> opt_from_ptr(T const *ptr)
{
    if (!ptr)
        return none;

    return *ptr;
}

template<unsigned int N>
struct nth_less
{
    template<typename T>
    bool operator()(T const &a, T const &b) const
    {
        return std::get<N>(a) < std::get<N>(b);
    };
};

#define SIMPLE_LAMBDA(arg, expr) [](arg) { return expr; }

#define CAPTURE_LAMBDA(arg, expr) [&](arg) { return expr; }

} // namespace stl_helpers