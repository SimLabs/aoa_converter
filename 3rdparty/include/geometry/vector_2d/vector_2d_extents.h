/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#if defined(__GNUG__)
    #pragma GCC diagnostic push

    #pragma GCC diagnostic ignored "-fpermissive"
    #pragma GCC diagnostic ignored "-Wparentheses"
#endif

#include "geometry/primitives/point_fwd.h"
#include "logger/assert.hpp"

namespace geom
{
    namespace extents
    {
        struct rect
        {
            rect() {}

            typedef point_2i extents_type;

            rect(extents_type const & ext) : extents_(ext) {}

            size_t width () const { return extents_.x; }
            size_t height() const { return extents_.y; }

        protected:
            void resize(point_2i const &ext) { extents_ = ext; }

        private:
            point_2i    extents_;
        };

        struct square
        {
            square() {}

            typedef size_t extents_type;

            square(size_t ext) : extents_(ext) {}

            square(point_2i const & ext) : extents_(ext.x)
            {
                Assert(ext.x == ext.y);
            }

            size_t width () const { return extents_; }
            size_t height() const { return extents_; }

        private:
            size_t   extents_;
        };

        struct columnwise
        {
            template <class E>
                static size_t principal(E const & ext)
            {
                return ext.height();
            }

            template <class E>
                static point_2i to2D(E const & ext, int idx)
            {
                return point_2i(idx / ext.height(), idx % ext.height());
            }
        };

        struct rowwise
        {
            template <class E>
                static size_t principal(E const & ext)
            {
                return ext.width();
            }

            template <class E>
                static point_2i to2D(E const & ext, int idx)
            {
                return point_2i(idx % (int)ext.width(), idx / (int)ext.width());
            }
        };
    }

    template <class Shape, class Selector>
        struct Extents : Shape
    {
        Extents() {}

        Extents(Shape const & ext) : Shape(ext) {}

        size_t size() const { return width() * height(); }

        point_2i extents() const { return point_2i((int)width(), (int)height()); }

        __forceinline bool contains(point_2i const &idx) const
        {
            return
                0 <= idx.x && size_t(idx.x) < width() &&
                0 <= idx.y && size_t(idx.y) < height();
        }

        typedef int linear_index;

        bool contains(linear_index const & idx) const
        {
            return 0 <= idx && idx < size();
        }

        void assert_valid(point_2i const & idx) const
        {
            Assert(0 <= idx.x);
            Assert(0 <= idx.y);
            Assert(size_t(idx.x) < width());
            Assert(size_t(idx.y) < height());
        }

        linear_index to1D(point_2i const & idx) const
        {
            return linear_index(idx.y * Selector::principal(*this) + idx.x);
        }

        point_2i to2D(linear_index const & idx) const
        {
            return Selector::to2D(*this, idx);
        }
    };
}

#if defined(__GNUG__)
    #pragma GCC diagnostic pop
#endif

