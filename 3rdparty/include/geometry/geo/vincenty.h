/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/geo/projection/mercator.h"

namespace geom
{

namespace vincenty
{

inline polar_point_2 geo_inverse(geo_point_2 const& a, geo_point_2 const& b)
{
    // http://en.wikipedia.org/wiki/Vincenty's_formulae
    // inverse problem
    // Between two nearly antipodal points, the iterative formula may fail to converge!!!
    using namespace wgs84;

    const double eps = 1.0e-9;
    const size_t max_iters = 50;
    if (fabs(a.lat - b.lat) < eps && fabs(a.lon - b.lon) < eps)
    {
        return 0.0;
    }
    double L = geom::grad2rad(b.lon - a.lon);
    double U1 = atan((1.0 - F) * tan(geom::grad2rad(a.lat)));
    double U2 = atan((1.0 - F) * tan(geom::grad2rad(b.lat)));
    double sinU1 = sin(U1), cosU1 = cos(U1);
    double sinU2 = sin(U2), cosU2 = cos(U2);
    double lambda = L;

    double sinLambda, cosLambda;
    double sinSigma, cosSigma;
    double sigma;
    double cos2SigmaM;
    double cosSqAlpha;

    for (size_t i = 0; i < max_iters; ++i)
    {
        sinLambda = sin(lambda);
        cosLambda = cos(lambda);

        sinSigma = sqrt(sqr(cosU2 * sinLambda) + sqr(cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));
        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma = atan2(sinSigma, cosSigma);

        double sinAlpha = (sinSigma == 0.0) ? 0.0 : (cosU1 * cosU2 * sinLambda) / sinSigma;
        cosSqAlpha = 1.0 - sqr(sinAlpha);
        cos2SigmaM = (cosSqAlpha == 0.0) ? 0.0 : cosSigma - 2.0 * sinU1 * sinU2 / cosSqAlpha;

        double C = (F / 16.0) * cosSqAlpha * (4.0 + F * (4.0 - 3.0 * cosSqAlpha));

        double lambdaP = lambda;
        lambda = L + (1.0 - C) * F * sinAlpha * (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * sqr(cos2SigmaM))));
        if (fabs(lambda - lambdaP) < eps)
        {
            break;
        }
    }
    double uSq = cosSqAlpha * (sqr(Rbig) - sqr(Rsmall)) / sqr(Rsmall);
    double A = 1.0 + (uSq / 16384.0) * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
    double B = (uSq / 1024.0) * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));
    double deltaSigma = B * sinSigma * (cos2SigmaM + 0.25 * B * (cosSigma * (-1.0 + 2.0 * sqr(cos2SigmaM)) -
                            (1.0 / 6.0) * B * cos2SigmaM * (-3.0 + 4.0 * sqr(sinSigma)) * (-3.0 + 4.0 * sqr(cos2SigmaM))));
    return polar_point_2(Rsmall * A * (sigma - deltaSigma),
                         geom::rad2grad(atan2(cosU2 * sinLambda, cosU1 * sinU2 - sinU1 * cosU2 * cosLambda)));
}

inline geo_point_2 geo_direct(geo_point_2 const & p, polar_point_2 const& offset)
{
    using namespace geom::wgs84;
    // http://en.wikipedia.org/wiki/Vincenty's_formulae
    // direct problem
    const double eps = 1.0e-9;
    const size_t max_iters = 50;
    double alpha1 = geom::grad2rad(offset.course);
    double sinAlpha1 = sin(alpha1);
    double cosAlpha1 = cos(alpha1);

    double tanU1 = (1.0 - F) * tan(geom::grad2rad(p.lat));
    double cosU1 = 1.0 / sqrt(1.0 + sqr(tanU1));
    double sinU1 = tanU1 * cosU1;
    double sigma1 = atan2(tanU1, cosAlpha1);
    double sinAlpha = cosU1 * sinAlpha1;
    double cosSqAlpha = 1.0 - sqr(sinAlpha);
    double uSq = cosSqAlpha * (sqr(Rbig) - sqr(Rsmall)) / sqr(Rsmall);
    double A = 1.0 + uSq / 16384.0 * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
    double B = uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));

    double sigma = offset.range / (Rsmall * A);
    double cos2SigmaM, sinSigma, cosSigma;
    double deltaSigma;

    for (size_t i = 0; i < max_iters; ++i)
    {
        cos2SigmaM = cos(2.0 * sigma1 + sigma);
        sinSigma = sin(sigma);
        cosSigma = cos(sigma);
        deltaSigma = B * sinSigma * (cos2SigmaM + 0.25 * B * (cosSigma * (-1.0 + 2.0 * sqr(cos2SigmaM)) -
                                (1.0 / 6.0) * B * cos2SigmaM * (-3.0 + 4.0 * sqr(sinSigma)) * (-3.0 + 4.0 * sqr(cos2SigmaM))));
        double sigmaP = sigma;
        sigma = offset.range / (Rsmall * A) + deltaSigma;
        if (fabs(sigma - sigmaP) < eps)
        {
            break;
        }
    }
    double tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
    double lat2 = atan2(sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1,
                      (1.0 - F) * sqrt(sqr(sinAlpha) + sqr(tmp)));
    double lambda = atan2(sinSigma * sinAlpha1,
                        cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1);
    double C = (F / 16.0) * cosSqAlpha * (4.0 + F * (4.0 - 3.0 * cosSqAlpha));
    double L = lambda - (1.0 - C) * F * sinAlpha * (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * sqr(cos2SigmaM))));
    double lon2 = geom::grad2rad(p.lon) + L;
    if (lon2 < -pi)
    {
        lon2 += 2.0 * pi;
    }
    if (lon2 > pi)
    {
        lon2 -= 2.0 * pi;
    }
    return geo_point_2(geom::rad2grad(lat2), geom::rad2grad(lon2));
}

}

}

