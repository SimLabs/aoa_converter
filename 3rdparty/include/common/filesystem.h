/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once


// DEPRECATED
// already implemented as fs::remove_all 
inline void remove_recursively(boost::filesystem::path const& path, bool log_errors = false)
{
    namespace fs = boost::filesystem;

    if (fs::exists(path))
    {
        boost::system::error_code ec;

        std::queue<fs::path>  visit;
        visit.push(path);

        while (!visit.empty())
        {
            fs::path cur_path = std::move(visit.front());
            visit.pop();

            fs::remove_all(cur_path, ec);
            if (ec)
            {
                if (log_errors)
                    LogError("Can't remove " << cur_path.string() << ": " << ec.message());

                if (fs::is_directory(cur_path))
                {
                    typedef fs::directory_iterator dit;
                    for (dit dirs(cur_path); dirs != dit(); ++dirs)
                        visit.push(dirs->path());
                }
            }
        }
    }
}

inline void copy_directory_recursively(boost::filesystem::path const & source, boost::filesystem::path const & destination, bool could_exist = true)
{
    namespace fs = boost::filesystem;
    
    Verify( fs::exists(source) && fs::is_directory(source));
    
    if (!could_exist)
    {
        Verify(!fs::exists(destination));
        Verify(fs::create_directory(destination));
    }

    // Iterate through the source directory
    for (fs::directory_iterator file(source); file != fs::directory_iterator(); ++file)
    {
        fs::path current(file->path());

        if (is_directory(current))
            copy_directory_recursively(current, destination / current.filename());
        else
            fs::copy_file(current, destination / current.filename());
    }
}

//inline bool file_less()