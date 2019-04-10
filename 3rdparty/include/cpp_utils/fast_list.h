/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once 
#include "serialization/io_streams_fwd.h"
#include "alloc/pool_stl.h"

namespace cpp_utils
{
    template<class T>
    struct fast_appending_list
    {
        typedef T   value_type;
        typedef T&  reference;
        typedef T*  pointer;

    /// List Node /////////////////////////////////////////////////////////////
    private:
        struct node_t
            : noncopyable
        {
            node_t()
                : next(0)
            {}

            node_t(T&& t)
                : item(move(t))
                , next(0)
            {}

            node_t(T const& t)
                : item(t)
                , next(0)
            {}

            T       item;
            node_t* next;
        };

    /// Iterators ////////////////////////////////////////////////////////////
    public:
        template<class derived_t>
        struct iterator_base
        {
            typedef typename fast_appending_list::value_type     value_type;
            typedef typename fast_appending_list::reference      reference;
            typedef typename fast_appending_list::pointer        pointer;

            explicit iterator_base(node_t* ref = nullptr)
                : ref_(ref)
            {}

            derived_t& operator++()
            {
                ref_ = ref_->next;
                return static_cast<derived_t&>(*this);
            }

            derived_t operator++(int)
            {
                iterator tmp(*this);
                ++*this;
                return static_cast<derived_t&>(tmp);
            }

            bool operator==(iterator_base const& other) const 
            {                                            
                return ref_ == other.ref_;               
            }                                            
                                                         
            bool operator!=(iterator_base const& other) const 
            {
                return !operator==(other);
            }

        protected:
            node_t* ref_;
        };

        struct iterator
            : iterator_base < iterator >
        {
            explicit iterator(node_t* ref = nullptr)
                : iterator_base(ref)
            {}

            T* operator->() { return &(ref_->item); }
            T& operator* () { return ref_->item; }
        };

        struct const_iterator
            : iterator_base < const_iterator >
        {
            explicit const_iterator(node_t* ref = nullptr)
                : iterator_base(ref)
            {}

            T const* operator->() const { return &(ref_->item); }
            T const& operator* () const { return ref_->item; }
        };

    /// Public Interface //////////////////////////////////////////////////////////
    public:
        fast_appending_list()
            : head_(nullptr)
            , tail_(nullptr)
            , size_(0)
        {}

        ~fast_appending_list()
        {
            clear();
        }

        fast_appending_list(fast_appending_list const& other)
            : fast_appending_list()
        {
            for (auto const& value : other)
                push_back(value);
        }

        fast_appending_list(fast_appending_list&& other)
            : fast_appending_list()
        {
            swap(other);
        }

        fast_appending_list& operator=(fast_appending_list other)
        {
            swap(other);
            return *this;
        }

        void swap(fast_appending_list& other)
        {
            using std::swap;
            swap(alloc_, other.alloc_);
            swap(head_, other.head_);
            swap(tail_, other.tail_);
            swap(size_, other.size_);
        }

    public:
        void push_back(T const& value)
        {
            node_t* back_node = new (buy_node_space()) node_t(value);
            push_back_node(back_node);
        }

        void push_back(T&& value)
        {
            node_t* back_node = new (buy_node_space()) node_t(move(value));
            push_back_node(back_node);
        }

        void clear()
        {
            node_t* cur_node = head_;
            while (cur_node)
            {
                node_t* next = cur_node->next;
                remove_node(cur_node);
                cur_node = next;
            }

            head_ = 0;
            tail_ = 0;
            size_ = 0;
        }

    public:
        size_t  size () const { return size_; }
        bool    empty() const { return size_ == 0; }

        iterator begin() { return iterator(head_); }
        iterator end() { return iterator(0); }

        const_iterator begin() const { return const_iterator(head_); }
        const_iterator end() const { return const_iterator(0); }

        reference front() { return head_->item; }
        reference back()  { return tail_->item; }

    private:
        node_t* buy_node_space()
        {
            return alloc_.allocate(1);
        }

        void remove_node(node_t* node)
        {
            node->~node_t();
            alloc_.deallocate(node, 1);
        }

        void push_back_node(node_t* node)
        {
            if (!empty())
            {
                tail_->next = node;
                tail_ = node;
            }
            else
            {
                head_ = node;
                tail_ = node;
            }

            ++size_;
        }

    private:
        private_heap_allocator<node_t>  alloc_;
        node_t*                         head_;
        node_t*                         tail_; // just fast reference to the end
        size_t                          size_;
    };

    template<class T>
    void write(binary::output_stream& os, fast_appending_list<T> const& list)
    {
        write(os, binary::size_type(list.size()));
        for (auto const& item : list)
            write(os, item);
    }

    template<class T>
    void read(binary::input_stream& is, fast_appending_list<T>& list)
    {
        binary::size_type sz = 0;
        read(is, sz);

        for (size_t i = 0; i < sz; ++i)
        {
            T item;
            read(is, item);

            list.push_back(std::move(item));
        }
    }
} // cpp_utils
