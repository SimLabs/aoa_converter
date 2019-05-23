/*
    * Copyright (C) 2015 SimLabs LLC - All rights reserved.
    * Unauthorized copying of this file or its part is strictly prohibited.
    * For any information on this file or its part please contact: support@sim-labs.com
*/

//#include "info_io/info_io.h"
#include "json_io.h"
#include "serialization/dict_ops.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include "rapidjson/error/en.h"


namespace json_io
{

namespace
{

pair<const char*, size_t> toCamel(const char* s, size_t size)
{
    static thread_local char buf[256];
    char* result = buf;
    auto end = s + size;
    for(; s < end; ++s)
    {
        if(*s != '_')
        {
            Verify(std::islower(*s));
            *result++ = *s;
        }
        else
            if(s < end - 1)
            {
                Verify(*(s+1) != '_');
                *result++ = std::toupper(*++s);
            }
    }

    *result = '\0';

    return {buf, result-buf};
}

pair<const char*, size_t> fromCamel(const char* s, size_t size)
{
    static thread_local char buf[256];
    char* result = buf;
    auto end = s + size;
    for(; s < end; ++s)
    {
        Verify(*s != '_');
        if(std::isupper(*s))
        {
            *result++ = '_';
            *result++ = std::tolower(*s);
        }
        else
        {
            Verify(std::islower(*s));
            *result++ = *s;
        }
    }

    *result = '\0';

    return { buf, result - buf };
}
    
struct read_dict_handler
{
    typedef char Ch;
    typedef size_t SizeType;
    
    read_dict_handler(bool camel_convert)
        : root_dict_(false)
        , camel_convert_(camel_convert)
    {
    }
    
    bool StartObject()
    {
        push_dict();
        return true;
    }
    
    bool EndObject(SizeType /*memberCount*/)
    {
        pop_dict();
        return true;
    }

    bool StartArray()
    {
        push_dict();
        ds_.top()->type_kind() = dict_t::TK_ARRAY;
        return true;
    }

    bool EndArray(SizeType /*elementCount*/)
    {
        pop_dict();
        return true;
    }

    bool Key(const Ch* str, SizeType length, bool /*copy*/)
    {
        if(camel_convert_)
        {
            auto uncameled = fromCamel(str, length);
            key_.assign(uncameled.first, uncameled.second);
        }
        else
        {
            key_.assign(str, length);
        }
        return true;
    }

    bool String(const Ch* str, SizeType length, bool /*copy*/)
    {
        string s(str, str + length);
        set_data(s, dict_t::TK_STRING);
        return true;
    }

    bool Bool(bool b)
    {
        string str(b ? "true" : "false");
        set_data(str, dict_t::TK_BOOLEAN);
        return true;
    }

    bool Int   (int      v) { set_data(v, dict_t::TK_INTEGRAL); return true; }
    bool Uint  (unsigned v) { set_data(v, dict_t::TK_INTEGRAL); return true; }
    bool Int64 (int64_t  v) { set_data(v, dict_t::TK_INTEGRAL); return true; }
    bool Uint64(uint64_t v) { set_data(v, dict_t::TK_INTEGRAL); return true; }
    bool Double(double   v) { set_data(v, dict_t::TK_FLOATING_POINT); return true; }
        
    bool Null()
    {
        set_null();
        return true;
    }

    dict_t &get_dict()
    {
        return root_dict_;
    }

private:
    void push_dict()
    {
        if (ds_.empty())
        {
            ds_.push(&root_dict_);
        }
        else
        {
            string key = use_key();
            dict_t &child = ds_.top()->add_child(key.c_str());
            ds_.push(&child);
        }
    }

    void pop_dict()
    {
        ds_.pop();
    }

    string use_key()
    {
        string new_key;
        std::swap(new_key, key_);
        return new_key;
    }

    void set_null()
    {
        push_dict();
        // for compatibility with dict_ops
        optional<bool> foo;
        write(*ds_.top(), foo);
        pop_dict();
    }

    void set_data(string const &str, dict_t::type_kind_t kind = dict_t::TK_OBJECT)
    {
        push_dict();
        ds_.top()->data().assign(str.begin(), str.end());
        ds_.top()->type_kind() = kind;
        pop_dict();
    }
    
