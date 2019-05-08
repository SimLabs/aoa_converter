/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "io_streams.h"
#include "cpp_utils/type_traits_aux.h"
#include "reflection/reflection_aux.h"
#include "binary_serializable.h"
#include "make_default.h"

namespace binary 
{

namespace type_traits
{
    // type traits
    template<class T, class S>
    struct is_equal_type
    {
        static const bool value = false;
    };

    template<class T>
    struct is_equal_type < T, T >
    {
        static const bool value = true;
    };

    template<class T>
    struct is_continues_type
    {
#ifdef __linux__
        enum { value = std::is_pod<T>::value && !is_equal_type<T, bool>::value };
#else
        // WARNING: std::is_standard_layout in MSVC 2010 is alias for std::is_pod. MSVC 2012 fixes this bug
        // ToDo: consider to use is_trivial_copyable instead of is_pod, it will increase speed significantly for types like point_3

#if _MSC_VER >= 1800
        enum { value = std::is_pod<T>::value && !is_equal_type<T, bool>::value };
#elif _MSC_VER == 1600
        enum { value = std::is_standard_layout<T>::value && !is_equal_type<T, bool>::value };
#else
#pragma error "Unsupported compiler for Windows"
#endif
#endif
    };

    template<class T>
    struct is_primitive 
    { 
        enum 
        { 
            value = std::is_pod<T>::value
        }; 
    };

    template<class T>
    struct is_container
    {
        enum
        {
            // better use cbegin and rbegin to separate our containers from STL and boost ones
            value = cpp_utils::has_rbegin_rend<T>::value || cpp_utils::has_cbegin_cend<T>::value
        };
    };

    template<class container>
    using if_continues_value_type = typename std::enable_if< is_continues_type<typename container::value_type>::value>::type;

    template<class container>
    using if_not_continues_value_type = typename std::enable_if<!is_continues_type<typename container::value_type>::value>::type;

    template<class T>   
    using if_primitive = typename std::enable_if< is_primitive<T>::value >::type;
    
    template<class container>
    using if_container = typename std::enable_if< is_container<container>::value>::type;

} // type_traits

//////////////////////////////////////////////////////////////////////////
// forward declarations 
template <class T>
T read_stream(binary::input_stream& is);
                               
//////////////////////////////////////////////////////////////////////////
// common functions
template<class T>
void write(output_stream& os, std::shared_ptr<T> const& ptr);
template<class T>
void read(input_stream& is, std::shared_ptr<T>& ptr);

//POD types, using SFINAE (including arithmetic types)
template<class type>
void write( output_stream& os, type object, 
            type_traits::if_primitive<type>* = 0)
{
    static_assert(!std::is_enum<type>::value || sizeof(type) == 4, "Only 4-byte enums are supported");
    os.append(&object, sizeof(object));
}

template<class type>
void read(  input_stream& is, type& object, 
            type_traits::if_primitive<type>* = 0)
{
    static_assert(!std::is_enum<type>::value || sizeof(type) == 4, "Only 4-byte enums are supported");
    is.read(&object, sizeof(object));
}

// special case - bool, for cross platform and architecture compatibility 
inline void write(output_stream& os, bool value )   { write(os, size_type(value)); }
inline void read ( input_stream& is, bool& value)   { value = (read_stream<size_type>(is) != 0); }

// some STL containers

// continuous_range

template<class container>
void write_continuous_range(output_stream& os, container const& cnt, type_traits::if_continues_value_type<container>* = 0)
{
    write(os, size_type(cnt.size()));
    
    if (!cnt.empty())
        os.append(cnt.data(), cnt.size() * sizeof(typename container::value_type));
}

template<class container>
void read_continuous_range(input_stream& is, container& cnt, type_traits::if_continues_value_type<container>* = 0)
{
    size_type size;
    read(is, size);

    cnt.resize(size, serialization::make_default<typename container::value_type>());
    
    if (!cnt.empty())
        is.read(cnt.data(), cnt.size() * sizeof(typename container::value_type));
}

// special case for std::array
template<class type, size_t size>
void read_continuous_range(input_stream& is, std::array<type, size>& arr)
{
    size_type sz;
    read(is, sz);

    Assert(size == sz);

    if (type_traits::is_continues_type<type>::value)
    {
        is.read(arr.data(), arr.size() * sizeof(type));
    }
    else
    {
        for (size_t i = 0; i < size; ++i)
            read(is, arr[i]);
    }
}
//

template<class container>
void write_continuous_range(output_stream& os, container const& cnt, type_traits::if_not_continues_value_type<container>* = 0)
{
    write(os, size_type(cnt.size()));
    for (size_t i = 0; i < cnt.size(); ++i)
        write(os, cnt[i]);
}

template<class container>
void read_continuous_range( input_stream& is, container& cnt, type_traits::if_not_continues_value_type<container>* = 0)
{
    size_type size;
    read(is, size);

    cnt.resize(size, serialization::make_default<typename container::value_type>());
    for (size_t i = 0; i < size; ++i)
        read(is, cnt[i]);
}

// string
template<class C, class T, class A>
void write(output_stream& os, std::basic_string<C, A, T> const& str)
{
    os.append(str.c_str(), str.size() + 1);
}

template<class C, class T, class A>
void read(input_stream& is, std::basic_string<C, A, T>& str)
{
    str = is.curr();
    is.skip(str.size() + 1);
}

// vector<T, A>
template<class T, class A>
void write(output_stream& os, std::vector<T, A> const& vec)
{
    write_continuous_range(os, vec);
}

template<class T, class A>
void read(input_stream& is, std::vector<T, A>& vec)
{
    read_continuous_range(is, vec);
}

// std::array<T, N>
template<class T, size_t N>
void write(output_stream& os, std::array<T, N> const& arr)
{
    write_continuous_range(os, arr);
}

template<class T, size_t N>
void read(input_stream& is, std::array<T, N>& arr)
{              
    read_continuous_range(is, arr);    
}

// std::pair<T1, T2>
template<class T1, class T2>
void write(output_stream& os, std::pair<T1, T2> const& pr)
{
    write(os, pr.first );
    write(os, pr.second);
}

template<class T1, class T2>
void read(input_stream& is, std::pair<T1, T2>& pr)
{
    read(is, const_cast<typename std::remove_const<T1>::type&>(pr.first)); // const cast for map::value_type 
    read(is, pr.second);
}

// common functions for most containers 
template<class type>
void write(output_stream& os, type const& cont, type_traits::if_container<type>* = 0)
{
    write(os, size_type(cont.size()));

    for (auto it = cont.begin(); it != cont.end(); ++it)
        write(os, *it);
}

template<class type>
void read(input_stream& is, type& cont, type_traits::if_container<type>* = 0)
{
    cont.clear();

    size_type sz = 0;
    read(is, sz);

    for (size_t i = 0; i < sz; ++i)
    {
        typename type::value_type value;
        read(is, value);

        cont.insert(cont.end(), std::move(value));
    }
}

// optional<T>
template <class T>
void write(output_stream& os, boost::optional<T> const& opt)
{
    write(os, size_type(opt.is_initialized()));
    if (opt)
        write(os, *opt);
}


template <class T>
void read(input_stream& is, boost::optional<T>& opt)
{
    size_type init;
    read(is, init);

    opt.reset();
    if (init)
    {
        T value = serialization::make_default<T>();
        read(is, value);
        opt = std::move(value);
    }
}


namespace detail
{
    template<typename T>
    void write_shared_ptr_impl(output_stream& os, std::shared_ptr<T> const& ptr, ENABLE_IF_BEGIN<!std::is_base_of<serializable, T>::value>ENABLE_IF_END)
    {
        write(os, *ptr);
    }

