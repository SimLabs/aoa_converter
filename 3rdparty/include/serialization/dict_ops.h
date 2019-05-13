/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

#include <type_traits>
#include <cpp_utils/type_traits_aux.h>

#include "reflection/proc/io_streams_refl.h"
#include "dict.h"

#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/device/back_inserter.hpp"

//#include "info_io/info_io.h"
#include "dict_serializable.h"

namespace dict 
{
namespace type_traits
{
    // 
    template<class T, class S>
    struct is_equal_type { static const bool value = false; };

    template<class T>
    struct is_equal_type < T, T > { static const bool value = true; };

    template<class T>
    struct is_leaf_type
    {
        enum
        {
            value =
            (is_equal_type<T, std::string>   ::value ||
            is_equal_type<T, binary::bytes_t>::value ||
            std::is_arithmetic<T>            ::value
            )

        };
    };

    template<class T>
    struct is_container
    {
        enum
        {
            // better use cbegin and rbegin to separate our containers from STL and boost ones

            value =
            (cpp_utils::has_rbegin_rend<T>::value ||
            cpp_utils::has_cbegin_cend<T>::value) &&
            !is_leaf_type<T>::value
        };
    };

    template <class T>
    struct is_simple_type
    {
        enum
        {
            value = 
            (is_leaf_type<T>::value &&
            !is_equal_type<T, binary::bytes_t>::value
            )
        };
    };

    //
    template<class T>
    using if_leaf_type = typename std::enable_if<is_leaf_type<T>::value>::type;

    template<class T>
    using if_container = typename std::enable_if<is_container<T>::value>::type;

    template<class T>
    using if_simple_type = typename std::enable_if<is_simple_type<T>::value>::type;

    template<class T>
    using if_aggregate = typename std::enable_if<!is_leaf_type<T>::value && !is_container<T>::value>::type;

    template<class T>
    using if_enum = std::enable_if_t<std::is_enum_v<T>>;

} // type_traits

//////////////////////////////////////////////////////////////////////////
namespace details
{

    //
    template<class T>
    struct wider_type_for_std_stream
    {
        wider_type_for_std_stream(T const& val)
            : value(val)
        {}

        T const& value;
    };

    inline std::ostream& operator << (std::ostream& os, wider_type_for_std_stream<char> const& t)
    {
        os << static_cast<int>(t.value);
        return os;
    }

    inline std::ostream& operator << (std::ostream& os, wider_type_for_std_stream<unsigned char> const& t)
    {
        os << static_cast<unsigned int>(t.value);
        return os;
    }

    template<class T>
    typename std::enable_if<std::is_enum<T>::value, std::ostream&>::type
        operator << (std::ostream& os, wider_type_for_std_stream<T> const& t)
    {
        static_assert(sizeof(T) == 4, "Only 4-byte enums are supported");
        os << static_cast<uint32_t>(t.value);
        return os;
    }

    template<class T>
    typename std::enable_if<!std::is_enum<T>::value, std::ostream&>::type
        operator << (std::ostream& os, wider_type_for_std_stream<T> const& t)
    {
        os << t.value;
        return os;
    }

    template<class T>
    struct is_char_type
    {
        enum {
            value =
            std::is_same<T, char>::value ||
            std::is_same<T, unsigned char>::value
        };
    };

    template<class T> struct wider_type;

    template<> struct wider_type < char > { typedef int type; };
    template<> struct wider_type < unsigned char > { typedef unsigned int type; };

    //

    struct bin_translator
    {
        typedef dict_t::data_t data_t;

        template<class T>
        static void write(data_t& data, T const& value)
        {
            binary::write(data, value);
        }

        template<class T>
        static void read(data_t const& data, T& value)
        {
            binary::input_stream is(data);
            binary::read(is, value);
        }

        // binary::bytes_t
        static void write(data_t& data, data_t const& value)
        {
            data = value;
        }

        static void read(data_t const& data, data_t& value)
        {
            value = data;
        }
    };


    struct str_translator
    {
        typedef dict_t::data_t data_t;

