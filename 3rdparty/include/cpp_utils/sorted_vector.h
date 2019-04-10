/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once


namespace cpp_utils
{


// sorted_vector adapter
// it was found that lower_bound on sorted vector was 2x-3x times faster than set
// there are several places where it is crucial (allocators, etc...)
// and where vector reorganization occurs not so often, so we can use it
template <class type, class comparator = std::less<type>>
struct sorted_vector
{
    sorted_vector( const comparator & c = comparator() ) : vector_(), cmp_(c) {}

    template <class in_it> sorted_vector( in_it first, in_it last, const comparator & c = comparator() )
        : vector_(first, last)
        , cmp_(c)
    {
        sort(begin(), end(), cmp_);
    }

    typedef typename vector<type>::iterator       iterator;
    typedef typename vector<type>::const_iterator const_iterator;

    iterator begin() { return vector_.begin(); }
    iterator end() { return vector_.end(); }
    const_iterator begin() const { return vector_.begin(); }
    const_iterator end() const { return vector_.end(); }

public:

    bool empty() const
    {
        return vector_.empty();
    }

    const_iterator lower_bound( const type & t ) const
    {
        return std::lower_bound(vector_.cbegin(), vector_.cend(), t, cmp_);
    }

    iterator lower_bound( const type & t )
    {
        return std::lower_bound(vector_.begin(), vector_.end(), t, cmp_);
    }

    void insert( const type & t )
    {
        auto it = lower_bound(t);
        if (it == vector_.end() || cmp_(t, *it))
            vector_.insert(it, t);
    }

    const_iterator find( const type & t ) const
    {
        auto it = lower_bound(t);
        return (it == vector_.end() || cmp_(t, *it)) ? vector_.end() : it;
    }

    iterator find( const type & t )
    {
        auto it = lower_bound(t);
        return (it == vector_.cend() || cmp_(t, *it)) ? vector_.end() : it;
    }

    void erase( iterator it )
    {
        vector_.erase(it);
    }

    void erase( const type & t )
    {
        auto it = find(t);
        if (it != vector_.end())
            vector_.erase(it);
    }

    size_t size()
    {
        return vector_.size();
    }

private:

    std::vector<type> vector_;
    comparator cmp_;
};

} // namespace utils
