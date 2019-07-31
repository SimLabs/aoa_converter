#pragma once
#ifndef _MATHLIB_DOUBLE_H_
#define _MATHLIB_DOUBLE_H_

//======================================================================================================================
//														YMM
//======================================================================================================================

const double c_dEps										= 1.11022302462515654042e-16; // pow(2, -52)
const double c_dInf										= -log(0.0);

const v4d c_ymmInf										= _mm256_set1_pd(c_dInf);
const v4d c_ymmInfMinus									= _mm256_set1_pd(-c_dInf);
const v4d c_ymm0001										= _mm256_setr_pd(0.0, 0.0, 0.0, 1.0);
const v4d c_ymm1111										= _mm256_set1_pd(1.0);
const v4d c_ymmSign										= _mm256_castsi256_pd(_mm256_set_epi32(0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000));
const v4d c_ymmFFF0										= _mm256_castsi256_pd(_mm256_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000));

#define ymm_mask_dp(xi, yi, zi, wi, xo, yo, zo, wo)		(xo | (yo << 1) | (zo << 2) | (wo << 3) | (xi << 4) | (yi << 5) | (zi << 6) | (wi << 7))
#define ymm_mask_dp4									ymm_mask_dp(1, 1, 1, 1, 1, 1, 1, 1)
#define ymm_mask_dp3									ymm_mask_dp(1, 1, 1, 0, 1, 1, 1, 1)

#define ymm_mask(x, y, z, w)							(x | (y << 1) | (z << 2) | (w << 3))
#define ymm_mask_x										ymm_mask(1, 0, 0, 0)
#define ymm_mask_xy										ymm_mask(1, 1, 0, 0)
#define ymm_mask_xyz									ymm_mask(1, 1, 1, 0)
#define ymm_mask_xyzw									ymm_mask(1, 1, 1, 1)

#define ymm_set_4f(x, y, z, w)							_mm256_setr_pd(x, y, z, w)
#define ymm_zero										_mm256_setzero_pd()
#define ymm_setw1(x)									_mm256_or_pd(_mm256_and_pd(x, c_ymmFFF0), c_ymm0001)
#define ymm_setw0(x)									_mm256_and_pd(x, c_ymmFFF0)

#define ymm_test1_none(v)								((_mm256_movemask_pd(v) & ymm_mask_x) == 0)
#define ymm_test1_all(v)								((_mm256_movemask_pd(v) & ymm_mask_x) != 0)

#define ymm_bits3(v)									(_mm256_movemask_pd(v) & ymm_mask_xyz)
#define ymm_test3(v, x, y, z)							(ymm_bits3(v) == ymm_mask(x, y, z, 0))
#define ymm_test3_all(v)								(ymm_bits3(v) == ymm_mask_xyz)
#define ymm_test3_none(v)								(ymm_bits3(v) == 0)
#define ymm_test3_any(v)								(ymm_bits3(v) != 0)

#define ymm_bits4(v)									_mm256_movemask_pd(v)
#define ymm_test4(v, x, y, z, w)						(ymm_bits4(v) == ymm_mask(x, y, z, w))
#define ymm_test4_all(v)								(ymm_bits4(v) == ymm_mask_xyzw)
#define ymm_test4_none(v)								(ymm_bits4(v) == 0)
#define ymm_test4_any(v)								(ymm_bits4(v) != 0)

// NOTE: < 0

#define ymm_isnegative1_all(v)							ymm_test1_all(v)
#define ymm_isnegative3_all(v)							ymm_test3_all(v)
#define ymm_isnegative4_all(v)							ymm_test4_all(v)

// NOTE: >= 0

#define ymm_ispositive1_all(v)							ymm_test1_none(v)
#define ymm_ispositive3_all(v)							ymm_test3_none(v)
#define ymm_ispositive4_all(v)							ymm_test4_none(v)

#define ymm_swizzle(v, x, y, z, w)						_mm256_permute4x64_pd(v, _MM_SHUFFLE(w, z, y, x))
#define ymm_shuffle(v0, v1, i0, j0, i1, j1)				_mm256_blend_pd(_mm256_permute4x64_pd(v0, _MM_SHUFFLE(j1, i1, j0, i0)), _mm256_permute4x64_pd(v1, _MM_SHUFFLE(j1, i1, j0, i0)), 0xC)