        template<class T>
        static void write(data_t& arr, T const& value, bool& bin_format)
        {
            namespace io = boost::iostreams;

            //
            struct append_device
            {
                typedef data_t::byte_type   char_type;
                typedef io::sink_tag        category;

                append_device(data_t& arr)
                    : arr_(&arr)
                {}

                std::streamsize write(const char_type* s, std::streamsize n)
                {
                    arr_->append(s, n);
                    return n;
                }
            protected:
                data_t* arr_;
            };

            //
            io::filtering_ostream out((append_device(arr)));
            out << std::setprecision(9) << std::boolalpha << wider_type_for_std_stream<T>(value); // filtering_ostream doesn't append '\0' automatically 

            bin_format = false;
        }

        template<class T>
        static
            typename std::enable_if<!std::is_enum<T>::value, void>::type
            read_generic(data_t const& arr, T& value)
        {
            namespace io = boost::iostreams;
            io::filtering_istream in(boost::make_iterator_range(arr.data(), arr.data() + arr.size()));
            in >> value;
        }

        template<class T>
        static
            typename std::enable_if<std::is_enum<T>::value, void>::type
            read_generic(data_t const& arr, T& value)
        {
            static_assert(sizeof(T) == 4, "Only 4-byte enums are supported");
            uint32_t temp;
            read_generic(arr, temp);
            value = static_cast<T>(temp);
        }

        template<class T>
        static void read(data_t const& arr, T& value, typename std::enable_if<!is_char_type<T>::value>::type * = nullptr)
        {
            read_generic(arr, value);
        }

        static void read(data_t const& arr, string & value)
        {
            value.assign(arr.begin(), arr.end());
        }

        // For correct reading bool in boolalpha form (default form is numeric)
        static void read(data_t const& arr, bool& value)
        {
            namespace io = boost::iostreams;
            io::filtering_istream in(boost::make_iterator_range(arr.data(), arr.data() + arr.size()));
            in >> value;

            if (in.fail())
            {
                // Try again in word form.
                in.clear();
                in.setf(std::ios_base::boolalpha);
                in >> value;
            }
        }

        template<class T>
        static void read(data_t const& arr, T& value, typename std::enable_if<is_char_type<T>::value>::type * = nullptr)
        {
            namespace io = boost::iostreams;
            io::filtering_istream in(boost::make_iterator_range(arr.data(), arr.data() + arr.size()));

            char a = 0, b = 0;
            in >> a >> value >> b;
            if (a != '\'' || b != '\'' || in.fail())
            {
                typename wider_type<T>::type wider_value;
                io::filtering_istream inin(boost::make_iterator_range(arr.data(), arr.data() + arr.size()));
                inin >> wider_value;
                value = static_cast<T>(wider_value);
            }
        }

        // binary::bytes_t
        static void write(data_t& data, data_t const& value, bool& bin_format)
        {
            data = value;
            bin_format = true;
        }

        static void read(data_t const& data, data_t& value)
        {
            value = data;
        }
    };

    //

    template <class T>
    dict_t::type_kind_t get_type_kind(T const& /*value*/, type_traits::if_aggregate<T>*  = nullptr)
    {
        return dict_t::TK_OBJECT;
    }

    template <class T>
    dict_t::type_kind_t get_type_kind(T const& /*value*/, 
        ENABLE_IF_BEGIN<type_traits::is_equal_type<T, std::string>::value>ENABLE_IF_END)
    {
        return dict_t::TK_STRING;
    }

    template <class T>
    dict_t::type_kind_t get_type_kind(T const& /*value*/, 
        ENABLE_IF_BEGIN<type_traits::is_equal_type<T, bool>::value>ENABLE_IF_END)
    {
        return dict_t::TK_BOOLEAN;
    }

    template <class T>
    dict_t::type_kind_t get_type_kind(T const& /*value*/,
        ENABLE_IF_BEGIN<
        !std::is_same<T, bool>::value &&
        (std::is_integral<T>::value || std::is_enum<T>::value)>
        ENABLE_IF_END)
    {
        return dict_t::TK_INTEGRAL;
    }

    template <class T>
    dict_t::type_kind_t get_type_kind(T const& /*value*/,
        ENABLE_IF_BEGIN<
        std::is_floating_point<T>::value>
        ENABLE_IF_END)
    {
        return dict_t::TK_FLOATING_POINT;
    }

    template <class T, class A>
    dict_t::type_kind_t get_type_kind(std::vector<T, A> const& /*vec*/)
    {
        return dict_t::TK_ARRAY;
    }

    template <class T, size_t S>
    dict_t::type_kind_t get_type_kind(std::array<T, S> const& /*arr*/)
    {
        return dict_t::TK_ARRAY;
    }

