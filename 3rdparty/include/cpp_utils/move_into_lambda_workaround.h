#pragma once

namespace utils
{

namespace details
{

// type that moves the underlying type when copied
template<class T>
struct rref_copy
{
    rref_copy()                       = delete;
    rref_copy& operator=(rref_copy const& other) = delete;

    rref_copy(T&& v)
        : value_(std::move(v))
        , empty_(false)
    {}

    rref_copy(rref_copy const& other)
        : value_(std::move(other.value_))
        , empty_(false)
    {
        Assert(!other.empty_);
        other.empty_ = true;
    }

    rref_copy(rref_copy&& other)
        : rref_copy(other)
    {}

    T& operator->()
    {
        Assert(!empty_);
        return value_;
    }

    T& operator*()
    {
        Assert(!empty_);
        return value_;
    }

    T const& operator->() const
    {
        Assert(!empty_);
        return value_;
    }

    T const& operator*() const
    {
        Assert(!empty_);
        return value_;
    }
private:
    T    value_;
    mutable bool empty_;
};

} // details

template<class T>
details::rref_copy<T> make_move_workaround(T&& v)
{
    return details::rref_copy<T>(std::move(v));
}

}