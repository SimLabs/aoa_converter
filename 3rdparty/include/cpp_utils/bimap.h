#pragma once

namespace cpp_utils
{

template<class Left, class Right>
struct bimap
{
    using left_map_t = std::map<Left, Right>;
    using right_map_t = std::map<Right, Left>;
    using value_type = typename left_map_t::value_type;

    bimap(std::initializer_list<value_type> init)
        : left_(init)
    {
        for (auto const &value: init)
            right_.emplace(value.second, value.first);    
    }

    void insert(value_type const &value)
    {
        left_.insert(value);
        right_.emplace(value.second, value.first);
    }

    Right const &to_right(Left const &key) const
    {
        return left_.at(key);
    }

    Left const &to_left(Right const &key) const
    {
        return right_.at(key);
    }

    bool contains_left(Left const &key) const
    {
        return left_.count(key) != 0;
    }

    bool contains_right(Right const &key) const
    {
        return right_.count(key) != 0;
    }

    left_map_t const &left() const
    {
        return left_;
    }

    right_map_t const &right() const
    {
        return right_;
    }
    
private:
    left_map_t left_;
    right_map_t right_;
};

} // namespace cpp_utils