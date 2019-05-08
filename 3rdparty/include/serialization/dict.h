/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 

#include "reflection/reflection.h"
#include "binary/bytes_array.h"

#include "cpp_utils/fast_list.h"
#include "cpp_utils/fast_string.h"

namespace dict
{

struct dict_t
{
    typedef binary::bytes_array                         data_t;
    typedef cpp_utils::fast_string                      key_t ;

    typedef std::pair<key_t, dict_t>                    child_t;
    typedef cpp_utils::fast_appending_list<child_t>     children_t ;
    typedef optional<children_t>                        children_opt;

    enum type_kind_t
    {
        TK_STRING,
        TK_BOOLEAN,
        TK_INTEGRAL,
        TK_FLOATING_POINT,
        TK_ARRAY,
        TK_OBJECT,
        TK_NULL,

        TK_SIMPLE_MAP, //special for json-like maps

        TK_SIZE
    };

public:
    explicit dict_t(bool bin_format = true);
    
    dict_t(dict_t const& other);
    dict_t(dict_t&& other);
    
    dict_t& operator=(dict_t const&other);
    dict_t& operator=(dict_t&& other);

    void swap(dict_t& other);

    data_t&         data();
    data_t const&   data() const;

    type_kind_t&         type_kind();
    type_kind_t const&   type_kind() const;

    children_t const&   children() const;

    const dict_t*   find     (const char* str) const;                             // returns nullptr if not found
    dict_t&         add_child(const char* str, optional<bool> bin_format = none);  // <none> leads to parent format inheritance 

    void    clear();
    bool    empty() const;

    bool    bin_format  () const;
    void    reset_format(bool bin_format);

    void    clear_cache();

private:
    typedef children_t::const_iterator              dict_cit;
    typedef unordered_map<key_t, dict_cit>          cache_t;
    typedef std::shared_ptr<cache_t>                cache_ptr; 

private:
    data_t          data_;
    type_kind_t     type_kind_;
    children_t      children_;
    bool            bin_format_;

private:
    mutable optional<dict_cit>  last_req_child_; 
    mutable cache_ptr           cache_;

    REFL_INNER(dict_t)
        REFL_ENTRY(data_)
        REFL_ENTRY(bin_format_)
        REFL_ENTRY(children_)
    REFL_END()
};
                                       
///// IMPLEMENTATION ///////////////////////////////////////////////////////////

inline dict_t::dict_t(bool bin_format)
    : type_kind_    (TK_OBJECT)
    , bin_format_   (bin_format)
{}

inline dict_t::dict_t(dict_t const& other)
    : data_             (other.data_)
    , type_kind_        (other.type_kind_)
    , bin_format_       (other.bin_format_)
    , children_         (other.children_)
    , last_req_child_   (other.last_req_child_)
    , cache_            (other.cache_)
{
}

inline dict_t::dict_t(dict_t&& other)
    : data_             (std::move(other.data_    ))
    , type_kind_        (other.type_kind_)
    , bin_format_       (other.bin_format_         )
    , children_         (std::move(other.children_))
    , cache_            (std::move(other.cache_   ))
{
    std::swap(last_req_child_, other.last_req_child_);
}

inline dict_t& dict_t::operator=(dict_t const& other)
{
    dict_t copy(other);
    swap(copy);
    return *this;
}

inline dict_t& dict_t::operator=(dict_t&& other)
{                
    swap(other);
    return *this;
}

inline void dict_t::swap(dict_t& other)
{
    using std::swap;

    swap(data_          , other.data_       );
    swap(type_kind_     , other.type_kind_  );
    swap(bin_format_    , other.bin_format_ );
    swap(children_      , other.children_   );
    swap(cache_         , other.cache_      );
    swap(last_req_child_, other.last_req_child_);
}

inline dict_t::data_t& dict_t::data() 
{
    return data_;
} 

inline dict_t::data_t const& dict_t::data() const
{
    return data_;
}

inline dict_t::type_kind_t& dict_t::type_kind()
{
    return type_kind_;
}

inline dict_t::type_kind_t const& dict_t::type_kind() const
{
    return type_kind_;
}

inline dict_t::children_t const& dict_t::children() const
{
    return children_;
}

inline const dict_t* dict_t::find(const char* str) const 
{   
    const size_t min_cache_size = 20;
    if (children_.empty())
        return nullptr;

    if (children_.size() < min_cache_size)
    {
        for (auto it = children_.begin(); it != children_.end(); ++it)
            if (it->first == key_t(str))
                return &(it->second);

        return nullptr;
    }

    // try from last child request 
    if (last_req_child_)
    {
        ++(*last_req_child_);
    
        if (*last_req_child_ == children_.end())
            last_req_child_.reset();
    }
    else if (!cache_) 
        last_req_child_ = children_.begin();

    if (last_req_child_ && last_req_child_.get()->first == str)
        return &(last_req_child_.get()->second);

    if (!cache_)
        cache_ = std::make_shared<cache_t>();
        
    if (cache_->empty())
    {
        for (auto it = children_.begin(); it != children_.end(); ++it)
        {
            auto res = cache_->insert(make_pair(it->first, it));
            // Assert(res.second); // to prevent from non-unique keys
        }
    }

    auto it = cache_->find(key_t(str));
    if (it != cache_->end())
    {
        //return &(it->second->second);
        last_req_child_ = it->second;
        return &(last_req_child_.get()->second);
    }
    else 
        last_req_child_.reset();


    return nullptr;
}

inline dict_t& dict_t::add_child(const char* str, optional<bool> bin_format)
{
    clear_cache();

    bool bin = bin_format 
        ? bin_format.get() 
        : bin_format_;

    children_.push_back(move(make_pair(move(key_t(str)), move(dict_t(bin)))));
    return children_.back().second;
}

inline void dict_t::clear()
{
    data_    .clear();
    children_.clear();
    
    clear_cache();
}

inline bool dict_t::empty() const
{
    return 
        data_    .empty() && 
        children_.empty();
}

inline bool dict_t::bin_format() const
{
    return bin_format_;
}

inline void dict_t::reset_format(bool bin_format)
{                                               
    bin_format_ = bin_format;
}

inline void dict_t::clear_cache()
{
    cache_         .reset();
    last_req_child_.reset();
}

//////////////////////////////////////////////////////////////////////////

} //dict

using dict::dict_t;

typedef dict_t&                         dict_ref;
typedef dict_t const&                   dict_cref;
typedef dict_t&&                        dict_rref;

typedef boost::optional<dict_t&>        dict_opt;
typedef boost::optional<dict_t const&>  dict_copt;