    template <class K, class V, class P, class A>
    dict_t::type_kind_t get_type_kind(std::map<K, V, P, A> const& /*map*/, 
        type_traits::if_simple_type<K>* = nullptr)
    {
        return dict_t::TK_SIMPLE_MAP;
    }

    template <class container>
    dict_t::type_kind_t get_type_kind(container const& /*container*/, type_traits::if_container<container>* = nullptr)
    {
        return dict_t::TK_ARRAY;
    }

    template<class value>
    dict_t::type_kind_t get_type_kind(optional<value> const&)
    {
        return dict_t::TK_NULL;
    }
} // details


using details::bin_translator;
using details::str_translator;


//////////////////////////////////////////////////////////////////////////
// general read/write functions

//- arithmetic types, bytes_t & string, using SFINAE
template<class type>
void write( 
    dict_t&         dict, 
    type const&     value, 
    
    // leaf type
    type_traits::if_leaf_type<type>* = 0)
{
    bool bin_fmt = dict.bin_format();

    if (bin_fmt)
        bin_translator::write(dict.data(), value);
    else                                        
        str_translator::write(dict.data(), value, bin_fmt);

    dict.type_kind() = details::get_type_kind(value);
    dict.reset_format(bin_fmt);
}

template<class type>
void read(dict_t const& dict, type& value, bool /*log_on_absence*/, type_traits::if_leaf_type<type>* = 0) // leaf type
{
    if (dict.bin_format())
        bin_translator::read(dict.data(), value);
    else 
        str_translator::read(dict.data(), value);
}


//////////////////////////////////////////////////////////////////////////
// enum

template<class type>
void write(
    dict_t&         dict,
    type const&     value,
    type_traits::if_enum<type>* = 0)
{
    bool bin_fmt = dict.bin_format();

    if(bin_fmt)
        bin_translator::write(dict.data(), (uint32_t) value);
    else
        str_translator::write(dict.data(), value, cpp_utils::enum_to_string(value));

    dict.type_kind() = details::get_type_kind(value);
    dict.reset_format(bin_fmt);
}

template<class type>
void read(dict_t const& dict, type& value, bool /*log_on_absence*/, type_traits::if_enum<type>* = 0) 
{

    if(dict.bin_format())
    {
        uint32_t bin_value;
        str_translator::read(dict.data(), bin_value);
        value = type(bin_value);
    }
    else
    {
        string str_value;
        bin_translator::read(dict.data(), str_value);
        value = *cpp_utils::string_to_enum<type>(str_value);
    }
}

//////////////////////////////////////////////////////////////////////////
// general read/write by key 

template<class type>
void write(dict_t& dict, type const& object, const char* key)
{
    write(dict.add_child(key), object);
}

template<class type>
void read(dict_t const& dict, type& object, const char*key, bool log_on_absence = true)
{
    if (auto child = dict.find(key))
        read(*child, object, log_on_absence);
    else 
    {
        if (log_on_absence)
            LogWarn("dict loading: \"" << key << "\" not found");
        return;
    }
}

//////////////////////////////////////////////////////////////////////////
// stl containers (you could add your own, if needed)

template<class container>
void write( 
    dict_t&             dict, 
    container const&    cont, 

    type_traits::if_container<container>* = 0)
{
    dict.type_kind() = details::get_type_kind(cont);
    for (auto it = cont.begin(); it != cont.end(); ++it)
        write(dict, *it, "");
}

template<class container>
void read(  
    dict_t const&   dict, 
    container&      cont, 
    bool            log_on_absence,

    type_traits::if_container<container>* = 0)
{
    cont.clear();
    for (auto it = dict.children().begin(); it != dict.children().end(); ++it)
    {
        typename container::value_type value;
        read(it->second, value, log_on_absence);

        cont.insert(cont.end(), std::move(value));
    }
}

// std::array<type, size>
template<class type, size_t size>
void read(dict_t const& dict, std::array<type, size>& arr, bool log_on_absence = true)
{
    Assert(dict.children().size() == size);

    size_t i = 0;
    for (auto it = dict.children().begin(); it != dict.children().end(); ++it)
        read(it->second, arr[i++], log_on_absence);
}

// std::map<simple_key, value, pred, alloc>
template <class K, class V, class P, class A>
void read(dict_t const& dict, std::map<K, V, P, A>& map, bool log_on_absence = true,
    type_traits::if_simple_type<K>* = nullptr)
{
    map.clear();
    typedef typename std::map<K, V, P, A>::value_type value_type;

    for (auto it = dict.children().begin(); it != dict.children().end(); ++it)
    {
        value_type value;

        if (!it->second.find("first") && !it->second.find("second"))
        {
            //read map item as simple key->value json object
            std::istringstream ss(it->first.c_str());
            ss >> const_cast<K&>(value.first);
            read(it->second, value.second, log_on_absence);
        }
        else
        {
            //fallback to generic map read
            read(it->second, value, log_on_absence);
        }

        map.insert(std::move(value));
    }
}

//////////////////////////////////////////////////////////////////////////
// std :: vector 

namespace detail
{
    template<class T, class A>
    void write_generic_vector(dict_t& dict, vector<T, A> const& vec)
    {
        for (auto it = vec.begin(); it != vec.end(); ++it)
            write(dict, static_cast<T const&>(*it), "");
    }

