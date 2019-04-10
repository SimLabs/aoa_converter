/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "cycle_val_fwd.h"

namespace geom
{
    template<typename range>
    struct cycle_value
    {
        typedef double scalar_type;

        double x;

        cycle_value() : x(range::def())   {}
        cycle_value( double x ) : x ( norm(x) ) {}
        cycle_value( cycle_value const& cv ) : x ( cv.x ) {}

        operator double  () const { return x ; }
        operator double& ()       { return x ; }

        static double norm ( double x )
        {
             if ( x >= range::max() )
                x = range::min() + geom::mod ( x - range::min(), range::max() - range::min() ) ;
             else if ( x < range::min() )
                x = range::max() - geom::mod ( range::max() - x, range::max() - range::min() );
             return x ;
        }

        static double min() { return range::min(); }
        static double max() { return range::max(); }
    };

    // ----------------------------------------------------------------------------------
    // basic operations
    template<typename range>
    inline cycle_value<range> operator - (cycle_value<range> const &A)
    {   return cycle_value<range>( - A.x );  }

    template<typename range>
    inline cycle_value<range> operator + (cycle_value<range> const &A, cycle_value<range> const &B)
    {   return cycle_value<range>( A.x + B.x );  }

    template<typename range>
    inline cycle_value<range> operator + (cycle_value<range> const &A, double x)
    {   return cycle_value<range>( A.x + x );  }

    template<typename range>
    inline cycle_value<range> operator + (double x, cycle_value<range> const &A)
    {   return cycle_value<range>( A.x + x );  }

    template<typename range>
    inline cycle_value<range> operator - (cycle_value<range> const &A, double x)
    {   return cycle_value<range>( A.x - x );  }

    template<typename range>
    inline cycle_value<range> operator - (cycle_value<range> const &A, cycle_value<range> const& B)
    {   return cycle_value<range>( A.x - B.x );  }

    template<typename range>
    inline cycle_value<range>  operator * (cycle_value<range> const &A, double d)
    {   return cycle_value<range>( A.x * d ); }

    template<typename range>
    inline cycle_value<range>  operator * (cycle_value<range> const &A, cycle_value<range> const &B)
    {   return cycle_value<range>( A.x * B.x ); }

    template<typename range>
    inline cycle_value<range>  operator * ( double d, cycle_value<range> const &A)
    {   return cycle_value<range>( A.x * d ); }

    template<typename range>
    inline cycle_value<range>  operator / (cycle_value<range> const &A, double d)
    {   return cycle_value<range>( A.x / d ); }

    template<typename range>
    double distance ( cycle_value<range> const& A, cycle_value<range> const& B )
    {   
        return min(fabs(A.x - B.x), 360. - fabs(A.x - B.x));
    }

    template<typename range>
    inline bool eq(cycle_value<range> const &A, cycle_value<range> const &B)
    {   return eq(0., distance(A,B));  }


    template<typename range>
    double path ( cycle_value<range> const& A, cycle_value<range> const& B )
    {
        return B.x < A.x ? 360. - (A.x - B.x) : B.x - A.x;
    }


    template<typename range>
    double left_turn ( cycle_value<range> const& A, cycle_value<range> const& B )
    {
        return path(A, B) < (range::max() - range::min())/2;
    }


}
