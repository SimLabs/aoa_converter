#pragma once 

namespace text_io
{
    // streams 
    typedef std::istream    input_stream_t;
    typedef std::ostream    output_stream_t;

    namespace details
    {
        bool is_separator(char c);
        bool is_quote    (char c);

        template<class func_f>
        char get_while(std::istream& is, func_f const& func, string* result = nullptr);

        inline string retrieve_string(std::istream& is);
    
    } // details 

    namespace type_traits
    {
        template<class T>
        using if_fundamental = typename std::enable_if<std::is_fundamental<T>::value>::type;
    
    } // type_traits

    //////////////////////////////////////////////////////////////////////////

    // fundamental types 
    template<class type>
    void read(input_stream_t& is, type& object, type_traits::if_fundamental<type>* = 0)
    {
        is >> object;
    }

    template<class type>
    void write(output_stream_t& os, type const& object, const char* trailing_separator = "", type_traits::if_fundamental<type>* = 0)
    { 
        os << object << trailing_separator;
    }

    // string 
    inline void read(input_stream_t& is, string& object)
    {
        //special processing of strings starting with double quotes (")
        object = details::retrieve_string(is);
    }

    inline void write(output_stream_t& os, string const& object, const char* trailing_separator = "")
    {
        os << object << trailing_separator;
    }

    // const char* - only write operation 
    inline void write(output_stream_t& os, const char* object, const char* trailing_separator = "")
    {
        os << object << trailing_separator;
    }
    
    // vector 
    template<class T, class A>
    void read(input_stream_t& is, vector<T, A>& vec)
    {
        size_t sz = 0;
        is >> sz;

        vec.resize(sz);
        for (size_t i = 0; i != sz; ++i)
            read(is, vec[i]);
    }

    template<class T, class A>
    void write(output_stream_t& os, vector<T, A> const& vec, const char* trailing_separator = "")
    {
        write(os, vec.size());
        
        if (vec.empty())
        {
            os << trailing_separator;
            return;
        }

        os << std::endl;
        for (size_t i = 0; i != vec.size(); ++i)
        {
            write(os, vec[i]);
            os << std::endl;
        }
    }

    // std::array 
    template<class T, size_t N>
    void read(input_stream_t& is, std::array<T, N>& arr)
    {
        for (size_t i = 0; i != N; ++i)
            read(is, arr[i]);
    }

    template<class T, size_t N>
    void write(output_stream_t& os, std::array<T, N> const& arr, const char* trailing_separator = "")
    {
        for (size_t i = 0; i != N; ++i)
        {
            write(os, arr[i]);
            
            if (i != N)
                os << " ";
        }

        os << trailing_separator;
    }




    //////////////////////////////////////////////////////////////////////////

    namespace details
    {
        inline bool is_separator(char c)
        {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n';
        }

        inline bool is_quote(char c)
        {
            return c == '\"';
        }

        template<class func_f>
        char get_while(std::istream& is, func_f const& func, string* result)
        {
            char ch = is.get();
            for (; is && func(ch); ch = is.get())
                if (result)
                    *result += ch;

            return ch;
        }

        inline string retrieve_string(std::istream& is)
        {
            //using std::not1;

            char c = is.get();

            if (is_separator(c))
                c = get_while(is, is_separator);

            string res;
            if (is_quote(c))
            {
                get_while(is, [](char ch) { return !is_quote(ch); }, &res);
                get_while(is, [](char ch) { return !is_separator(ch); });
            }
            else
            {
                res += c;
                get_while(is, [](char ch) { return !is_separator(ch); }, &res);
            }

            return res;
        }
    } // details 

} // text_io