#define ymm_Azw_Bzw(a, b)								_mm256_permute2f128_pd(a, b, (1 << 4) | 3)
#define ymm_Axy_Bxy(a, b)								_mm256_permute2f128_pd(a, b, (2 << 4) | 0)
#define ymm_Ay_By_Aw_Bw(a, b)							_mm256_unpackhi_pd(a, b)
#define ymm_Ax_Bx_Az_Bz(a, b)							_mm256_unpacklo_pd(a, b)

#define ymm_get_x(x)									_mm_cvtsd_f64( _mm256_extractf128_pd(x, 0) )
#define ymm_store_x(ptr, x)								_mm_storel_pd(ptr, _mm256_extractf128_pd(x, 0))

#define ymm_madd(a, b, c)								_mm256_fmadd_pd(a, b, c)
#define ymm_msub(a, b, c)								_mm256_fmsub_pd(a, b, c)
#define ymm_nmadd(a, b, c)								_mm256_fnmadd_pd(a, b, c)

#define ymm_negate_comp(v, x, y, z, w)					_mm256_xor_pd(v, _mm256_castsi256_pd(_mm256_setr_epi32(x ? 0x80000000 : 0, 0, y ? 0x80000000 : 0, 0, z ? 0x80000000 : 0, 0, w ? 0x80000000 : 0, 0)))
#define ymm_negate(v)									_mm256_xor_pd(v, c_ymmSign)
#define ymm_abs(v)										_mm256_andnot_pd(c_ymmSign, v)

#define ymm_greater(a, b)								_mm256_cmp_pd(a, b, _CMP_GT_OQ)
#define ymm_less(a, b)									_mm256_cmp_pd(a, b, _CMP_LT_OQ)
#define ymm_gequal(a, b)								_mm256_cmp_pd(a, b, _CMP_GE_OQ)
#define ymm_lequal(a, b)								_mm256_cmp_pd(a, b, _CMP_LE_OQ)
#define ymm_equal(a, b)									_mm256_cmp_pd(a, b, _CMP_EQ_OQ)
#define ymm_notequal(a, b)								_mm256_cmp_pd(a, b, _CMP_NEQ_UQ)

#define ymm_greater0_all(a)								ymm_test4_all( ymm_greater(a, ymm_zero) )
#define ymm_gequal0_all(a)								ymm_test4_all( ymm_gequal(a, ymm_zero) )
#define ymm_not0_all(a)									ymm_test4_all( ymm_notequal(a, ymm_zero) )

#define ymm_rsqrt_(a)									_mm256_div_pd(c_ymm1111, _mm256_sqrt_pd(a))
#define ymm_rcp_(a)										_mm256_div_pd(c_ymm1111, a)

#define ymm_round(x)									_mm256_round_pd(x, _MM_FROUND_TO_NEAREST_INT | ROUNDING_EXEPTIONS_MASK)
#define ymm_floor(x)									_mm256_round_pd(x, _MM_FROUND_FLOOR | ROUNDING_EXEPTIONS_MASK)
#define ymm_ceil(x)										_mm256_round_pd(x, _MM_FROUND_CEIL | ROUNDING_EXEPTIONS_MASK)

#define ymm_select(a, b, mask)							_mm256_blendv_pd(a, b, mask)

#define ymm_to_xmm(a)									_mm256_cvtpd_ps(a);

// IMPORTANT: use it in some strange cases...
#define ymm_zero_upper									_mm256_zeroupper
#define ymm_zero_all									_mm256_zeroall

#ifdef MATH_CHECK_W_IS_ZERO

	ENGINE_INLINE bool ymm_is_w_zero(const v4d& x)
	{
		v4d t = ymm_equal(x, ymm_zero);

		return (ymm_bits4(t) & ymm_mask(0, 0, 0, 1)) == ymm_mask(0, 0, 0, 1);
	}

#else

	#define ymm_is_w_zero(x)		(true)

#endif

ENGINE_INLINE v4d ymm_dot33(const v4d& a, const v4d& b)
{
	DEBUG_Assert( ymm_is_w_zero(a) && ymm_is_w_zero(b) );

	v4d r = _mm256_mul_pd(a, b);
	r = ymm_setw0(r);
	r = _mm256_hadd_pd(r, _mm256_permute2f128_pd(r, r, (0 << 4) | 3));
	r = _mm256_hadd_pd(r, r);

	return r;
}

