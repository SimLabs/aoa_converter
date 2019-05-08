#pragma once

namespace cpp_utils
{

struct reflect_hash_processor
{
    reflect_hash_processor()
        : result_(0)
    {}

    template<class T>
    void operator()(T &lhs, ...)
    {
        using boost::hash_value;
        boost::hash_combine(result_, hash_value(lhs));
    }

    size_t result() const
    {
        return result_;
    }

private:
    size_t result_;
};

} // namespace cpp_utils

#define ENABLE_REFL_HASH(type)                   \
    friend size_t hash_value(type const &a)      \
    {                                            \
        cpp_utils::reflect_hash_processor proc;  \
        reflect(proc, a);                        \
        return proc.result();                    \
    }