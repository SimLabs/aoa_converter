#pragma once

struct tree
{
    using data_t = std::string;
    using key_t = std::string;

    data_t& data()
    {
        return data_;
    }

    data_t const& data() const
    {
        return data_;
    }

    size_t count(key_t const& key)
    {
        return children_.count(key);
    }

    tree const* find(key_t const& key) const
    {
        auto it = children_.find(key);
        return it != children_.end() ? &it->second : nullptr;
    }

    decltype(auto) find_all(key_t const& key) const
    {
        return children_.equal_range(key);
    }

    tree& add_child(key_t const& key)
    {
        return children_.emplace(key, tree{})->second;
    }

    bool is_leaf() const
    {
        return children_.empty();
    }

private:
    data_t data_;
    std::multimap<key_t, tree> children_;
};
