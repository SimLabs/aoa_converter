/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "binary/binary.h"
#include <boost/filesystem.hpp>

namespace binary
{

// file functions 
inline void save_to(fs::path file, bytes_cref bytes)
{
    ofstream ofs(file.string(), std::ios_base::binary);
    ofs.write(raw_ptr(bytes), size(bytes));
}

inline bytes_t load_from(fs::path file)
{
    bytes_t bytes;
    bytes.resize(size_t(fs::file_size(file)));

    ifstream ifs(file.string(), std::ios_base::binary);

    ifs.read(raw_ptr(bytes), size(bytes));
    return bytes;
}


} // binary