/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "vector_2d_extents.h"
#include "geometry/primitives/rectangle.h"

#include "reflection/proc/io_streams_refl.h"

namespace geom 
{

    // 2d dimension array
    // data is located in sequential memory chunk
    // valid indexes must be from [0, extents)
    template <class T, class Shape = extents::rect, class Dir = extents::rowwise>
    struct vector_2d : Extents<Shape, Dir>
    {
        typedef Extents<Shape, Dir> Ext;

        typedef T                   value_type;
        typedef int                 linear_index;
        typedef point_2i            index_type;
        typedef point_2i const &    index_cref;
        typedef T &                 reference;
        typedef T const &           const_reference;
        typedef T *                 pointer;
        typedef T const *           const_pointer;
        typedef T *                 iterator;
        typedef T const *           const_iterator;

        vector_2d(index_cref ext, T const &defvalue);
        vector_2d(index_cref ext);
        vector_2d();
        vector_2d(vector_2d const & rhs);
        vector_2d const & operator = (vector_2d const & rhs);
        
        ~vector_2d();

        void resize(point_2i const & ext);
        void resize(point_2i const & ext, T const & defvalue);
        
        reference at (index_cref idx);
        const_reference at (index_cref idx) const;

        reference       operator [] (index_cref idx)       { return at(idx); }
        const_reference operator [] (index_cref idx) const { return at(idx); }

        iterator begin() { return data_; }
        iterator end  () { return data_ + Ext::size(); }

        const_iterator begin() const { return data_; }
        const_iterator end  () const { return data_ + Ext::size(); }

        index_type index(const_iterator p) const {  return Ext::to2D(int(p - data_)); }

        bool contains(const_iterator p) const {  return data_ <= p && p <= data_ + Ext::size(); }

        bool contains(index_cref idx) const {   return Ext::contains(idx);  }

        size_t memory_allocated() const;

    private:
        void copy( vector_2d const & rhs );

    private:
        T  * data_;
    };

    //////////////////////////////////////////////////////////////////////////

    template< class T, class Shape, class Dir >
    inline void write( binary::output_stream& stream, vector_2d<T, Shape, Dir> const& array )
    {
        write( stream, array.extents() );

        for ( auto v = array.begin(); v != array.end(); ++v )
            write(stream, *v);
    }

    template< class T, class Shape, class Dir >
    inline void read( binary::input_stream& stream, vector_2d<T, Shape, Dir> & array )
    {
        geom::point_2i ext;
        read( stream, ext );

        array.resize(ext);

        for ( auto v = array.begin(); v != array.end(); ++v )
            read( stream, *v );
    }


    //////////////////////////////////////////////////////////////////////////

    template <class T, class S, class D>
    vector_2d<T,S,D>::vector_2d(index_cref ext, T const &defvalue)
        : Ext(ext), data_(new T[Ext::size()])
    {
        std::fill(data_, data_ + Ext::size(), defvalue);
    }

    template <class T, class S, class D>
    vector_2d<T,S,D>::vector_2d(index_cref ext)
        : Ext(ext), data_(new T[Ext::size()])
    {}

    template <class T, class S, class D>
    vector_2d<T,S,D>::vector_2d() 
        : data_(nullptr) 
    {}

    template <class T, class S, class D>
    vector_2d<T,S,D>::vector_2d(vector_2d const & rhs)
        : Ext(rhs.extents()), data_(new T[Ext::size()])
    {
        copy( rhs );
    }

    template <class T, class S, class D>
        vector_2d<T,S,D> const & vector_2d<T,S,D>::operator=(vector_2d const & rhs)
    {
        if ( this != &rhs )
        {
            resize( rhs.extents( ) );
            copy( rhs );
        }
        return *this;
    }

    template <class T, class S, class D>
    vector_2d<T,S,D>::~vector_2d() 
    {
        delete [] data_; 
    }

    template <class T, class S, class D>
        void vector_2d<T,S,D>::resize(point_2i const & ext)
    {
        Ext::resize(ext);

        if (data_)
            delete [] data_;

        data_ = new T[Ext::size()];
    }

    template <class T, class S, class D>
        void vector_2d<T,S,D>::resize(point_2i const & ext, T const & defvalue)
    {
        resize(ext);

        std::fill(data_, data_ + Ext::size(), defvalue);
    }

    template <class T, class S, class D>
        typename vector_2d<T,S,D>::reference vector_2d<T,S,D>::at (index_cref idx)
    {
        Ext::assert_valid(idx);
        return data_[Ext::to1D(idx)];
    }

    template <class T, class S, class D>
        typename vector_2d<T,S,D>::const_reference vector_2d<T,S,D>::at (index_cref idx) const 
    {
        Ext::assert_valid(idx);
        return data_[Ext::to1D(idx)];
    }

    template <class T, class S, class D>
        size_t vector_2d<T,S,D>::memory_allocated() const 
        {
            return
                sizeof(Ext::extents_) +
                sizeof(data_) +
                sizeof(T) * Ext::size();
        }

    template <class T, class S, class D>
        void vector_2d<T,S,D>::copy( vector_2d const & rhs )
        {
            iterator j = begin();
            for ( const_iterator i = rhs.begin(), e = rhs.end(); i != e; ++i, ++j )
                *j = *i;
        }

}
