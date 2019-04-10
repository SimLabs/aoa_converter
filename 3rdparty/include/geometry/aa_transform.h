/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "primitives/point.h"
#include "primitives/triangle.h"

namespace geom {

    struct aa_transform
    {
        point_2  world2local (point_2 const &pt) const {
            point_2 res = pt;
            res -= org_;
            res /= unit_;
            return res;
        }

        point_2  local2world (point_2 const &pt) const {
            point_2 res = pt;
            res &= unit_;
            res += org_;
            return res;
        }

        aa_transform(point_2 const &org, point_2 const &unit)
            :   org_ (org), unit_ (unit)
        {}

        aa_transform() : org_(), unit_(1,1) {}

        point_2 const & origin() const { return org_; }
        point_2 const & unit  () const { return unit_;}

        template<class processor> friend void reflect2(processor&, aa_transform const&, aa_transform const&);

    private:
        point_2  org_;
        point_2  unit_;
    };

    inline segment_2 world2local(segment_2 const &s, aa_transform const &tform)
    {
        return segment_2(tform.world2local(s.P0()), tform.world2local(s.P1()));
    }

    inline triangle_2 world2local(triangle_2 const &t, aa_transform const &tform)
    {
        return
            triangle_2(
                tform.world2local(t[0]),
                tform.world2local(t[1]),
                tform.world2local(t[2])
            );
    }

    inline rectangle_2 cellbound (aa_transform const & tform, point_2i const & idx)
    {
        point_2 const & org = tform.local2world(idx);
        return rectangle_2(org, org + tform.unit());
    }

    inline point_2    cellcenter (aa_transform const & tform, point_2i const & idx)
    {
       return tform.local2world(idx + point_2(0.5, 0.5));
    }
}
