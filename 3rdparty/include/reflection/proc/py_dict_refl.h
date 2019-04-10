/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "serialization/py_dict_ops.h"
#include "common/primitives_io.h"

namespace script
{
    namespace type_traits
    {
        template<class T> using is_container = binary::type_traits::is_container<T>;
        template<class T> using is_leaf_type = dict::type_traits::is_leaf_type<T>;
        template<class T> using if_leaf_type = dict::type_traits::if_leaf_type<T>;

        template<class T>
        using if_not_leaf_n_container = typename std::enable_if<!is_container<T>::value && !is_leaf_type<T>::value>::type;
    
    } // type_traits

    namespace details
    {
        // processors 
        struct write_processor
        {
            explicit write_processor(py::dict& dict)
                : bd_(dict)
            {
            }

            template<class type>
            void operator()(type const& object, char const* key, ...) // just forwarding to outer functions for reflection
            {
                using namespace dict;
                write(bd_, object, key);
            }

        private:
            py::dict& bd_;
        };

        //////////////////////////////////////////////////////////////////////////

        struct read_processor
        {
            explicit read_processor(py::dict const& dict, bool log_on_absence = true)
                : bd_(dict)
                , log_on_absence_(log_on_absence)
            {
            }

            template<class type>
            void operator()(type& object, char const* key, ...) // just forwarding to outer functions for reflection
            {
                using namespace dict;
                read(bd_, object, key, log_on_absence_);
            }

        private:
            py::dict const&   bd_;
            bool            log_on_absence_;
        };

        template<class Collection>
        void write_collection(py::dict& dict, Collection const& object, const char* key)
        {
            py::list dst_list;
        
            for (auto const &src_elem : object)
            {
                py::object dst_elem;
                write(dst_elem, src_elem);
                dst_list.append(dst_elem);
            }

            dict[key] = dst_list;
        }

    } // details 

    template<class type>
    void write(py::dict& dict, type const& object, const char* key)
    {
        py::object obj;
        write(obj, object);
        dict[key] = obj;
    }

    template<class type>
    void write(py::dict& dict, optional<type> const& object, const char* key)
    {
        py::object obj;
        
        if (object)
            write(obj, *object);

        dict[key] = obj;
    }
    
    template<class type>
    void write(py::dict& dict, vector<type> const& object, const char* key)
    {
        return details::write_collection(dict, object, key);
    }

    template<class type, size_t Size>
    void write(py::dict& dict, array<type, Size> const& object, const char* key)
    {
        return details::write_collection(dict, object, key);
    }

    template<class type>
    void read(py::dict const& dict, type& object, const char* key, bool log_on_absence)
    {
        if (dict.has_key(key))
        {
            py::object obj = dict.get(key);
            if (!obj.is_none())
            {
                read(obj, object, log_on_absence);
            }
            else
            {
                if (log_on_absence)
                    LogWarn("key \"" << key << "\" value is None");
            }
        }
        else
        {
            if (log_on_absence)
                LogWarn("key \"" << key << "\" not found");
            //LogWarn("py::dict loading: \"" << key << "\" not found");
            return;
        }
    }


    template<class type>
    void write_leaf(py::object& pyobj, type const& object, ENABLE_IF_MACRO(!std::is_enum<type>::value))
    {
        pyobj = py::object(object);
    }
    
    template<class type>
    void write_leaf(py::object& pyobj, type const& object, ENABLE_IF_MACRO(std::is_enum<type>::value))
    {
        static_assert(sizeof(object) == 4, "Only 32-bit enums are supported");
        pyobj = py::object(static_cast<uint32_t>(object));
    }

    template<class type>
    void write(py::object& pyobj, type const& object, type_traits::if_leaf_type<type>* = 0)
    {
        write_leaf(pyobj, object);
    }

    template<class type>
    void read_leaf(py::object const& pyobj, type & object, ENABLE_IF_MACRO(std::is_enum<type>::value))
    {
        static_assert(sizeof(object) == 4, "Only 32-bit enums are supported");
        uint32_t uint_value = py::extract<uint32_t>(pyobj);
        object = static_cast<type>(uint_value);
    }

    template<class type>
    void read_leaf(py::object const& pyobj, type & object, ENABLE_IF_MACRO(!std::is_enum<type>::value))
    {
        object = py::extract<type>(pyobj);
    }

    template<class type>
    void read(py::object const& pyobj, type & object, bool /*log_on_absence*/, type_traits::if_leaf_type<type>* = 0)
    {
        read_leaf(pyobj, object);
    }


    template<class type>
    void write(py::object& pyobj, type const& object, type_traits::if_not_leaf_n_container<type>* = 0)
    {
        py::dict dict;
        details::write_processor wp(dict);
        reflect(wp, object);
        pyobj = dict;
    }

    template<class type>
    void read(py::object const& pyobj, type& object, bool log_on_absence = true, type_traits::if_not_leaf_n_container<type>* = 0)
    {
        py::dict dict = py::extract<py::dict>(pyobj);
        details::read_processor rp(dict, log_on_absence);
        reflect(rp, object);
    }

    template<typename T>
    void read(py::object const& src_list, vector<T> &dst_vector, bool log_on_absence)
    {
        size_t len = boost::python::len(src_list);
        dst_vector.resize(len);

        for (size_t i = 0; i < len; ++i)
            read(src_list[i], dst_vector[i], log_on_absence);
    }

    template<class type>
    py::object wrap(type& object)
    {
        py::object o;
        write(o, object);
        return o;
    }

} // script 

namespace refl
{
    template<> struct processor_type < script::details::read_processor  > { static const size_t value = pt_input; };
    template<> struct processor_type < script::details::write_processor > { static const size_t value = pt_output; };
} // refl