ENGINE_INLINE v4d ymm_dot44(const v4d& a, const v4d& b)
{
	v4d r = _mm256_mul_pd(a, b);
	r = _mm256_hadd_pd(r, _mm256_permute2f128_pd(r, r, (0 << 4) | 3));
	r = _mm256_hadd_pd(r, r);

	return r;
}

ENGINE_INLINE v4d ymm_dot43(const v4d& a, const v4d& b)
{
	DEBUG_Assert( ymm_is_w_zero(b) );

	v4d r = ymm_setw1(b);
	r = _mm256_mul_pd(a, r);
	r = _mm256_hadd_pd(r, _mm256_permute2f128_pd(r, r, (0 << 4) | 3));
	r = _mm256_hadd_pd(r, r);

	return r;
}

ENGINE_INLINE v4d ymm_cross(const v4d& x, const v4d& y)
{
	v4d a = ymm_swizzle(x, 1, 2, 0, 3);
	v4d b = ymm_swizzle(y, 2, 0, 1, 3);
	v4d c = ymm_swizzle(x, 2, 0, 1, 3);
	v4d d = ymm_swizzle(y, 1, 2, 0, 3);

	c = _mm256_mul_pd(c, d);

	return ymm_msub(a, b, c);
}

ENGINE_INLINE v4d ymm_sqrt(const v4d& r)
{
	DEBUG_Assert( ymm_gequal0_all(r) );

	return _mm256_sqrt_pd(r);
}

ENGINE_INLINE v4d ymm_rsqrt(const v4d& r)
{
	DEBUG_Assert( ymm_greater0_all(r) );

	#ifdef MATH_NEWTONRAPHSON_APROXIMATION

		v4d c = ymm_rsqrt_(r);
		v4d a = _mm256_mul_pd(c, _mm256_set1_pd(0.5));
		v4d t = _mm256_mul_pd(r, c);
		v4d b = ymm_nmadd(t, c, _mm256_set1_pd(3.0));
		
		return _mm256_mul_pd(a, b);

	#else

		return ymm_rsqrt_(r);

	#endif
}

ENGINE_INLINE v4d ymm_rcp(const v4d& r)
{
	DEBUG_Assert( ymm_not0_all(r) );

	#ifdef MATH_NEWTONRAPHSON_APROXIMATION

		v4d c = ymm_rcp_(r);
		v4d a = _mm256_mul_pd(c, r);
		v4d b = _mm256_add_pd(c, c);

		return ymm_nmadd(a, c, b);

	#else

		return ymm_rcp_(r);

	#endif
}

ENGINE_INLINE v4d ymm_sign(const v4d& x)
{
	// NOTE: 1 for +0, -1 for -0

	v4d v = _mm256_and_pd(x, c_ymmSign);

	return _mm256_or_pd(v, c_ymm1111);
}

ENGINE_INLINE v4d ymm_fract(const v4d& x)
{
	v4d flr0 = ymm_floor(x);
	v4d sub0 = _mm256_sub_pd(x, flr0);

	return sub0;
}

ENGINE_INLINE v4d ymm_mod(const v4d& x, const v4d& y)
{
	v4d div = _mm256_div_pd(x, y);
	v4d flr = ymm_floor(div);

	return ymm_nmadd(y, flr, x);
}

ENGINE_INLINE v4d ymm_clamp(const v4d& x, const v4d& vmin, const v4d& vmax)
{
	v4d min0 = _mm256_min_pd(x, vmax);

	return _mm256_max_pd(min0, vmin);
}

ENGINE_INLINE v4d ymm_saturate(const v4d& x)
{
	v4d min0 = _mm256_min_pd(x, c_ymm1111);

	return _mm256_max_pd(min0, ymm_zero);
}

ENGINE_INLINE v4d ymm_mix(const v4d& a, const v4d& b, const v4d& x)
{
	v4d sub0 = _mm256_sub_pd(b, a);

	return ymm_madd(sub0, x, a);
}

ENGINE_INLINE v4d ymm_step(const v4d& edge, const v4d& x)
{
	v4d cmp = ymm_gequal(x, edge);

	return _mm256_and_pd(c_ymm1111, cmp);
}

ENGINE_INLINE v4d ymm_linearstep(const v4d& edge0, const v4d& edge1, const v4d& x)
{
	v4d sub0 = _mm256_sub_pd(x, edge0);
	v4d sub1 = _mm256_sub_pd(edge1, edge0);
	v4d div0 = _mm256_div_pd(sub0, sub1);

	return ymm_saturate(div0);
}

