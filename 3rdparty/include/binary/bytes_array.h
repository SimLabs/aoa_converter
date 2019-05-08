/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/
#pragma once 

// Fast dynamic bytes array implementation.
// Much faster than std::vector<char>, aspecially in DEBUG mode 
// cause does no additional check and provides small object optimization. 


namespace binary
{
    struct bytes_array
    {
        typedef char                byte_type;
        typedef byte_type*          pointer;
        typedef byte_type const*    const_pointer;

    public:
        bytes_array();
       ~bytes_array();

        bytes_array(bytes_array const& other);
        bytes_array(bytes_array&& other);

        bytes_array(const void* data, size_t size);

        bytes_array& operator=(bytes_array other);

    public:
        template<class fwd_it>
        void assign(fwd_it begin, fwd_it end,

            // just check for random access iterator and 'byte' type :)
            typename std::enable_if<
                std::is_same<
                    typename std::iterator_traits<fwd_it>::iterator_category, 
                    std::random_access_iterator_tag
                >::value &&
                (sizeof(typename std::iterator_traits<fwd_it>::value_type) == 1)
            >::type* = 0)
        {
            if (begin == end)
                return;

            append(&(*begin), std::distance(begin, end));
        }
        
        template<class fwd_it>
        bytes_array(fwd_it begin, fwd_it end)
        {
            assign(begin, end);
        }

    public:
        void append(void const* data, size_t size);
        void append(bytes_array const& other);

        void resize(size_t size, bool fill_with_zero = false);

        void clear();

    public:
        pointer         data();
        const_pointer   data() const;

        size_t  size () const;
        bool    empty() const;

    public:
        pointer begin();
        pointer end  ();

        const_pointer begin() const;
        const_pointer end  () const;

    public:
        byte_type& operator[](size_t index);
        byte_type  operator[](size_t index) const;

    public:
        bool is_dynamic() const;

    private:
        void    buy_more_space  (size_t min_size);
        size_t  granularity_ceil(size_t current);

    private:
        const static size_t small_obj_opt_size = 32;
        const static size_t blow_factor = 2;

    private:
        char small_buffer_[small_obj_opt_size];

        size_t  capacity_;
        size_t  size_;
        pointer dyn_buffer_;
    };
} // binary


//// IMPLEMENTATION //////////////////////////////////////////////////////

namespace binary
{
    inline bytes_array::bytes_array()
        : capacity_(small_obj_opt_size)
        , size_(0)
        , dyn_buffer_(0)
    {
    }

    inline bytes_array::~bytes_array()
    {
        delete[] dyn_buffer_;
    }

    inline bytes_array::bytes_array(bytes_array const& other)
        : capacity_(other.capacity_)
        , size_(other.size_)
        , dyn_buffer_(0)
    {
        if (other.is_dynamic())
            dyn_buffer_ = new byte_type[capacity_];

        if (!empty())
            memcpy(data(), other.data(), granularity_ceil(size_));
    }

    inline bytes_array::bytes_array(bytes_array&& other)
        : capacity_(other.capacity_)
        , size_(other.size_)
        , dyn_buffer_(other.dyn_buffer_)
    {
        if (!is_dynamic() && !empty())
            memcpy(small_buffer_, other.small_buffer_, granularity_ceil(size_));

        other.capacity_ = 0;
        other.size_ = 0;
        other.dyn_buffer_ = 0;
    }

    inline bytes_array::bytes_array(const void* data, size_t size)
        : bytes_array()
    {
        append(data, size);
    }

    inline bytes_array& bytes_array::operator=(bytes_array other)
    {
        using std::swap;

        swap(capacity_, other.capacity_);
        swap(size_, other.size_);
        swap(dyn_buffer_, other.dyn_buffer_);

        if (!is_dynamic() && !empty())
            memcpy(small_buffer_, other.small_buffer_, granularity_ceil(size_));

        return *this;
    }

    //////////////////////////////////////////////////////////////////////////
    inline void bytes_array::append(void const* in_data, size_t size)
    {
        size_t new_size = size_ + size;

        if (capacity_ < new_size)
            buy_more_space(new_size);

        memcpy(data() + size_, (const_pointer)in_data, size);
        size_ = new_size;
    }


    inline void bytes_array::append(bytes_array const& other)
    {
        append(other.data(), other.size());
    }

    inline void bytes_array::resize(size_t size, bool fill_with_zero)
    {
        if (capacity_ < size)
            buy_more_space(size);

        size_ = size;

        if (fill_with_zero)
            memset(data(), 0, granularity_ceil(size_));
    }

    inline void bytes_array::clear()
    {
        resize(0);
    }

    //////////////////////////////////////////////////////////////////////////
    inline bytes_array::pointer bytes_array::data()
    {
        return is_dynamic() ? dyn_buffer_ : small_buffer_;
    }

    inline bytes_array::const_pointer bytes_array::data() const
    {
        return is_dynamic() ? dyn_buffer_ : small_buffer_;
    }

    inline size_t bytes_array::size() const
    {
        return size_;
    }

    inline bool bytes_array::empty() const
    {
        return size_ == 0;
    }

    inline bytes_array::pointer bytes_array::begin() { return data(); }
    inline bytes_array::pointer bytes_array::end  () { return data() + size(); }
    
    inline bytes_array::const_pointer bytes_array::begin() const { return data(); }
    inline bytes_array::const_pointer bytes_array::end  () const { return data() + size(); }


    inline bytes_array::byte_type& bytes_array::operator[](size_t index)
    {
        return data()[index];
    }

    inline bytes_array::byte_type bytes_array::operator[](size_t index) const
    {
        return data()[index];
    }

    inline bool bytes_array::is_dynamic() const
    {
        return capacity_ > small_obj_opt_size;
    }

    inline void bytes_array::buy_more_space(size_t min_size)
    {
        size_t max = std::max(min_size, size_t(capacity_ * blow_factor));
        max = granularity_ceil(max);

        pointer new_buf = new byte_type[max];

        if (!empty())
            memcpy(new_buf, data(), granularity_ceil(size_));

        delete[] dyn_buffer_;

        capacity_ = max;
        dyn_buffer_ = new_buf;
    }

    inline size_t bytes_array::granularity_ceil(size_t current)
    {
        // memory granularity is taken 8
        if ((current & 0x07) != 0)
            current = (current | 0x07) + 1;

        return current;
    }
} // binary
