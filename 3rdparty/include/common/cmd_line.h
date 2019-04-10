/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

namespace cmd_line 
{

struct arg_map
{
    typedef std::map<std::string, std::string> raw_arg_map_t;

    bool contains(string const& name);

    template<class T> T extract(string const& name);
    template<class T> T extract(string const& name, T const& def);
    
    template<class T> optional<T> extract_opt(string const& name);

    template<class parser_t>
    bool parse(parser_t const& p, int argc, char* argv[]);

private:
    raw_arg_map_t args_; 
};



struct naive_parser 
{
    naive_parser& add_arg(string const& name, bool has_value = false, bool optional = true, string value_dscr = "");
    string        usage  (string const& app_name = "") const;

public:
    // for arg_map usage
    boost::optional<arg_map::raw_arg_map_t> parse(int argc, char* argv[]) const;

private:
    struct arg_dscr
    {
        bool    has_value;
        bool    optional;
        string  description;

        arg_dscr(bool has_value, bool optional, string description)
            : has_value  (has_value)
            , optional   (optional )
            , description(description)
        {
        }

        arg_dscr() : has_value(false), optional(true)
        {}
    };

    std::map<string, arg_dscr> templ_;
};


//////////////////////////////////////////////////////////////////////////
// implementation 
                          
inline bool arg_map::contains(string const& name)
{
    return args_.count(name) != 0;
}

template<class T> 
T arg_map::extract(string const& name)
{
    VerifyMsg(contains(name), "Command line doesn\'t contain arg " << name);
    return boost::lexical_cast<T>(args_[name]);
}

template<class T> 
T arg_map::extract(string const& name, T const& def)
{
    try
    {
        return boost::lexical_cast<T>(args_[name]);
    }
    catch(boost::bad_lexical_cast const&)
    {
    }
    
    return def;
}

template<class T> 
optional<T> arg_map::extract_opt(string const& name)
{   
    if (contains(name))
    {
        try
        {
            return boost::lexical_cast<T>(args_[name]);
        }
        catch (boost::bad_lexical_cast const&)
        {
        }
    }
                                                     
    return none;
}

template<class parser_t>
bool arg_map::parse(parser_t const& p, int argc, char* argv[])
{
    auto am = p.parse(argc, argv);

    if (!am)
        return false;

    args_ = *am;
    return true;
}


//////////////////////////////////////////////////////////////////////////

inline naive_parser& naive_parser::add_arg(string const& name, bool has_value, bool optional, string dscr)
{
    templ_[name] = arg_dscr(has_value, optional, dscr);
    return *this;
}

inline string naive_parser::usage(string const& app_name) const
{
    std::stringstream ss;

    if (!app_name.empty())    
        ss << app_name << " ";
    
    for (auto it = templ_.begin(); it != templ_.end(); ++it)
    {           
        bool opt = it->second.optional;

        if (opt)
            ss << '[';

        ss << "--" << it->first;

        if (it->second.has_value)
            ss << " <" << it->second.description << ">";

        if (opt)
            ss << ']';

        ss << " ";
    }

    return ss.str();
}

inline boost::optional<arg_map::raw_arg_map_t> naive_parser::parse(int argc, char* argv[]) const
{
    arg_map::raw_arg_map_t am;

    size_t obligatory_args = 0;
    for (auto it = templ_.begin(); it != templ_.end(); ++it)
        if (!it->second.optional)
            ++obligatory_args;

    for(int i = 1; i < argc; ++i) // skipping 0, it's an image filename
    {
        if (!boost::starts_with(argv[i], "--"))
        {
            LogError("Invalid parameter format, should be started with \"--\" " << argv[i]);
            return none;
        }

        auto it = templ_.find(argv[i] + 2);

        if (it == templ_.end())
        {
            LogError("unknown parameter " << argv[i]);
            return none;
        }

        if (it->second.has_value && (i + 1 == argc))
        {   
            LogError("parameter " << argv[i] << " needs value, which wasn't provided");
            return none;
        }

        if (!it->second.optional)
        {
            Assert(obligatory_args >= 0);
            --obligatory_args;
        }

        am[it->first] = it->second.has_value ? argv[++i] : "";
    }

    if (obligatory_args != 0)
    {
        string args;

        for (auto it = templ_.begin(); it != templ_.end(); ++it)
            if (!it->second.optional && am.count(it->first) == 0)
                args += it->first + " ";

        LogError("Not all obligatory arguments were mentioned. Missing args: " << args);
        return none;
    }

    return am;
}

} // cmd_line 