    template<typename T>
    void read_shared_ptr_impl(input_stream& is, std::shared_ptr<T>& ptr, ENABLE_IF_BEGIN<!std::is_base_of<serializable, T>::value>ENABLE_IF_END)
    {
        typedef typename std::remove_const<T>::type non_const_T;
        ptr = make_shared<non_const_T>();
        read(is, *ptr);
    }

    template<typename T>
    void write_shared_ptr_impl(output_stream& os, std::shared_ptr<T> const& ptr, ENABLE_IF_BEGIN<std::is_base_of<serializable, T>::value>ENABLE_IF_END)
    {
        write(os, serializable::begin_magic);
        ptr->write_to_stream(os);
        write(os, serializable::end_magic);
    }

    template<typename T>
    void read_shared_ptr_impl(input_stream& is, std::shared_ptr<T>& ptr, ENABLE_IF_BEGIN<std::is_base_of<serializable, T>::value>ENABLE_IF_END)
    {
        uint32_t magic;
        read(is, magic);
        Verify(magic == serializable::begin_magic);

        create_from_binary_stream(is, ptr);

        read(is, magic);
        Verify(magic == serializable::end_magic);
    }

} // namespace detail


// shared_ptr<T>
template<class T>
void write(output_stream& os, std::shared_ptr<T> const& ptr)
{
    write(os, size_type(bool(ptr)));
    if (ptr)
        detail::write_shared_ptr_impl<T>(os, ptr);
}               

template<class T>
void read(input_stream& is, std::shared_ptr<T>& ptr)
{
    size_type init;
    read(is, init);

    ptr.reset();
    if (init)
        detail::read_shared_ptr_impl(is, ptr);
}

// bytes_array
inline void write(output_stream& os, bytes_array const& arr)
{
    write(os, size_type(arr.size()));
    os.append(arr.data(), arr.size());
}

inline void read(input_stream& is, bytes_array& arr)
{
    size_type sz = 0;
    read(is, sz);
    
    arr.resize(sz);
    is.read(arr.data(), sz);
}

//////////////////////////////////////////////////////////////////////////
// special function for reading binary data 

// type_traits::bytes_reader_impl
template<class T>
void write(output_stream& os, refl::bin_adaptor_impl<T> const& ba)
{
    write(os, ba.ref());
}

template<class T>
void read(input_stream& is, refl::bin_adaptor_impl<T>& ba)
{
    read(is, ba.ref());
}


//////////////////////////////////////////////////////////////////////////
template <class T>
T read_stream(input_stream& is)
{
    T t;
    read(is, t);

    return t;
}

//////////////////////////////////////////////////////////////////////////
// shorter way serialization 

template<class type>
bytes_t wrap(type const& object)
{
    output_stream os;
    write(os, object);

    return os;
}

template<class type>
void unwrap(bytes_cref data, type& object)
{
    input_stream is(data);
    read(is, object);
}

template<class type>
type unwrap(bytes_cref data)
{
    input_stream is(data);
    return read_stream<type>(is);
}



} // binary
