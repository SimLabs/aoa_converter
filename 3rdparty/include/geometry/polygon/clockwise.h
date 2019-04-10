/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

namespace geom
{
    // first and last point must not be equal
    template <class Point>
        inline bool clockwise_ordered( Point const *points, int count )
    {
        double maxy = -std::numeric_limits<double>::max();
        int idx  = -1;

        for (int i=0; i<count; ++i)
        {
            if (maxy < points[i].y)
            {
                maxy = points[i].y;
                idx = i;
            }
        }

        int i_prev = prev(idx, count);
        int i_next = next(idx, count);

        return normalized( point_2( points[i_prev] - points[idx] ) ).x <
            normalized( point_2( points[i_next] - points[idx] ) ).x;
    }

    template <class FwdIter>
        FwdIter findTopmost (FwdIter first, FwdIter beyond)
    {
        Assert(first != beyond);

        double maxy =  first->y;
        FwdIter best = first;

        for (++first; first != beyond; ++first)
        {
            if (maxy < first->y)
            {
                maxy = first->y;
                best = first;
            }
        }

        return best;
    }

    template <class FwdIter, class Traits>
        FwdIter findTopmost (FwdIter first, FwdIter beyond, Traits const & traits)
    {
        Assert(first != beyond);

        double maxy =  traits.at(*first).y;
        FwdIter best = first;

        for (++first; first != beyond; ++first)
        {
            if (maxy < traits.at(*first).y)
            {
                maxy = traits.at(*first).y;
                best = first;
            }
        }

        return best;
    }

    template <class FwdIter>
        inline bool clockwise_ordered( FwdIter first, FwdIter beyond )
    {
        if (first != beyond)
        {
           if ( *first == *( beyond - 1 ) )
              --beyond;

           if ( first == beyond )
              return false;

           FwdIter topmost = findTopmost(first, beyond);

           FwdIter i_prev = topmost == first ? beyond - 1 : topmost - 1;
           FwdIter i_next = topmost == beyond - 1 ? first : topmost + 1;

           return normalized( point_2( *i_prev - *topmost ) ).x <
                  normalized( point_2( *i_next - *topmost ) ).x;
        }
        else
        {
           // Can't determine orientation for contour without points.
           Assert(0);
           return false;
        }
    }

    template <class FwdIter, class Traits>
        inline bool clockwise_ordered( FwdIter first, FwdIter beyond, Traits const & traits )
    {
        Assert( first != beyond );

        if ( traits.at( *first ) == traits.at( *( beyond - 1 ) ) )
           --beyond;

        if ( first == beyond )
           return false;

        FwdIter topmost = findTopmost(first, beyond, traits);

        FwdIter i_prev = topmost == first ? beyond - 1 : topmost - 1;
        FwdIter i_next = topmost == beyond - 1 ? first : topmost + 1;

        return normalized( point_2( traits.at(*i_prev) - traits.at(*topmost) ) ).x <
               normalized( point_2( traits.at(*i_next) - traits.at(*topmost) ) ).x;
    }
}