ENGINE_INLINE v4d ymm_smoothstep(const v4d& edge0, const v4d& edge1, const v4d& x)
{
	v4d b = ymm_linearstep(edge0, edge1, x);
	v4d c = ymm_nmadd(_mm256_set1_pd(2.0), b, _mm256_set1_pd(3.0));
	v4d t = _mm256_mul_pd(b, b);

	return _mm256_mul_pd(t, c);
}

ENGINE_INLINE v4d ymm_normalize(const v4d& x)
{
	v4d r = ymm_dot33(x, x);
	r = ymm_rsqrt(r);

	return _mm256_mul_pd(x, r);
}

ENGINE_INLINE v4d ymm_length(const v4d& x)
{
	v4d r = ymm_dot33(x, x);

	return _mm256_sqrt_pd(r);
}

#ifndef ENGINE_INTEL_COMPILER

	// IMPORTANT: use Intel SVML compatible names

	v4d _mm256_sin_pd(const v4d& x);
	v4d _mm256_cos_pd(const v4d& x);
	v4d _mm256_sincos_pd(v4d* pCos, const v4d& d);
	v4d _mm256_tan_pd(const v4d& x);
	v4d _mm256_atan_pd(const v4d& d);
	v4d _mm256_atan2_pd(const v4d& y, const v4d& x);
	v4d _mm256_asin_pd(const v4d& d);
	v4d _mm256_acos_pd(const v4d& d);
	v4d _mm256_log_pd(const v4d& d);
	v4d _mm256_exp_pd(const v4d& d);

	ENGINE_INLINE v4d _mm256_pow_pd(const v4d& x, const v4d& y)
	{
		v4d t = _mm256_log_pd(x);
		t = _mm256_mul_pd(t, y);

		return _mm256_exp_pd(t);
	}

#endif

//======================================================================================================================
//														double3
//======================================================================================================================

class double3
{
	public:

		static const double3 ortx;
		static const double3 orty;
		static const double3 ortz;
		static const double3 one;

	public:

		union
		{
			struct
			{
				v4d ymm;
			};

			struct
			{
				double v[COORD_3D];
			};

			struct
			{
				double x, y, z;
			};
		};

	public:

		// NOTE: constructors

		ENGINE_INLINE double3()
		{
		}

		ENGINE_INLINE double3(double a, double b, double c) : ymm( ymm_set_4f(a, b, c, 0.0) )
		{
		}

		ENGINE_INLINE double3(double a) : ymm( _mm256_broadcast_sd(&a) )
		{
		}

		ENGINE_INLINE double3(const double* v3) : ymm( ymm_set_4f(v3[0], v3[1], v3[2], 0.0) )
		{
		}

		ENGINE_INLINE double3(const double3& vec) : ymm( vec.ymm )
		{
		}

		ENGINE_INLINE double3(const v4d& m) : ymm(m)
		{
		}

		// FIXME: used only for template cExpression instantiation

		ENGINE_INLINE double3(uint a) : ymm( _mm256_set1_pd(double(a)) )
		{
		}

		// NOTE: set

		ENGINE_INLINE void Set0()
		{
			ymm = ymm_zero;
		}

		ENGINE_INLINE void Set(const double* v3)
		{
			DEBUG_Assert(v3);

			ymm = ymm_set_4f(v3[0], v3[1], v3[2], 0.0);
		}

		ENGINE_INLINE void Set(const double3& v)
		{
			Set(v.ymm);
		}

		ENGINE_INLINE void Set(double a, double b, double c)
		{
			ymm = ymm_set_4f(a, b, c, 0.0);
		}

		ENGINE_INLINE void Set(double a)
		{
			ymm = _mm256_broadcast_sd(&a);
		}

		ENGINE_INLINE void Set(const v4d& m)
		{
			ymm = m;
		}

		ENGINE_INLINE void operator = (const double3& vec)
		{
			Set(vec);
		}

		// NOTE: compare

		ENGINE_INLINE bool operator == (const double3& v) const
		{
			v4d r = ymm_equal(ymm, v.ymm);

			return ymm_test3_all(r);
		}

		ENGINE_INLINE bool operator != (const double3& v) const
		{
			v4d r = ymm_notequal(ymm, v.ymm);

			return ymm_test3_any(r);
		}

