#pragma once 

#include "serialization/text_io_ops.h"

namespace text_io
{
    namespace type_traits
    {
        template<class T>
        using use_reflection = typename std::enable_if<!std::is_fundamental<T>::value>::type;

    } // type_traits


    // write specifiers 
    enum action
    {
        no      = 0,
        endl    = 1 << 0,
        quotes  = 1 << 1 ,
        decimals= 1 << 2,
    };
    
    // tags 
    struct tag_t
    {
        tag_t(size_t actions = no, uint32_t digits = 0)
            : actions(actions)
            , digits (digits)
        {}
    
        size_t   actions;
        uint32_t digits;
    };

    //////////////////////////////////////////////////////////////////////////
    // reflection processors
    namespace details
    {
        struct write_processor
        {
            explicit write_processor(output_stream_t& os)
                : os(os)
            {
            }

            template<class type>
            void operator()(type const& object, char const *, ...) // just forwarding to outer functions for reflection
            {
                using namespace text_io;
                write(os, object, " ");
            }

            template<class type>
            void operator()(type const& object, char const *, tag_t const& tag...) // just forwarding to outer functions for reflection
            {
                using namespace text_io;

                bool make_quotes    = ((tag.actions & quotes    ) == quotes  );
                bool make_endl      = ((tag.actions & endl      ) == endl    );
                bool make_decimals  = ((tag.actions & decimals  ) == decimals);

                if (make_decimals)
                {
                    os.precision(tag.digits);
                    os.setf(std::ios::fixed, std::ios::floatfield);
                }

                if (make_quotes)
                    os << '\"';

                write(os, object);

                if (make_quotes)
                    os << '\"';

                if (make_endl)
                    os << std::endl;
                else
                    os << " ";
            }


        private:
            output_stream_t& os;
        };

        struct read_processor
        {
            explicit read_processor(input_stream_t& is)
                : is(is)
            {
            }

            template<class type>
            void operator()(type& object, char const *, ...) // just forwarding to outer functions for reflection
            {
                using namespace text_io;
                read(is, object);
            }

        private:
            input_stream_t& is;
        };

    } // details

    // via --reflect--
    template<class type>
    void read(input_stream_t& is, type& object, type_traits::use_reflection<type>* = 0)
    {
        details::read_processor rp(is);
        reflect(rp, object);
    }

    template<class type>
    void write(output_stream_t& os, type const& object, const char* /*trailing_separator*/ = "", type_traits::use_reflection<type>* = 0)
    {
        details::write_processor wp(os);
        reflect(wp, object);
    }

} // text_io

namespace refl
{
    template<> struct processor_type<text_io::details::read_processor > { static const size_t value = pt_input ; };
    template<> struct processor_type<text_io::details::write_processor> { static const size_t value = pt_output; };
} // refl