    template<class T, class A>
    void read_generic_vector(
        dict_t const& dict, 
        vector<T, A>& vec, 
        bool log_on_absence = true)
    {
        vec.clear();
        vec.resize(dict.children().size());

        auto read_it = vec.begin();
        for (auto it = dict.children().begin(); it != dict.children().end(); ++it, ++read_it)
            read(it->second, *read_it, log_on_absence);        
    }

} // namespace detail


template<class T, class A>
void write(
    dict_t& dict, 
    vector<T, A> const& vec,
    std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr)
{
    // acceleration 
    dict.type_kind() = dict_t::TK_ARRAY;

    if (dict.bin_format())
    {
        write(dict.data(), vec);
        return;
    }

    detail::write_generic_vector(dict, vec);
}

template<class T, class A>
void write(
    dict_t& dict, 
    vector<T, A> const& vec,
    std::enable_if_t<!std::is_arithmetic_v<T>>* = nullptr)
{
    // acceleration 
    dict.type_kind() = dict_t::TK_ARRAY;

    detail::write_generic_vector(dict, vec);
}



template<class T, class A>
void read(
    dict_t const& dict, 
    vector<T, A>& vec, 
    bool log_on_absence = true,
    std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr)
{
    // acceleration 
    if (dict.bin_format())
    {
        binary::input_stream is(dict.data().data(), dict.data().size());
        read(is, vec);
        return;
    }

    detail::read_generic_vector(dict, vec, log_on_absence);
}

template<class T, class A>
void read(
    dict_t const& dict, 
    vector<T, A>& vec, 
    bool log_on_absence = true,
    std::enable_if_t<!std::is_arithmetic_v<T>>* = nullptr)
{
    detail::read_generic_vector(dict, vec, log_on_absence);
}



//////////////////////////////////////////////////////////////////////////
// optional<T>

template<class T>
void write(dict_t& dict, boost::optional<T> const& opt)
{
    dict.type_kind() = details::get_type_kind(opt);
    write(dict, opt.is_initialized(), "initialized");
    if (opt)
        write(dict, *opt, "value");
}

template<class T>
void read(dict_t const& dict, boost::optional<T>& opt, bool log_on_absence = true)
{

    if (dict.data().size() != 0)
    {
        opt.reset(T());
        read(dict, *opt, log_on_absence);
    } 
    else 
    {
        bool initialized = false;
        read(dict, initialized, "initialized", log_on_absence);

        if(!initialized)
            opt.reset();
        else 
        {
            opt.reset(T());
            read(dict, *opt, "value", log_on_absence);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// shared_ptr<T>

template<typename T>
void write(dict_t& dict, std::shared_ptr<T> const& ptr, 
    std::enable_if_t<
        !std::is_base_of_v<binary::serializable, T>
     && !std::is_base_of_v<dict::serializable, T>
    >* = nullptr)
{
    write(dict, bool(ptr), "initialized");
    if (ptr)
        write(dict, *ptr, "value");
}

template<typename T>
void read(dict_t const& dict, std::shared_ptr<T>& ptr, bool log_on_absence, 
    std::enable_if_t<
        !std::is_base_of_v<binary::serializable, T>
     && !std::is_base_of_v<dict::serializable, T>
    >* = nullptr)
{
    bool initialized = false;
    read(dict, initialized, "initialized", log_on_absence);

    if (initialized)
    {
        ptr = std::make_shared<T>();
        read(dict, *ptr, "value", log_on_absence);
    }
    else 
        ptr.reset();
}

template<typename T>
void write(dict_t& dict, std::shared_ptr<T> const& ptr, std::enable_if_t<std::is_base_of_v<dict::serializable, T>>* = nullptr)
{
    if (!ptr)
        return;

    ptr->write_to_dict(dict);
}

template<typename T>
void read(dict_t const& dict, std::shared_ptr<T>& ptr, bool log_on_absence, std::enable_if_t<std::is_base_of_v<dict::serializable, T>>* = nullptr)
{
    (void)log_on_absence;

    create_from_dict(dict, ptr);
}



template<typename T>
void write(dict_t& dict, std::shared_ptr<T> const& ptr, ENABLE_IF_BEGIN<std::is_base_of<binary::serializable, T>::value>ENABLE_IF_END)
{
    bin_translator::write(dict.data(), ptr);
}

template<typename T>
void read(dict_t const& dict, std::shared_ptr<T>& ptr, bool log_on_absence, ENABLE_IF_BEGIN<std::is_base_of<binary::serializable, T>::value>ENABLE_IF_END)
{
    (void)log_on_absence;
    bin_translator::read(dict.data(), ptr);
}


//////////////////////////////////////////////////////////////////////////
// dict_t itself 
inline void write(dict_t& target, dict_t const& source)
{
    target = source;
}

inline void read(dict_t const& source, dict_t& target, bool /*log_on_absence*/ = true)
{
    target = source;
}

//////////////////////////////////////////////////////////////////////////
// special function for reading binary data 

// type_traits::bytes_reader_impl
template<class T>
void write(dict_t& dict, refl::bin_adaptor_impl<T> const& ba)
{
    dict.reset_format(true);
    write(dict.data(), ba.ref());
}

// const because of g++
template<class T>
void read(dict_t const& dict, refl::bin_adaptor_impl<T>& ba, bool log_on_absence = true)
{
    Assert(dict.bin_format());

    if (dict.data().empty())
    {
        if (log_on_absence)
            LogWarn( "bin_adaptor not found");
        return;
    }

    binary::input_stream is(dict.data().data(), dict.data().size());
    read(is, ba.ref());
}

//////////////////////////////////////////////////////////////////////////
// utility functions

template<class type>
type read_dict(dict_t const& dict, const char* key, type def = type(), bool log_on_absence = true)
{
    read(dict, def, key, log_on_absence);
    return def;
}

inline bool is_leaf(dict_t const& dict)
{
    return dict.children().empty();
}

//////////////////////////////////////////////////////////////////////////
// short way for streams operations 
template<class type>
dict_t wrap(type const& object, bool bin_format = true)
{
    dict_t dict(bin_format);
    write(dict, object);

    return dict;
}

//////////////////////////////////////////////////////////////////////////
// file & stream operations

/////////
// binary

//- dict
inline void save_binary(fs::path file, dict_t const& dict)
{
    ofstream ofs(file.string(), std::ios_base::binary);
    
    binary::output_stream os;
    binary::write(os, dict);

    ofs.write(os.data(), os.size());
    Verify(ofs.good());
}

inline void load_binary(fs::path file, dict_t& dict)
{
    ifstream ifs(file.string(), std::ios_base::binary);
    binary::bytes_t data;
    data.resize(size_t(fs::file_size(file)));

    ifs.read(binary::raw_ptr(data), data.size());
    Verify(ifs.good());

    binary::input_stream is(data);
    binary::read(is, dict);
}

//- any type
template<class type>
void save_binary(fs::path file, type const& object)
{
    dict_t dict;
    write(dict, object);
    
    save_binary(file, dict);
}

template<class type>
void load_binary(fs::path file, type& object, bool log_on_absence = true)
{
    dict_t dict;
    load_binary(file, dict);

    read(dict, object, log_on_absence);
}

///////
// info

//inline void load_info(fs::path filename, dict_t& dict)
//{
//    try
//    {
//        info_io::read(dict, filename);
//    }
//    catch (verify_error const&)
//    {
//        VerifyMsg(false, "Can\'t load dict from file " << filename.string());
//    }
//}
//
//inline void save_info(fs::path filename, dict_t const& dict)
//{
//    info_io::write(dict, filename);
//}


} // dict

