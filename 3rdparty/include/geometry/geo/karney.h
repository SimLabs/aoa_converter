/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once

#include "geometry/geo/projection/mercator.h"

namespace geom
{

// based on Geographiclib: http://geographiclib.sourceforge.net/index.html
// http://arxiv.org/pdf/1102.1215v1.pdf

namespace karney
{

struct geoid
{

geoid(double Rbig, double f)
    : Rbig(Rbig)
    , Rsmall(Rbig * (1.0 - f))
    , F(f)
    , Esq(f * (2.0 - f))
    , E2sq(Esq / sqr(1.0 - f))
    , N(f / (2.0 - f))
    , Csq((sqr(Rbig) + sqr(Rsmall) * (Esq == 0 ? 1 : boost::math::atanh(sqrt(Esq)) / sqrt(Esq))) / 2)
    , tiny(sqrt(std::numeric_limits<double>::min()))
    , tol0(geom::epsilon<double>())
    , tol1(200 * tol0)
    , tol2(sqrt(tol0))
    , tolb(tol0 * tol1)
    , etol2(0.1 * tol2 /
             sqrt(std::max(0.001, abs(F)) * std::min(1.0, 1 - F / 2) / 2))
{
    A3coeff(N, A3x);
    C3coeff(N, C3x);
    C4coeff(N, C4x);
}

static void A3coeff(double _n, double _A3x[])
{
    _A3x[0] = 1;
    _A3x[1] = (_n-1)/2;
    _A3x[2] = -1/4.0;
}

static void C3coeff(double _n, double _C3x[])
{
    _C3x[0] = (1-_n)/4;
    _C3x[1] = 1/8.0;
    _C3x[2] = 1/16.0;
}

static void C4coeff(double _n, double _C4x[])
{
    _C4x[0] = (_n * (8 * _n - 28) + 70) / 105;
    _C4x[1] = (16 * _n - 7) / 35;
    _C4x[2] = -2 / 105.0;
    _C4x[3] = (7 - 16 * _n) / 315;
    _C4x[4] = -2 / 105.0;
    _C4x[5] = 4 / 525.0;
}

inline polar_point_2 inverse(geo_point_2 const& a, geo_point_2 const& b) const
{
    double lon12 = geom::norm180(b.lon - a.lon);
    int lonsign = lon12 >= 0 ? 1 : -1;
    lon12 *= lonsign;

    double lam12 = geom::grad2rad(lon12);
    double lat1 = geom::grad2rad(a.lat);
    double lat2 = geom::grad2rad(b.lat);
    int swapp = 1;
    if (fabs(lat1) < fabs(lat2))
    {
        swapp = -1;
        lonsign = -lonsign;
        std::swap(lat1, lat2);
    }
    int latsign = lat1 < 0 ? 1 : -1;
    lat1 *= latsign;
    lat2 *= latsign;

    double sbet1 = (1.0 - F) * sin(lat1);
    double cbet1 = cos(lat1);
    if (cbet1 < tiny)
    {
        cbet1 = tiny;
    }
    double t1 = hypot(sbet1, cbet1);
    sbet1 /= t1; cbet1 /= t1;

    double sbet2 = (1.0 - F) * sin(lat2);
    double cbet2 = bound(cos(lat2), tiny, 1.0);
    double t2 = hypot(sbet2, cbet2);
    sbet2 /= t2; cbet2 /= t2;

    if (cbet1 < -sbet1)
    {
        if (geom::eq(cbet2, cbet1))
        {
            sbet2 = (sbet2 < 0) ? sbet1 : -sbet1;
        }
    }
    else
    {
        if (geom::eq(fabs(sbet2), -sbet1))
        {
            cbet2 = cbet1;
        }
    }

    double dn1 = sqrt(1.0 + E2sq * sqr(sbet1));
    double dn2 = sqrt(1.0 + E2sq * sqr(sbet2));
    double slam12 = geom::eq(lam12, pi) ? 0.0 : sin(lam12);
    double clam12 = cos(lam12);

    double s12x, m12x, m0;
    double sig12, calp1, salp1, calp2, salp2;
    double dummy;
    double C1a[PRECISION + 1];
    double C2a[PRECISION + 1];
    double C3a[PRECISION];

    bool meridian = geom::eq(lat1, -pi / 2.0) || (slam12 == 0.0);

    if (meridian)
    {
        salp1 = slam12, calp1 = clam12;
        salp2 = 0, calp2 = 1;
        double ssig1 = sbet1, csig1 = calp1 * cbet1;
        double ssig2 = sbet2, csig2 = calp2 * cbet2;
        sig12 = atan2(std::max(csig1 * ssig2 - ssig1 * csig2, 0.0),
            csig1 * csig2 + ssig1 * ssig2);
        lengths(N, sig12,
            ssig1, csig1, dn1, ssig2, csig2, dn2,
            s12x, m12x, m0,
            C1a, C2a);
        if (sig12 < 1.0 || m12x >= 0.0)
        {
            m12x *= Rsmall;
            s12x *= Rsmall;
        }
        else
        {
            meridian = false;
        }
    }
    double omg12;
    if (!meridian && geom::eq(sbet1, 0.0) && (F <= 0 || lam12 <= pi - F * pi))
    {
        calp1 = calp2 = 0;
        salp1 = salp2 = 1;
        s12x = Rbig * lam12;
        sig12 = omg12 = lam12 / (1.0 - F);
        m12x = Rsmall * sin(sig12);
    }
    else if (!meridian)
    {
        double dnm;
        sig12 = inverseStart(sbet1, cbet1, dn1, sbet2, cbet2, dn2, lam12,
                     salp1, calp1, salp2, calp2, dnm,
                     C1a, C2a);
        if (sig12 >= 0)
        {
            s12x = sig12 * Rsmall * dnm;
            m12x = sqr(dnm) * Rsmall * sin(sig12 / dnm);
            omg12 = lam12 / ((1.0 - F) * dnm);
        }
        else
        {
            double ssig1, csig1, ssig2, csig2, eps;
            unsigned numit = 0;
            double salp1a = tiny, calp1a = 1.0;
            double salp1b = tiny, calp1b = -1.0;
            for (; numit < MAXIT1; ++numit)
            {
                double dv;
                double v = Lambda12(sbet1, cbet1, dn1, sbet2, cbet2, dn2, salp1, calp1,
                                salp2, calp2, sig12, ssig1, csig1, ssig2, csig2,
                                eps, omg12, dv, C1a, C2a, C3a) - lam12;
                if (fabs(v) < tol0)
                {
                    break;
                }
                if (v > 0)
                {
                    salp1b = salp1; calp1b = calp1;
                }
                else
                {
                    salp1a = salp1; calp1a = calp1;
                }
                if (dv > 0)
                {
                    double dalp1 = -v / dv;
                    double sdalp1 = sin(dalp1), cdalp1 = cos(dalp1);
                    double nsalp1 = salp1 * cdalp1 + calp1 * sdalp1;
                    if (nsalp1 > 0 && fabs(dalp1) < pi)
                    {
                        calp1 = calp1 * cdalp1 - salp1 * sdalp1;
                        salp1 = std::max(0.0, nsalp1);
                        double t = hypot(salp1, calp1);
                        salp1 /= t; calp1 /= t;
                        continue;
                    }
                }
                salp1 = (salp1a + salp1b) / 2.0;
                calp1 = (calp1a + calp1b) / 2.0;
                double t = hypot(salp1, calp1);
                salp1 /= t; calp1 /= t;
            }


            lengths(eps, sig12, ssig1, csig1, dn1, ssig2, csig2, dn2,
                              s12x, m12x, dummy, C1a, C2a);
            m12x *= Rsmall;
            s12x *= Rsmall;
            omg12 = lam12 - omg12;
        }
    }
    double s12 = s12x;
    if (swapp < 0)
    {
        std::swap(salp1, salp2);
        std::swap(calp1, calp2);
    }

    salp1 *= swapp * lonsign; calp1 *= swapp * latsign;
    salp2 *= swapp * lonsign; calp2 *= swapp * latsign;

    // minus signs give range [-180, 180). 0- converts -0 to +0.
    double azi1 = geom::rad2grad(- atan2(- salp1, calp1));
//    double azi2 = geom::rad2grad(- atan2(- salp2, calp2));
    return polar_point_2(s12, azi1);
}

inline geo_point_2 direct(geo_point_2 const& p, polar_point_2 const& offset )
{
    double lat1 = geom::grad2rad(p.lat), lon1 = geom::grad2rad(p.lon);

    double course = geom::grad2rad(offset.course);
    double salp1 = sin(course);
    double calp1 = cos(course);

    double sbet1 = (1.0 - F) * sin(lat1);
    double cbet1 = eq(lat1, pi / 2.0) ? tiny : cos(lat1);
    double t1 = hypot(sbet1, cbet1);
    sbet1 /= t1; cbet1 /= t1;

    double salp0 = salp1 * cbet1; // alp0 in [0, pi/2 - |bet1|]
    // Alt: calp0 = hypot(sbet1, calp1 * cbet1).  The following
    // is slightly better (consider the case salp1 = 0).
    double calp0 = hypot(calp1, salp1 * sbet1);

    double somg1, comg1;
    double ssig1 = sbet1; somg1 = salp0 * sbet1;
    double csig1 = comg1 = sbet1 != 0 || calp1 != 0 ? cbet1 * calp1 : 1;
    double t = hypot(ssig1, csig1);
    ssig1 /= t; csig1 /= t;

    double k2 = sqr(calp0) * E2sq;
    double eps = k2 / (2 * (1 + sqrt(1 + k2)) + k2);
    double A1m1 = A1m1f(eps);

    double C3a[PRECISION];
    C3f(eps, C3a);
    double A3c = -F * salp0 * A3f(eps);
    double B31 = sinCosSeries(true, ssig1, csig1, C3a, PRECISION - 1);

    double C1a[PRECISION + 1];
    double C1pa[PRECISION + 1];
    C1f(eps, C1a);
    C1pf(eps, C1pa);
    double B11 = sinCosSeries(true, ssig1, csig1, C1a);

    double sB11 = sin(B11), cB11 = cos(B11);
    double stau1 = ssig1 * cB11 + csig1 * sB11;
    double ctau1 = csig1 * cB11 - ssig1 * sB11;

    double tau12 = offset.range / (Rsmall * (1 + A1m1));
    double stau12 = sin(tau12);
    double ctau12 = cos(tau12);
    double B12 = - sinCosSeries(true, stau1 * ctau12 + ctau1 * stau12,
                      ctau1 * ctau12 - stau1 * stau12, C1pa);
    double sig12 = tau12 - (B12 - B11);
    double ssig12 = sin(sig12), csig12 = cos(sig12);
    if (abs(F) > 0.01)
    {
        // Reverted distance series is inaccurate for |f| > 1/100, so correct
        // sig12 with 1 Newton iteration.
        double ssig2 = ssig1 * csig12 + csig1 * ssig12,
            csig2 = csig1 * csig12 - ssig1 * ssig12;
        B12 = sinCosSeries(true, ssig2, csig2, C1a);
        double serr = (1 + A1m1) * (sig12 + (B12 - B11)) - offset.range / Rsmall;
        sig12 = sig12 - serr / sqrt(1 + k2 * sqr(ssig2));
        ssig12 = sin(sig12); csig12 = cos(sig12);
    }

    double ssig2 = ssig1 * csig12 + csig1 * ssig12;
    double csig2 = csig1 * csig12 - ssig1 * ssig12;
    double sbet2 = calp0 * ssig2;
    double cbet2 = hypot(salp0, calp0 * csig2);
    if (cbet2 == 0)
    {
        cbet2 = csig2 = tiny;
    }
    double somg2 = salp0 * ssig2, comg2 = csig2;  // No need to normalize
    double omg12 = atan2(somg2 * comg1 - comg2 * somg1,
                  comg2 * comg1 + somg2 * somg1);
    double lon12 = omg12 + A3c * (sig12 + (sinCosSeries(true, ssig2, csig2, C3a, PRECISION - 1) - B31));
    double lon2 = geom::norm180(geom::rad2grad(lon1 + lon12));
    double lat2 = geom::rad2grad(atan2(sbet2, (1.0 - F) * cbet2));
    return geo_point_2(lat2, lon2);
}

inline double sinCosSeries(bool sinp, double sinx, double cosx, double c[], int n = PRECISION) const
{
    int k = n + (sinp ? 1 : 0);
    double ar = 2 * (cosx - sinx) * (cosx + sinx);
    double y0 = (n&1) ? c[--k] : 0, y1 = 0;
    n /= 2;
    while (n--)
    {
        y1 = ar * y0 - y1 + c[--k];
        y0 = ar * y1 - y0 + c[--k];
    }
    return (sinp ? 2 * sinx * cosx * y0 : cosx * (y0 - y1));
}

inline void C1f(double eps, double c[]) const
{
    double eps2 = sqr(eps);
    double d = eps;

    c[1] = d * (3 * eps2 - 8) / 16;
    d *= eps;
    c[2] = -d / 16;
    d *= eps;
    c[3] = -d / 48;
}

inline void C1pf(double eps, double c[]) {
    double eps2 = sqr(eps);
    double d = eps;

    c[1] = d * (16 - 9 * eps2) / 32;
    d *= eps;
    c[2] = 5 * d / 16;
    d *= eps;
    c[3] = 29 * d / 96;
}

inline void C2f(double eps, double c[]) const
{
    double eps2 = sqr(eps);
    double d = eps;

    c[1] = d * (eps2 + 8) / 16;
    d *= eps;
    c[2] = 3 * d / 16;
    d *= eps;
    c[3] = 5 * d / 48;
}

inline void C3f(double eps, double c[]) const
{
  // Evaluate C3 coeffs by Horner's method
  // Elements c[1] thru c[nC3_ - 1] are set
    for (int j = PRECISION * (PRECISION - 1) / 2, k = PRECISION - 1; k > 0; )
    {
        double t = 0;
        for (int i = PRECISION - k; i > 0; --i)
        {
            t = eps * t + C3x[--j];
        }
        c[k--] = t;
    }
    double mult = 1;
    for (size_t k = 1; k < PRECISION; )
    {
        mult *= eps;
        c[k++] *= mult;
    }
}

inline void C4f(double eps, double c[]) const
{
    for (int j = PRECISION * (PRECISION + 1) / 2, k = PRECISION; k; )
    {
        double t = 0;
        for (int i = PRECISION - k + 1; i; --i)
        {
            t = eps * t + C4x[--j];
        }
        c[--k] = t;
    }
    double mult = 1;
    for (size_t k = 1; k < PRECISION;)
    {
        mult *= eps;
        c[k++] *= mult;
    }
}

inline double A1m1f(double eps) const
{
    double eps2 = sqr(eps);
    double t = eps2*(eps2*(eps2+4)+64)/256;
    return (t + eps) / (1 - eps);
}

inline double A2m1f(double eps) const
{
    double eps2 = sqr(eps);
    double t = eps2*(eps2*(25*eps2+36)+64)/256;
    return t * (1 - eps) - eps;
}

inline double A3f(double eps) const
{
    double v = 0;
    for(int i = PRECISION; i--;)
        v = eps * v + A3x[i];
    return v;
}

inline void lengths(double eps, double sig12, double ssig1, double csig1,
            double dn1, double ssig2, double csig2, double dn2,
            double& s12b, double& m12b, double& m0,
            double C1a[], double C2a[]) const
{
    C1f(eps, C1a);
    C2f(eps, C2a);
    double A1m1 = A1m1f(eps);
    double AB1 = (1 + A1m1) * (sinCosSeries(true, ssig2, csig2, C1a) - sinCosSeries(true, ssig1, csig1, C1a));
    double A2m1 = A2m1f(eps);
    double AB2 = (1 + A2m1) * (sinCosSeries(true, ssig2, csig2, C2a) - sinCosSeries(true, ssig1, csig1, C2a));
    m0 = A1m1 - A2m1;
    double J12 = m0 * sig12 + (AB1 - AB2);
    m12b = dn2 * (csig1 * ssig2) - dn1 * (ssig1 * csig2) - csig1 * csig2 * J12;
    s12b = (1 + A1m1) * sig12 + AB1;
}

inline double inverseStart(double sbet1, double cbet1, double dn1,
               double sbet2, double cbet2, double dn2,
               double lam12,
               double& salp1, double& calp1,
               double& salp2, double& calp2,
               double& dnm,
               double C1a[], double C2a[]) const
{
    double sbet12 = sbet2 * cbet1 - cbet2 * sbet1;
    double cbet12 = cbet2 * cbet1 + sbet2 * sbet1;
    double sig12=-1;
    double sbet12a = sbet2 * cbet1 + cbet2*sbet1;
    bool shortline = cbet12 >= 0 && sbet12 < 0.5 && cbet2 * lam12 < 0.5;
    double omg12 = lam12;
    if (shortline)
    {
        double sbetm2 = sqr(sbet1 + sbet2);
        sbetm2 /= sbetm2 + sqr(cbet1 + cbet2);
        dnm = sqrt(1 + E2sq * sbetm2);
        omg12 /= (1.0 - F) * dnm;
    }
    double somg12 = sin(omg12), comg12 = cos(omg12);
    salp1 = cbet2 * somg12;
    calp1 = comg12 >=0 ?
        sbet12 + cbet2 * sbet1 * sqr(somg12) / (1 + comg12):
        sbet12a - cbet2 * sbet1 * sqr(somg12) / (1 - comg12);
    double ssig12 = hypot(salp1, calp1);
    double csig12 = sbet1 * sbet2 + cbet1 * cbet2 * comg12;
    if (shortline && ssig12 < etol2)
    {
        salp2 = cbet1 * somg12;
        calp2 = sbet12 - cbet1 * sbet2 *
            (comg12 >= 0 ? sqr(somg12) / (1 + comg12) : 1 - comg12);
        double t = hypot(salp2, calp2);
        salp2 /= t; calp2 /= t;
        sig12 = atan2(ssig12, csig12);
    }
    else if (!(fabs(N) > 0.1 || csig12 >= 0 || ssig12 >= 6 * fabs(N) * pi * sqr(cbet1)))
    {
        double y, lamscale, betscale;
        double x;
        if (F >= 0)
        {
            double k2 = sqr(sbet1) * E2sq;
            double eps = k2 / (2 * (1 + sqrt(1 + k2)) + k2);
            lamscale = F * cbet1 * A3f(eps) * pi;
            betscale = lamscale * cbet1;
            x = (lam12 - pi) / lamscale;
            y = sbet12a / betscale;
        }
        else
        {
            double cbet12a = cbet2 * cbet1 - sbet2 * sbet1;
            double bet12a = atan2(sbet12a, cbet12a);
            double m12b, m0;
            double dummy;
            lengths(N, pi + bet12a,
                sbet1, -cbet1, dn1, sbet2, cbet2, dn2,
                dummy, m12b, m0, C1a, C2a);
            x = -1 + m12b / (cbet1 * cbet2 * m0 * pi);
            betscale = x < -0.01 ? sbet12a / x:
                - F * sqr(cbet1) * pi;
            lamscale = betscale / cbet1;
            y = (lam12 - pi) / lamscale;
        }
        if (y > -tol1 && x > -1 - 1000 * tol2)
        {
            if (F >= 0)
            {
              salp1 = std::min(1.0, -x);
              calp1 = - sqrt(1 - sqr(salp1));
            }
            else
            {
              calp1 = std::max(x > -tol1 ? 0.0 : -1.0, (double)x);
              salp1 = sqrt(1 - sqr(calp1));
            }
        }
        else
        {
            double k = astroid(x, y);
            double omg12a = lamscale * (F >= 0 ? -x*k/(1+k) : -y*(1+k)/k);
            somg12 = sin(omg12a); comg12 = -cos(omg12a);
            salp1 = cbet2 * somg12;
            calp1 = sbet12a - cbet2 * sbet1 * sqr(somg12) / (1 - comg12);
        }
    }
    if (salp1 > 0)
    {
        double t = hypot(salp1, calp1);
        salp1 /= t; calp1 /= t;
    }
    else
    {
        salp1 = 1; calp1 = 0;
    }
    return sig12;
}

inline double astroid(double x, double y) const
{
    double k;
    double p = sqr(x);
    double q = sqr(y);
    double r = (p + q - 1) / 6;
    if (!(q == 0 && r <= 0))
    {
        double S = p * q/4;
        double r2 = sqr(r);
        double r3 = r * r2;
        double disc = S * (S + 2 * r3);
        double u=r;
        if (disc>=0)
        {
            double T3 = S + r3;
            T3 += T3 < 0 ? -sqrt(disc) : sqrt(disc);
            double T = boost::math::cbrt(T3);
            u += T + (T != 0 ? r2 / T : 0);
        }
        else
        {
            double ang = atan2(sqrt(-disc), -(S + r3));
            u += 2 * r * cos(ang/3);
        }
        double v = sqrt(sqr(u)+q);
        double uv = u < 0 ? q / (v - u) : u + v;
        double w = (uv - q) / (2 * v);
        k = uv / (sqrt(uv + sqr(w)) + w);
    }
    else
    {
        k=0;
    }
    return k;
}

inline double Lambda12(double sbet1, double cbet1, double dn1,
            double sbet2, double cbet2, double dn2,
            double salp1, double calp1,
            double& salp2, double& calp2,
            double& sig12,
            double& ssig1, double& csig1,
            double& ssig2, double& csig2,
            double& eps, double& domg12,
            double& dlam12,
            double C1a[], double C2a[], double C3a[]) const
{
    if (sbet1 == 0 && calp1 == 0)
    {
        // Break degeneracy of equatorial line.  This case has already been
        // handled.
        calp1 = -tiny;
    }
    double salp0 = salp1 * cbet1;
    double calp0 = hypot(calp1, salp1 * sbet1); // calp0 > 0

    double somg1 = salp0 * sbet1, comg1 = calp1 * cbet1;
    ssig1 = sbet1; csig1 = comg1;
    double t1 = hypot(ssig1, csig1);
    ssig1 /= t1; csig1 /= t1;

    salp2 = cbet2 != cbet1 ? salp0 / cbet2 : salp1;
    calp2 = cbet2 != cbet1 || fabs(sbet2) != -sbet1 ?
        sqrt(sqr(calp1 * cbet1) +
         (cbet1 < -sbet1 ?
          (cbet2 - cbet1) * (cbet1 + cbet2) :
          (sbet1 - sbet2) * (sbet1 + sbet2))) / cbet2 :
        abs(calp1);
    double somg2 = salp0 * sbet2, comg2 = calp2 * cbet2;
    ssig2 = sbet2, csig2 = comg2;

    double t2 = hypot(ssig2, csig2);
    ssig2 /= t2; csig2 /= t2;

    sig12 = atan2(std::max(csig1 * ssig2 - ssig1 * csig2, 0.0),
                csig1 * csig2 + ssig1 * ssig2);

    double omg12 = atan2(std::max(comg1 * somg2 - somg1 * comg2, 0.0),
                comg1 * comg2 + somg1 * somg2);
    double B312, h0;
    double k2 = sqr(calp0) * E2sq;
    eps = k2 / (2 * (1 + sqrt(1 + k2)) + k2);
    C3f(eps, C3a);
    B312 = (sinCosSeries(true, ssig2, csig2, C3a, PRECISION - 1) -
          sinCosSeries(true, ssig1, csig1, C3a, PRECISION - 1));
    h0 = -F * A3f(eps);
    domg12 = salp0 * h0 * (sig12 + B312);
    double lam12 = omg12 + domg12;

    if (calp2 == 0)
    {
        dlam12 = - 2 * (1.0 - F) * dn1 / sbet1;
    }
    else
    {
        double dummy;
        lengths(eps, sig12, ssig1, csig1, dn1, ssig2, csig2, dn2,
            dummy, dlam12, dummy, C1a, C2a);
        dlam12 *= (1.0 - F) / (calp2 * cbet2);
    }
   
    return lam12;
}

static geoid WGS84()
{
    static geoid wgs84geoid(wgs84::Rbig, wgs84::F);
    return wgs84geoid;
}

const static size_t PRECISION = 3;
const static size_t MAXIT1 = 10;
const static size_t MAXIT2 = MAXIT1 + std::numeric_limits<double>::digits + 10;

const double Rbig;
const double Rsmall;
const double F;
const double Esq;
const double E2sq;
const double N;
const double Csq;

const double tiny;
const double tol0;
const double tol1;
const double tol2;
const double tolb;
const double etol2;

double A3x[PRECISION];
double C3x[PRECISION * (PRECISION - 1) / 2];
double C4x[PRECISION * (PRECISION + 1) / 2];

};


inline polar_point_2 geo_inverse(geo_point_2 const& a, geo_point_2 const& b)
{
    return geoid::WGS84().inverse(a, b);
}

inline geo_point_2 geo_direct(geo_point_2 const & p, polar_point_2 const& offset)
{
    return geoid::WGS84().direct(p, offset);
}

}

}
