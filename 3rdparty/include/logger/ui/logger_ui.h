/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>

#include "net_logger.h"

namespace logger
{

struct ui_log_entry
{
    ui_log_entry() {}
    ui_log_entry( net_layer::log_entry const&entry )
        : host  (entry.host == boost::asio::ip::address_v4::loopback() ? "localhost" : entry.host.to_string())
        , app   (entry.app)
        , level (logging::level_name(entry.level))
        , time  (logging::time_string(pt::milliseconds(entry.time)))
        , source(entry.source)
        , file  (entry.file)
        , log   (entry.log)
    {}

    string host;
    string app;
    string level;
    string time;
    string source;
    string file;
    string log;
};

typedef std::vector<ui_log_entry>                      ui_log_entries_t;


void save_log_file(string const& path, ui_log_entries_t const& entries);
bool load_log_file(string const& path, ui_log_entries_t& entries);

///////////////////////////////////////////////////////////////
/// implementation
inline void save_log_file(string const& path, ui_log_entries_t const& entries)
{
    std::ofstream ofstr(path.c_str());
    if (!ofstr.good())
        return;

    for (auto entry = entries.begin(); entry != entries.end(); ++entry)
    {
        std::string line = str(format("%-21.21s %-12.12s %-23.23s %-7.7s %-24.24s %-33.33s  ")
            % entry->host
            % entry->app 
            % entry->time 
            % entry->level
            % entry->source 
            % entry->file);

        ofstr << line;

        std::vector<std::string> tokens;
        boost::algorithm::split(tokens, entry->log, boost::is_any_of("\r\n"), boost::token_compress_on);

        for (auto tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
        {
            if (tok_iter != tokens.begin())
                ofstr << std::string(127, ' ');  // 127 spaces to align multi line log messages to the last column

            ofstr << *tok_iter << std::endl;
        }
    }
}

inline bool load_log_file(const string& path, ui_log_entries_t& entries)
{
    namespace bi = boost::interprocess;
    typedef boost::char_separator<char>                     separator_t;
    typedef boost::tokenizer<boost::char_separator<char> >  tokenizer_t;

    entries.clear();

    if ((size_t)boost::filesystem::file_size(path) == 0)
        return false;

    bi::file_mapping  fmap(path.c_str(), bi::read_only);
    bi::mapped_region mreg(fmap, bi::read_only);

    std::string text((char*)mreg.get_address(), mreg.get_size());

    std::istringstream istream(text);

    std::string line;
    while (std::getline(istream, line))
    {
        if (line.size() < 100)
            continue;

        if (line[0] != ' ')
        {
            ui_log_entry lentry;

            size_t from = 0;
            size_t to   = line.find_first_of(" ");
            if (from == line.npos || to == line.npos)
                continue;
            lentry.host = line.substr(from, to - from);

            from = line.find_first_not_of(" ", to);
            to   = line.find_first_of(" ", from);
            if (from == line.npos || to == line.npos)
                continue;
            lentry.app = line.substr(from, to - from);

            from = line.find_first_not_of(" ", to);
            to   = line.find_first_of(" ", from);
            to   = line.find_first_not_of(" ", to); // skip space between date and time
            to   = line.find_first_of(" ", to);
            if (from == line.npos || to == line.npos)
                continue;
            lentry.time = line.substr(from, to - from);

            from = line.find_first_not_of(" ", to);
            to   = line.find_first_of(" ", from);
            if (from == line.npos || to == line.npos)
                continue;
            lentry.level = line.substr(from, to - from);

            from = line.find_first_not_of(" ", to);
            to   = line.find_first_of("(", from);
            if (from == line.npos || to == line.npos)
                continue;
            lentry.source = line.substr(from, to - from);
            boost::trim(lentry.source);

            from = to + 1;
            to   = line.find_first_of(")", from);
            if (from == line.npos || to == line.npos)
                continue;
            lentry.file = line.substr(from, to - from);

            from = line.find_first_not_of(" ", to + 1);
            if (from == line.npos)
                continue;
            lentry.log = line.substr(from);

            entries.push_back(lentry);
        }
        else if (!entries.empty())
        {
            boost::trim(line);
            entries.back().log.append(line + "\n");
        }
    }
    
    return true;
}

}