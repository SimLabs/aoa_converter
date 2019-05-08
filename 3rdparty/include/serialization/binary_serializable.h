#pragma once

#include "serialization/io_streams_fwd.h"

namespace binary
{

struct serializable
{
    virtual ~serializable() {};

    virtual void write_to_stream(output_stream &os) const = 0;

    static const uint32_t begin_magic = ('I' << 0) | ('B' << 8) | ('E' << 16) | ('G' << 24);
    static const uint32_t end_magic   = ('I' << 0) | ('E' << 8) | ('N' << 16) | ('D' << 24);
};

} // namespace binary