		// NOTE: arithmetic

		ENGINE_INLINE double3 operator - () const
		{
			return ymm_negate(ymm);
		}

		ENGINE_INLINE double3 operator + (const double3& v) const
		{
			return _mm256_add_pd(ymm, v.ymm);
		}

		ENGINE_INLINE double3 operator - (const double3& v) const
		{
			return _mm256_sub_pd(ymm, v.ymm);
		}

		ENGINE_INLINE double3 operator * (const double3& v) const
		{
			return _mm256_mul_pd(ymm, v.ymm);
		}

		ENGINE_INLINE double3 operator / (const double3& v) const
		{
			DEBUG_Assert( ymm_not0_all(v.ymm) );

			return _mm256_div_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator += (const double3& v)
		{
			ymm = _mm256_add_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator -= (const double3& v)
		{
			ymm = _mm256_sub_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator *= (const double3& v)
		{
			ymm = _mm256_mul_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator /= (const double3& v)
		{
			DEBUG_Assert( ymm_not0_all(v.ymm) );

			ymm = _mm256_div_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator *= (double s)
		{
			ymm = _mm256_mul_pd(ymm, _mm256_broadcast_sd(&s));
		}

		ENGINE_INLINE void operator /= (double s)
		{
			DEBUG_Assert( s != 0.0 );

			ymm = _mm256_div_pd(ymm, _mm256_broadcast_sd(&s));
		}

		ENGINE_INLINE double3 operator / (double s) const
		{
			DEBUG_Assert( s != 0.0 );

			return _mm256_div_pd(ymm, _mm256_broadcast_sd(&s));
		}

		ENGINE_INLINE double3 operator * (double s) const
		{
			return _mm256_mul_pd(ymm, _mm256_broadcast_sd(&s));
		}

		// NOTE: misc

		ENGINE_INLINE bool IsZero() const
		{
			v4d r = ymm_equal(ymm, ymm_zero);

			return ymm_test3_all(r);
		}

		// NOTE: swizzle

		ENGINE_INLINE double3 xxx() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_X, 0); }
		ENGINE_INLINE double3 xxy() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Y, 0); }
		ENGINE_INLINE double3 xxz() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Z, 0); }
		ENGINE_INLINE double3 xyx() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_X, 0); }
		ENGINE_INLINE double3 xyy() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Y, 0); }
		ENGINE_INLINE double3 xyz() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Z, 0); }
		ENGINE_INLINE double3 xzx() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_X, 0); }
		ENGINE_INLINE double3 xzy() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Y, 0); }
		ENGINE_INLINE double3 xzz() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Z, 0); }
		ENGINE_INLINE double3 yxx() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_X, 0); }
		ENGINE_INLINE double3 yxy() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Y, 0); }
		ENGINE_INLINE double3 yxz() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Z, 0); }
		ENGINE_INLINE double3 yyx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_X, 0); }
		ENGINE_INLINE double3 yyy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Y, 0); }
		ENGINE_INLINE double3 yyz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Z, 0); }
		ENGINE_INLINE double3 yzx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_X, 0); }
		ENGINE_INLINE double3 yzy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Y, 0); }
		ENGINE_INLINE double3 yzz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Z, 0); }
		ENGINE_INLINE double3 zxx() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_X, 0); }
		ENGINE_INLINE double3 zxy() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Y, 0); }
		ENGINE_INLINE double3 zxz() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Z, 0); }
		ENGINE_INLINE double3 zyx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_X, 0); }
		ENGINE_INLINE double3 zyy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Y, 0); }
		ENGINE_INLINE double3 zyz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Z, 0); }
		ENGINE_INLINE double3 zzx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_X, 0); }
		ENGINE_INLINE double3 zzy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Y, 0); }
		ENGINE_INLINE double3 zzz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Z, 0); }
};

ENGINE_INLINE double Dot33(const double3& a, const double3& b)
{
	v4d r = ymm_dot33(a.ymm, b.ymm);

	return ymm_get_x(r);
}

ENGINE_INLINE double LengthSquared(const double3& x)
{
	v4d r = ymm_dot33(x.ymm, x.ymm);

	return ymm_get_x(r);
}

ENGINE_INLINE double Length(const double3& x)
{
	v4d r = ymm_length(x.ymm);

	return ymm_get_x(r);
}

