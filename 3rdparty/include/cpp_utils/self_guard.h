#pragma once

struct self_guard_dummy_t{};

struct weak_self_guard_t
{
    explicit weak_self_guard_t(weak_ptr<self_guard_dummy_t> wptr)
        : wptr_(wptr)
    {}

    bool expired() const
    {
        return wptr_.expired();
    }
    
private:
    weak_ptr<self_guard_dummy_t> wptr_;
};


struct self_guard_t
{
    self_guard_t()
        : ptr_(make_shared<self_guard_dummy_t>())
    {}

    self_guard_t(self_guard_t const &/*rhs*/)
        : ptr_(make_shared<self_guard_dummy_t>())
    {}

    self_guard_t(self_guard_t &&/*rhs*/)
        : ptr_(make_shared<self_guard_dummy_t>())
    {}

    self_guard_t &operator=(self_guard_t const &/*rhs*/)
    {
        return *this;
    }

    self_guard_t &operator=(self_guard_t &&/*rhs*/)
    {
        return *this;
    }

    weak_self_guard_t get_weak() const
    {
        return weak_self_guard_t(ptr_);
    }
    
private:
    shared_ptr<self_guard_dummy_t> ptr_;
};