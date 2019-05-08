/*
    * Copyright (C) 2015 SimLabs LLC - All rights reserved.
    * Unauthorized copying of this file or its part is strictly prohibited.
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace script
{

//////////////////////////////////////////////////////////////////////////
// optional<T>

template<class T>
void write(py::object& obj, boost::optional<T> const& opt)
{
    if (opt)
        write(obj, *opt);
}

template<class T>
void read(py::object const& obj, boost::optional<T>& opt, bool log_on_absence = true)
{
    if (obj)
    {
        opt.reset(T());
        read(obj, *opt, log_on_absence);
    }
    else
        opt.reset();
}

inline void unwrap_py_dict(dict::dict_t& dict, py::dict const& pydict)
{
    py::list keys = pydict.keys();
    for (int i = 0; i < len(keys); ++i)
    {
        string key = py::extract<string>(keys[i]);
        auto value = pydict[keys[i]];

        auto& child = dict.add_child(key.c_str());

        py::extract<string> get_string(value);
        if (get_string.check())
        {
            string s_child = get_string;
            write(child, s_child);
            continue;
        }
        py::extract<double> get_double(value);
        if (get_double.check())
        {
            double s_child = get_double;
            write(child, s_child);
            continue;
        }
        py::extract<bool> get_bool(value);
        if (get_bool.check())
        {
            bool s_child = get_bool;
            write(child, s_child);
            continue;
        }

        py::extract<py::dict> get_dict(value);
        if (get_dict.check())
        {
            unwrap_py_dict(child, py::extract<py::dict>(value));
        }
    }
}


}