ENGINE_INLINE double3 Normalize(const double3& x)
{
	return ymm_normalize(x.ymm);
}

ENGINE_INLINE double3 Cross(const double3& x, const double3& y)
{
	return ymm_cross(x.ymm, y.ymm);
}

ENGINE_INLINE double3 Rcp(const double3& x)
{
	return ymm_rcp(x.ymm);
}

ENGINE_INLINE double3 Ceil(const double3& x)
{
	return ymm_ceil(x.ymm);
}

ENGINE_INLINE double3 Madd(const double3& a, const double3& b, const double3& c)
{
	return ymm_madd(a.ymm, b.ymm, c.ymm);
}

// NOTE: build basis from normal vector
// IMPORTANT: must be synchronized with GLSL version in std_lib.glsl

ENGINE_INLINE void BasisFromDirection_carmack(const double3& N, double3& T, double3& B)
{
	T.Set(N.z, -N.x, N.y);
	T -= N * Dot33(T, N);
	T = Normalize(T);
	
	B = Cross(N, T);
}

ENGINE_INLINE void BasisFromDirection_frisvad(const double3& N, double3& T, double3& B)
{
	/*
	// ORIGINAL:
	if( N.z < -0.9999999999 )
	{
		T.Set(0.0, -1.0, 0.0);
		B.Set(-1.0, 0.0, 0.0);

		return;
	}
	*/

	DEBUG_Assert( N.z > -0.9999999999 );

	double a = 1.0 / (1.0 + N.z);
	double3 c = double3(0.0, 1.0, 1.0) - N.yxy() * N.xxy() * a;

	T.Set(c.y, c.x, -N.x);
	B.Set(c.x, c.z, -N.y);
}

//======================================================================================================================
//														double4
//======================================================================================================================

class double4
{
	public:

		union
		{
			struct
			{
				v4d ymm;
			};

			struct
			{
				double v[COORD_4D];
			};

			struct
			{
				double x, y, z, w;
			};
		};

	public:

		// NOTE: constructors

		ENGINE_INLINE double4()
		{
		}
		
		ENGINE_INLINE double4(double a) : ymm( _mm256_broadcast_sd(&a) )
		{
		}
		
		ENGINE_INLINE double4(double a, double b, double c, double d) : ymm( ymm_set_4f(a, b, c, d) )
		{
		}

		ENGINE_INLINE double4(double a, double b, double c) : ymm( ymm_set_4f(a, b, c, 1.0) )
		{
		}

		ENGINE_INLINE double4(const double* v4) : ymm( _mm256_loadu_pd(v4) )
		{
		}

		ENGINE_INLINE double4(const double4& vec) : ymm( vec.ymm )
		{
		}

		ENGINE_INLINE double4(const v4d& m) : ymm(m)
		{
		}

		// NOTE: set

		ENGINE_INLINE void Set0001()
		{
			ymm = c_ymm0001;
		}

		ENGINE_INLINE void Set0()
		{
			ymm = ymm_zero;
		}

		ENGINE_INLINE void Set(double a, double b, double c, double d = 1.0)
		{
			ymm = ymm_set_4f(a, b, c, d);
		}

		ENGINE_INLINE void Set(const double* v4)
		{
			DEBUG_Assert(v4);

			ymm = _mm256_loadu_pd(v4);
		}

		ENGINE_INLINE void Set(const double4& v)
		{
			ymm = v.ymm;
		}

		ENGINE_INLINE void Set(const double3& v)
		{
			ymm = ymm_setw1(v.ymm);
		}

		ENGINE_INLINE void Set(const v4d& m)
		{
			ymm = m;
		}

		ENGINE_INLINE void operator = (const double4& vec)
		{
			Set(vec);
		}

		// NOTE: compare

		ENGINE_INLINE bool operator == (const double4& v) const
		{
			v4d r = ymm_equal(ymm, v.ymm);

			return ymm_test4_all(r);
		}

		ENGINE_INLINE bool operator != (const double4& v) const
		{
			v4d r = ymm_notequal(ymm, v.ymm);

			return ymm_test4_any(r);
		}

		// NOTE: arithmetic

		ENGINE_INLINE double4 operator - () const
		{
			return ymm_negate(ymm);
		}

		ENGINE_INLINE double4 operator + (const double4& v) const
		{
			return _mm256_add_pd(ymm, v.ymm);
		}