    template<typename T>
    void set_data(T t, dict_t::type_kind_t kind = dict_t::TK_OBJECT)
    {
        set_data(boost::lexical_cast<string>(t), kind);
    }

private:
    dict_t root_dict_;
    std::stack<dict_t *> ds_;
    string key_;
    bool camel_convert_;
};

bool check_if_array(dict_t const &dict)
{
    dict_t::children_t const &children = dict.children();

    for (auto it = children.begin(); it != children.end(); ++it)
    {
        dict_t::key_t const &key = it->first;
        if (!key.empty())
            return false;
    }

    return true;
}

template<typename Writer>
void write_dict(Writer &writer, dict_t const &dict, bool camel_convert)
{
    auto const &data = dict.data();
    std::istringstream inp(string(data.begin(), data.size()));

    if (dict.type_kind() == dict_t::TK_INTEGRAL)
    {
        int64_t i;
        inp >> i;
        writer.Int64(i);
    }
    else if (dict.type_kind() == dict_t::TK_FLOATING_POINT)
    {
        double d;
        inp >> d;
        writer.Double(d);
    }
    else if (dict.type_kind() == dict_t::TK_BOOLEAN)
    {
        bool b;
        inp >> std::boolalpha >> b;
        writer.Bool(b);
    }
    else if(dict.type_kind() == dict_t::TK_NULL)
    {
        bool initialized = false;
        read(dict, initialized, "initialized");
        if(initialized)
            write_dict(writer, *dict.find("value"), camel_convert);
        else
            writer.Null();
    }
    else if(dict.type_kind() == dict_t::TK_STRING)
    {
        writer.String(data.begin(), rapidjson::SizeType(data.size()));
    } 
    else if(dict.type_kind() == dict_t::TK_SIMPLE_MAP 
         || dict.type_kind() == dict_t::TK_ARRAY
         || dict.type_kind() == dict_t::TK_OBJECT)
    {  
        if (dict.type_kind() == dict_t::TK_ARRAY)
            writer.StartArray();
        else
            writer.StartObject();

        dict_t::children_t const &children = dict.children();
        auto size = children.size();
        (void)size;

        for (auto it = children.begin(); it != children.end(); ++it)
        {
            if (dict.type_kind() == dict_t::TK_SIMPLE_MAP)
            {
                const dict_t* key = it->second.find("first");
                const dict_t* value = it->second.find("second");

                VerifyMsg(key, "Writing map object, but no field named first in element");
                VerifyMsg(value, "Writing map object, but no field named second in element");

                if(camel_convert)
                {
                    auto cameled_key = toCamel(key->data().begin(), rapidjson::SizeType(key->data().size()));
                    writer.Key(cameled_key.first, rapidjson::SizeType(cameled_key.second));
                }
                else
                {
                    writer.Key(key->data().begin(), rapidjson::SizeType(key->data().size()));
                }
                write_dict(writer, *value, camel_convert);
            }
            else
            {
                if (dict.type_kind() == dict_t::TK_OBJECT)
                {
                    dict_t::key_t const &key = it->first;
                    if(camel_convert)
                    {
                        auto cameled_key = toCamel(key.c_str(), rapidjson::SizeType(key.size()));
                        writer.Key(cameled_key.first, rapidjson::SizeType(cameled_key.second));
                    }
                    else
                    {
                        writer.Key(key.c_str(), rapidjson::SizeType(key.size()));
                    }
                }

                dict_t const &child = it->second;
                write_dict(writer, child, camel_convert);
            }
        }
        
        if (dict.type_kind() == dict_t::TK_ARRAY)
            writer.EndArray();
        else
            writer.EndObject();
    } else {
        Verify(false);
    }
}

class IStreamWrapper {
public:
    typedef char Ch;
    IStreamWrapper(std::istream& is) : is_(is) {
    }
    Ch Peek() const { // 1
        int c = is_.peek();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }
    Ch Take() { // 2
        int c = is_.get();
        return c == std::char_traits<char>::eof() ? '\0' : (Ch)c;
    }
    size_t Tell() const { return (size_t)is_.tellg(); } // 3
    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch) { assert(false); }
    void Flush() { assert(false); }
    size_t PutEnd(Ch*) { assert(false); return 0; }
private:
    IStreamWrapper(const IStreamWrapper&);
    IStreamWrapper& operator=(const IStreamWrapper&);
    std::istream& is_;
};

class OStreamWrapper {
public:
    typedef char Ch;
    OStreamWrapper(std::ostream& os) : os_(os) {
    }
    Ch Peek() const { assert(false); return '\0'; }
    Ch Take() { assert(false); return '\0'; }
    size_t Tell() const {  }
    Ch* PutBegin() { assert(false); return 0; }
    void Put(Ch c) { os_.put(c); }                  // 1
    void Flush() { os_.flush(); }                   // 2
    size_t PutEnd(Ch*) { assert(false); return 0; }
private:
    OStreamWrapper(const OStreamWrapper&);
    OStreamWrapper& operator=(const OStreamWrapper&);
    std::ostream& os_;
};

} // namespace

dict_t stream_to_dict(std::istream &json_stream, bool camel_convert)
{
    read_dict_handler handler(camel_convert);
    IStreamWrapper sw(json_stream);

    {
        //time_counter tc;
        rapidjson::Reader reader;
        reader.Parse(sw, handler);
        if(reader.HasParseError())
        {
            std::ostringstream ss;
            ss << "json parse error at offset " << reader.GetErrorOffset() << ": ";
            ss << rapidjson::GetParseError_En(reader.GetParseErrorCode());
            throw parse_error(ss.str());
        }
    }

    return std::move(handler.get_dict());
}

void dict_to_stream(std::ostream &json_stream, dict_t const &dict, bool pretty, bool camel_convert)
{
    OStreamWrapper sw(json_stream);
    
    if (pretty)
    {
        rapidjson::PrettyWriter<OStreamWrapper> writer(sw);
        write_dict(writer, dict, camel_convert);
    }
    else
    {
        rapidjson::Writer<OStreamWrapper> writer(sw);
        write_dict(writer, dict, camel_convert);
    }
}

} // namespace json_io
