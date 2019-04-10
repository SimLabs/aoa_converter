#pragma once 

namespace refl
{
    template<size_t proc_type>
    struct any_processor;

    template<>
    struct any_processor < refl::pt_input >
    {
        template<class processor, class T, class tag_t>
        static bool process2(processor& proc, any& entry, tag_t const& tag, string const& type_key, string const& key)
        {
            if (type_key != key)
                return false;

            T data;
            proc(data, data, "data", tag);
            entry = data;

            return true;
        }
    };

    template<>
    struct any_processor < refl::pt_output >
    {
        template<class processor, class T, class tag_t>
        static bool process2(processor& proc, any const& entry, tag_t const& tag, string const& type_key, string const&)
        {
            if (auto ptr = any_cast<T>(&entry))
            {
                proc(type_key, type_key, "key", tag);
                proc(*ptr, *ptr, "data", tag);

                return true;
            }

            return false;
        }
    };
}

#define REFL_ENTRY_ANY_TAG(entry, proc_tag)                             \
    {                                                                   \
        any& any_entry  = lobj.entry;                                   \
        auto const& tag = proc_tag;                                     \
                                                                        \
        string key;                                                     \
        if (refl::processor_type<processor>::value == refl::pt_input)   \
        proc(key, key, "key", tag);                                     \
                                                                        \
        bool processed = false;


#define REFL_ENTRY_ANY(entry)                       \
    REFL_ENTRY_ANY_TAG(entry, refl::empty_tag_t())


#define ANY_TYPE(type)                                                          \
        processed =                                                             \
            processed ||                                                        \
            refl::any_processor<refl::processor_type<processor>::value>         \
                ::process2<processor, type>(proc, any_entry, tag, #type, key);

#define ANY_TYPE_NAMED(type, name)                                              \
        processed =                                                             \
            processed ||                                                        \
            refl::any_processor<refl::processor_type<processor>::value>         \
                ::process2<processor, type>(proc, any_entry, tag, name, key);

#define REFL_ENTRY_ANY_END() \
    VerifyMsg(processed, "Unsupported any value"); \
    }
