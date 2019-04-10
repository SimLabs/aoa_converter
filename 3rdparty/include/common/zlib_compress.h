/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <deque>
#include <zlib/zlib.h>

/// Compress input buffer into a deque using zlib. Default compression level is Z_BEST_SPEED (1)
int compress_memory(char *in_data, size_t in_data_size, std::deque<char>& out_data,
                    int compression_level = Z_BEST_SPEED)
{
    int result;
    std::deque<char> buf_deque;

    const size_t BUFSIZE = 128 * 1024;
    char temp_buffer[BUFSIZE];

    z_stream zstrm;
    zstrm.zalloc = Z_NULL;
    zstrm.zfree = Z_NULL;
    zstrm.next_in = (Bytef*)in_data;
    zstrm.avail_in = uInt(in_data_size);
    zstrm.next_out = (Bytef*)temp_buffer;
    zstrm.avail_out = (uInt)BUFSIZE;

    result = deflateInit(&zstrm, compression_level);
    if (result != Z_OK)
        return result;
    // read till end of input
    while (zstrm.avail_in != 0)
    {
        result = deflate(&zstrm, Z_NO_FLUSH);
        if (result != Z_OK)
        {
            deflateEnd(&zstrm);
            return result;
        }
        if (zstrm.avail_out == 0)
        {
            buf_deque.insert(buf_deque.end(), temp_buffer, temp_buffer + BUFSIZE);
            zstrm.next_out = (Bytef*)temp_buffer;
            zstrm.avail_out = BUFSIZE;
        }
    }
    // read the rest
    result = Z_OK;
    while (result == Z_OK)
    {
        if (zstrm.avail_out == 0)
        {
            buf_deque.insert(buf_deque.end(), temp_buffer, temp_buffer + BUFSIZE);
            zstrm.next_out = (Bytef*)temp_buffer;
            zstrm.avail_out = BUFSIZE;
        }
        result = deflate(&zstrm, Z_FINISH);
    }

    buf_deque.insert(buf_deque.end(), temp_buffer, temp_buffer + BUFSIZE - zstrm.avail_out);
    deflateEnd(&zstrm);
    out_data.swap(buf_deque);
    return result == Z_STREAM_END ? Z_OK : result;
}

/// Uncompress input buffer into a deque using zlib.
int uncompress_memory(char *in_data, size_t in_data_size, std::deque<char>& out_data)
{
    int result;
    std::deque<char> buf_deque;

    const size_t BUFSIZE = 128 * 1024;
    char temp_buffer[BUFSIZE];

    z_stream zstrm;
    zstrm.zalloc = Z_NULL;
    zstrm.zfree = Z_NULL;
    zstrm.next_in = (Bytef*)in_data;
    zstrm.avail_in = uInt(in_data_size);
    zstrm.next_out = (Bytef*)temp_buffer;
    zstrm.avail_out = (uInt)BUFSIZE;

    result = inflateInit(&zstrm);
    if (result != Z_OK)
        return result;
    // read till end of input
    while (zstrm.avail_in != 0)
    {
        result = inflate(&zstrm, Z_NO_FLUSH);
        switch (result) {
        case Z_NEED_DICT:
            result = Z_DATA_ERROR; // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            inflateEnd(&zstrm);
            return result;
        }
        if (zstrm.avail_out == 0)
        {
            buf_deque.insert(buf_deque.end(), temp_buffer, temp_buffer + BUFSIZE);
            zstrm.next_out = (Bytef*)temp_buffer;
            zstrm.avail_out = BUFSIZE;
        }
    }
    // read the rest
    result = Z_OK;
    while (result == Z_OK)
    {
        if (zstrm.avail_out == 0)
        {
            buf_deque.insert(buf_deque.end(), temp_buffer, temp_buffer + BUFSIZE);
            zstrm.next_out = (Bytef*)temp_buffer;
            zstrm.avail_out = BUFSIZE;
        }
        result = inflate(&zstrm, Z_FINISH);
    }

    buf_deque.insert(buf_deque.end(), temp_buffer, temp_buffer + BUFSIZE - zstrm.avail_out);
    inflateEnd(&zstrm);
    out_data.swap(buf_deque);
    return result == Z_STREAM_END ? Z_OK : result;
}

int uncompress_to_stream(char *in_data, size_t in_data_size, std::ostream & stream)
{
    int result;

    const size_t BUFSIZE = 128 * 1024;
    char temp_buffer[BUFSIZE];

    z_stream zstrm;
    zstrm.zalloc = Z_NULL;
    zstrm.zfree = Z_NULL;
    zstrm.next_in = (Bytef*)in_data;
    zstrm.avail_in = uInt(in_data_size);
    zstrm.next_out = (Bytef*)temp_buffer;
    zstrm.avail_out = (uInt)BUFSIZE;

    result = inflateInit(&zstrm);
    if (result != Z_OK)
        return result;
    // read till end of input
    while (zstrm.avail_in != 0)
    {
        result = inflate(&zstrm, Z_NO_FLUSH);
        switch (result) {
        case Z_NEED_DICT:
            result = Z_DATA_ERROR; // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            inflateEnd(&zstrm);
            return result;
        }
        if (zstrm.avail_out == 0)
        {
            stream.write(temp_buffer, BUFSIZE);
            zstrm.next_out = (Bytef*)temp_buffer;
            zstrm.avail_out = BUFSIZE;
        }
    }
    // read the rest
    result = Z_OK;
    while (result == Z_OK)
    {
        if (zstrm.avail_out == 0)
        {
            stream.write(temp_buffer, BUFSIZE);
            zstrm.next_out = (Bytef*)temp_buffer;
            zstrm.avail_out = BUFSIZE;
        }
        result = inflate(&zstrm, Z_FINISH);
    }

    stream.write(temp_buffer, BUFSIZE - zstrm.avail_out);
    inflateEnd(&zstrm);
    return result == Z_STREAM_END ? Z_OK : result;
}
