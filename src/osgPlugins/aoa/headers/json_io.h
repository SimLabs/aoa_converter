/*
    * Copyright (C) 2015 SimLabs LLC - All rights reserved.
    * Unauthorized copying of this file or its part is strictly prohibited.
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "reflection/proc/dict_refl.h"
#include "serialization/dict_ops.h"

namespace json_io
{

dict_t stream_to_dict(std::istream &json_stream, bool camel_convert = false);
void dict_to_stream(std::ostream &stream, dict_t const &d, bool pretty, bool camel_convert = false);

template<typename T>
void stream_to_data(std::istream &stream, T &data, bool camel_convert = false)
{
    dict_t d = stream_to_dict(stream, camel_convert);
    {
        //time_counter tc;
        dict::read(d, data, false);
    }
}

template<typename T>
void data_to_stream(std::ostream &stream, T const &data, bool pretty, bool camel_convert = false)
{
    dict_t d(false);
    dict::write(d, data);
    dict_to_stream(stream, d, pretty, camel_convert);
}

template<typename T>
void string_to_data(std::string const &json, T &data, bool camel_convert = false)
{
    std::istringstream ss(json);
    stream_to_data(ss, data, camel_convert);
}

template<typename T>
string data_to_string(T const &data, bool pretty, bool camel_convert = false)
{
    std::ostringstream ss;
    data_to_stream(ss, data, pretty, camel_convert);
    return ss.str();
}


} // namespace json_io