		ENGINE_INLINE double4 operator - (const double4& v) const
		{
			return _mm256_sub_pd(ymm, v.ymm);
		}

		ENGINE_INLINE double4 operator * (const double4& v) const
		{
			return _mm256_mul_pd(ymm, v.ymm);
		}

		ENGINE_INLINE double4 operator / (const double4& v) const
		{
			DEBUG_Assert( ymm_not0_all(v.ymm) );

			return _mm256_div_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator += (const double4& v)
		{
			ymm = _mm256_add_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator -= (const double4& v)
		{
			ymm = _mm256_sub_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator *= (const double4& v)
		{
			ymm = _mm256_mul_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator /= (const double4& v)
		{
			DEBUG_Assert( ymm_not0_all(v.ymm) );

			ymm = _mm256_div_pd(ymm, v.ymm);
		}

		ENGINE_INLINE void operator *= (double s)
		{
			ymm = _mm256_mul_pd(ymm, _mm256_broadcast_sd(&s));
		}

		ENGINE_INLINE void operator /= (double s)
		{
			DEBUG_Assert( s != 0.0 );

			ymm = _mm256_div_pd(ymm, _mm256_broadcast_sd(&s));
		}

		ENGINE_INLINE double4 operator / (double s) const
		{
			DEBUG_Assert( s != 0.0 );

			return _mm256_div_pd(ymm, _mm256_broadcast_sd(&s));
		}

		ENGINE_INLINE double4 operator * (double s) const
		{
			return _mm256_mul_pd(ymm, _mm256_broadcast_sd(&s));
		}

		// NOTE: misc

		ENGINE_INLINE double3 To3d() const
		{
			return ymm;
		}

		ENGINE_INLINE bool IsZero() const
		{
			v4d r = ymm_equal(ymm, ymm_zero);

			return ymm_test4_all(r);
		}

		// NOTE: swizzle

		ENGINE_INLINE double4 xxxx() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE double4 xxxy() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 xxxz() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 xxxw() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE double4 xxyx() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 xxyy() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 xxyz() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 xxyw() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 xxzx() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 xxzy() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 xxzz() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 xxzw() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 xxwx() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE double4 xxwy() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 xxwz() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 xxww() const { return ymm_swizzle(ymm, COORD_X, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE double4 xyxx() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE double4 xyxy() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 xyxz() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 xyxw() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE double4 xyyx() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 xyyy() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 xyyz() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 xyyw() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 xyzx() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 xyzy() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 xyzz() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 xyzw() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 xywx() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE double4 xywy() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 xywz() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 xyww() const { return ymm_swizzle(ymm, COORD_X, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE double4 xzxx() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE double4 xzxy() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 xzxz() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 xzxw() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE double4 xzyx() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 xzyy() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 xzyz() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 xzyw() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 xzzx() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 xzzy() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 xzzz() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 xzzw() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 xzwx() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE double4 xzwy() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 xzwz() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 xzww() const { return ymm_swizzle(ymm, COORD_X, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE double4 xwxx() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE double4 xwxy() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 xwxz() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 xwxw() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE double4 xwyx() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 xwyy() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 xwyz() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 xwyw() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 xwzx() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 xwzy() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 xwzz() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 xwzw() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 xwwx() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE double4 xwwy() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 xwwz() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 xwww() const { return ymm_swizzle(ymm, COORD_X, COORD_W, COORD_W, COORD_W); }
		ENGINE_INLINE double4 yxxx() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE double4 yxxy() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 yxxz() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 yxxw() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE double4 yxyx() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 yxyy() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 yxyz() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 yxyw() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 yxzx() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 yxzy() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 yxzz() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 yxzw() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 yxwx() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE double4 yxwy() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 yxwz() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 yxww() const { return ymm_swizzle(ymm, COORD_Y, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE double4 yyxx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE double4 yyxy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 yyxz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 yyxw() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE double4 yyyx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 yyyy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 yyyz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 yyyw() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 yyzx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 yyzy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 yyzz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 yyzw() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 yywx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE double4 yywy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 yywz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 yyww() const { return ymm_swizzle(ymm, COORD_Y, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE double4 yzxx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE double4 yzxy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 yzxz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 yzxw() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE double4 yzyx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 yzyy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 yzyz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 yzyw() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 yzzx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 yzzy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 yzzz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 yzzw() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 yzwx() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE double4 yzwy() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 yzwz() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 yzww() const { return ymm_swizzle(ymm, COORD_Y, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE double4 ywxx() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE double4 ywxy() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 ywxz() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 ywxw() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE double4 ywyx() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 ywyy() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 ywyz() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 ywyw() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 ywzx() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 ywzy() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 ywzz() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 ywzw() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 ywwx() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE double4 ywwy() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 ywwz() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 ywww() const { return ymm_swizzle(ymm, COORD_Y, COORD_W, COORD_W, COORD_W); }
		ENGINE_INLINE double4 zxxx() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE double4 zxxy() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 zxxz() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 zxxw() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE double4 zxyx() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 zxyy() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 zxyz() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 zxyw() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 zxzx() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 zxzy() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 zxzz() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 zxzw() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 zxwx() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE double4 zxwy() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 zxwz() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 zxww() const { return ymm_swizzle(ymm, COORD_Z, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE double4 zyxx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE double4 zyxy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 zyxz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 zyxw() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE double4 zyyx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 zyyy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 zyyz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 zyyw() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 zyzx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 zyzy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 zyzz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 zyzw() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 zywx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE double4 zywy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 zywz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 zyww() const { return ymm_swizzle(ymm, COORD_Z, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE double4 zzxx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE double4 zzxy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 zzxz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 zzxw() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE double4 zzyx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 zzyy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 zzyz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 zzyw() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 zzzx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 zzzy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 zzzz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 zzzw() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 zzwx() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE double4 zzwy() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 zzwz() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 zzww() const { return ymm_swizzle(ymm, COORD_Z, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE double4 zwxx() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE double4 zwxy() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 zwxz() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 zwxw() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE double4 zwyx() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 zwyy() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 zwyz() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 zwyw() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 zwzx() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 zwzy() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 zwzz() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 zwzw() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 zwwx() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE double4 zwwy() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 zwwz() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 zwww() const { return ymm_swizzle(ymm, COORD_Z, COORD_W, COORD_W, COORD_W); }
		ENGINE_INLINE double4 wxxx() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE double4 wxxy() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 wxxz() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 wxxw() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE double4 wxyx() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 wxyy() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 wxyz() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 wxyw() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 wxzx() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 wxzy() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 wxzz() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 wxzw() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 wxwx() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE double4 wxwy() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 wxwz() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 wxww() const { return ymm_swizzle(ymm, COORD_W, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE double4 wyxx() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE double4 wyxy() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 wyxz() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 wyxw() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE double4 wyyx() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 wyyy() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 wyyz() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 wyyw() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 wyzx() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 wyzy() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 wyzz() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 wyzw() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 wywx() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE double4 wywy() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 wywz() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 wyww() const { return ymm_swizzle(ymm, COORD_W, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE double4 wzxx() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE double4 wzxy() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 wzxz() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 wzxw() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE double4 wzyx() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 wzyy() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 wzyz() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 wzyw() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 wzzx() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 wzzy() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 wzzz() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 wzzw() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 wzwx() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE double4 wzwy() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 wzwz() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 wzww() const { return ymm_swizzle(ymm, COORD_W, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE double4 wwxx() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE double4 wwxy() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE double4 wwxz() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE double4 wwxw() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE double4 wwyx() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE double4 wwyy() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE double4 wwyz() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE double4 wwyw() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE double4 wwzx() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE double4 wwzy() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE double4 wwzz() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE double4 wwzw() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE double4 wwwx() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE double4 wwwy() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE double4 wwwz() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE double4 wwww() const { return ymm_swizzle(ymm, COORD_W, COORD_W, COORD_W, COORD_W); }
};

ENGINE_INLINE double Dot44(const double4& a, const double4& b)
{
	v4d r = ymm_dot44(a.ymm, b.ymm);

	return ymm_get_x(r);
}

ENGINE_INLINE double Dot43(const double4& a, const double3& b)
{
	v4d r = ymm_dot43(a.ymm, b.ymm);

	return ymm_get_x(r);
}

ENGINE_INLINE double4 Rcp(const double4& x)
{
	return ymm_rcp(x.ymm);
}

ENGINE_INLINE double4 Ceil(const double4& x)
{
	return ymm_ceil(x.ymm);
}

ENGINE_INLINE double4 Madd(const double4& a, const double4& b, const double4& c)
{
	return ymm_madd(a.ymm, b.ymm, c.ymm);
}

#endif
