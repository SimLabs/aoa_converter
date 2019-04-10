/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "camera_fwd.h"

#include "geometry/primitives.h"
#include "geometry/matrix_helpers.h"

#pragma pack(push, 1)

namespace geom
{
    template< typename S >
    class camera_t
    {
    public:

        typedef point_t      <S, 3> point_3t;
        typedef cpr_t        <S>    cpr_3t;
        typedef transform_t  <S, 4> transform_4t;

    public:

        camera_t();
        camera_t( point_3t const & pos,  cpr_3t const & orientation );
        camera_t( point_3t const & look, point_3t const & look_at, point_3t const & up );

        camera_t( camera_t const & other );
        camera_t & operator=( camera_t const & other);

        template< typename _S >
        camera_t( camera_t<_S> const & other );

        template< typename _S >
        camera_t & operator=( camera_t<_S> const & other );

        void set_position   ( point_3t const & position );
        void set_orientation( cpr_3t   const & orientation );

        point_3t const & position   () const;
        cpr_3t   const & orientation() const;

        point_3t dir  () const;
        point_3t up   () const;
        point_3t right() const;

        transform_4t const & world_view_transform   () const;

        size_t version() const;

    private:
        void reset_transform    ();
        void calculate_transform() const;

    private:
        point_3t pos_;
        cpr_3t   orientation_;

        mutable boost::optional<transform_4t> world_view_;
        size_t version_;

        template<typename> friend class camera_t;
    };

    //
    // impl
    //

    template< typename S >
    camera_t<S>::camera_t()
        : version_(-1)
    {
    }

    template< typename S >
    camera_t<S>::camera_t( point_3t const & pos, cpr_3t const & orientation )
        : pos_        (pos)
        , orientation_(orientation)
        , version_    (-1)
    {
        world_view_ = transform_4t(matrix_camera(pos, orientation), ss_unscaled);
    }

    template< typename S >
    camera_t<S>::camera_t( point_3t const & pos, point_3t const & look_at, point_3t const & up )
        : pos_    (pos)
        , version_(-1)
    {
        world_view_ = transform_4t(matrix_look_at(pos, look_at, up), ss_unscaled);
        orientation_ = world_view_->inverted().rotation().cpr();
    }

    template< typename S >
    camera_t<S>::camera_t( camera_t<S> const & other )
        : pos_          (other.pos_)
        , orientation_  (other.orientation_)
        , world_view_   (other.world_view_)
        , version_      (-1)
    {
    }

    template< typename S >
    camera_t<S> & camera_t<S>::operator=( camera_t<S> const & other )
    {
        pos_           = other.pos_;
        orientation_   = other.orientation_;
        world_view_    = other.world_view_;

        ++version_;
        return *this;
    }

    template< typename S > template < typename _S >
    camera_t<S>::camera_t( camera_t<_S> const & other )
        : pos_        (other.pos_)
        , orientation_(other.orientation_)
        , world_view_ (other.world_view_)
        , version_    (-1)
    {
    }

    template< typename S > template< typename _S >
    camera_t<S> & camera_t<S>::operator=( camera_t<_S> const & other )
    {
        pos_           = other.pos_;
        orientation_   = other.orientation_;
        world_view_    = other.world_view_;

        ++version_;
        return *this;
    }

    template< typename S >
    void camera_t<S>::set_orientation( cpr_3t const & orientation )
    {
        orientation_ = orientation;

        reset_transform();
        ++version_;
    }

    template< typename S >
    void camera_t<S>::set_position( point_3t const & position )
    {
        pos_ = position;

        reset_transform();
        ++version_;
    }

    template< typename S >
    typename camera_t<S>::point_3t const & camera_t<S>::position() const
    {
        return pos_;
    }

    template< typename S >
    typename camera_t<S>::cpr_3t const & camera_t<S>::orientation() const
    {
        return orientation_;
    }

    template< typename S >
    typename camera_t<S>::point_3t camera_t<S>::dir() const
    {
        return -world_view_transform().direct_matrix().get_row(2);
    }

    template< typename S >
    typename camera_t<S>::point_3t camera_t<S>::up() const
    {
        return world_view_transform().direct_matrix().get_row(1);
    }

    template< typename S >
    typename camera_t<S>::point_3t camera_t<S>::right() const
    {
        return world_view_transform().direct_matrix().get_row(0);
    }

    template< typename S >
    typename camera_t<S>::transform_4t const & camera_t<S>::world_view_transform() const
    {
        if (!world_view_)
        {
            world_view_ = transform_4t(matrix_camera(pos_, orientation_), ss_unscaled);
        }

        return *world_view_;
    }

    template< typename S >
    size_t camera_t<S>::version() const
    {
        return version_;
    }

    template< typename S >
    void camera_t<S>::reset_transform()
    {
        world_view_ = boost::none;
    }


    template<typename S>
    void read(binary::input_stream& is, camera_t<S>& cam)
    {
        using namespace binary;
        cam.set_position    (read_stream<typename camera::point_3t>(is));
        cam.set_orientation (read_stream<typename camera::cpr_3t  >(is));
    }

    template<typename S>
    void write(binary::output_stream& os, camera_t<S> const& cam)
    {
        write(os, cam.position   ());
        write(os, cam.orientation());
    }

} // geom


#pragma pack(pop)
