/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "reflection.h"
#include <stdarg.h>

// macro summary: 
/*

REFL_STRUCT(type)
REFL_INNER (type)
REFL_END   ()      

REFL_ENTRY    (entry)

REFL_CHAIN (base)

*/

namespace prop_attr
{
    struct read_only_tag;
}

namespace refl
{

namespace details
{
    struct null_end_list_parser
    {
        null_end_list_parser(const char** names)
            : names_(names)
        {}

        inline size_t apply(char const *label0, ...) const
        {
            va_list argptr;

            size_t count = 0;
            va_start(argptr, label0);
            for (char const * label = label0; label != NULL; label = va_arg(argptr, char const*))
                names_[count++] = label;
            va_end(argptr);

            names_[count] = 0;
            return count;
        }

    private:
        const char** names_;
    };

} // details

// TURN OFF attrs
struct just_serialization{};

         
} // refl

#define REFL_SER(entry)                                                             \
    proc(lobj.entry, robj.entry, #entry, refl::just_serialization());

#define REFL_SER_RAW(entry)                                                         \
    proc(entry, entry, #entry, refl::just_serialization());

#define REFL_SER_AS_TYPE(entry, type)                                               \
    refl::process_as_type<type>                                                     \
    (proc, lobj.entry, robj.entry, #entry, refl::just_serialization());

#define REFL_SER_BIN(entry)                                                         \
    {auto ba = refl::bin_adaptor(lobj.entry); proc(ba, ba, #entry, refl::just_serialization());}

#define REFL_ENTRY_RO(entry)                                                        \
    proc(lobj.entry, robj.entry, #entry, prop_attr::read_only_tag());

#define REFL_ENTRY_EDITOR(entry, editor)    \
    REFL_ENTRY_TAG(entry, editor)

#define REFL_RANGE_EDITOR(rng, editor)   \
{                                        \
    REFL_ENTRY_EDITOR(rng.hi_, editor)   \
    REFL_ENTRY_EDITOR(rng.lo_, editor)   \
}

#define REFL_ENTRY_EDITOR_NAMED(entry, name, editor)        \
{                                                           \
    proc(lobj.entry, lobj.entry, name, editor);           \
}
