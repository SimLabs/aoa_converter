#pragma once
#ifndef _MATHLIB_FLOAT_H_
#define _MATHLIB_FLOAT_H_

//======================================================================================================================
//														XMM
//======================================================================================================================

const float	c_fEps										= 1.1920928955078125e-7f; // pow(2, -23)
const float c_fInf										= -log(0.0f);

const v4f c_xmmInf										= _mm_set1_ps(c_fInf);
const v4f c_xmmInfMinus									= _mm_set1_ps(-c_fInf);
const v4f c_xmm0001										= _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
const v4f c_xmm1111										= _mm_set1_ps(1.0f);
const v4f c_xmmSign										= _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
const v4f c_xmmFFF0										= _mm_castsi128_ps(_mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000));

#define xmm_mask_dp(xi, yi, zi, wi, xo, yo, zo, wo)		(xo | (yo << 1) | (zo << 2) | (wo << 3) | (xi << 4) | (yi << 5) | (zi << 6) | (wi << 7))
#define xmm_mask_dp4									xmm_mask_dp(1, 1, 1, 1, 1, 1, 1, 1)
#define xmm_mask_dp3									xmm_mask_dp(1, 1, 1, 0, 1, 1, 1, 1)

#define xmm_mask(x, y, z, w)							(x | (y << 1) | (z << 2) | (w << 3))
#define xmm_mask_x										xmm_mask(1, 0, 0, 0)
#define xmm_mask_xy										xmm_mask(1, 1, 0, 0)
#define xmm_mask_xyz									xmm_mask(1, 1, 1, 0)
#define xmm_mask_xyzw									xmm_mask(1, 1, 1, 1)

#define xmm_set_4f(x, y, z, w)							_mm_setr_ps(x, y, z, w)
#define xmm_zero										_mm_setzero_ps()
#define xmm_setw1(x)									_mm_or_ps(_mm_and_ps(x, c_xmmFFF0), c_xmm0001)
#define xmm_setw0(x)									_mm_and_ps(x, c_xmmFFF0)

#define xmm_test1_none(v)								((_mm_movemask_ps(v) & xmm_mask_x) == 0)
#define xmm_test1_all(v)								((_mm_movemask_ps(v) & xmm_mask_x) != 0)

#define xmm_bits3(v)									(_mm_movemask_ps(v) & xmm_mask_xyz)
#define xmm_test3(v, x, y, z)							(xmm_bits3(v) == xmm_mask(x, y, z, 0))
#define xmm_test3_all(v)								(xmm_bits3(v) == xmm_mask_xyz)
#define xmm_test3_none(v)								(xmm_bits3(v) == 0)
#define xmm_test3_any(v)								(xmm_bits3(v) != 0)

#define xmm_bits4(v)									_mm_movemask_ps(v)
#define xmm_test4(v, x, y, z, w)						(xmm_bits4(v) == xmm_mask(x, y, z, w))
#define xmm_test4_all(v)								(xmm_bits4(v) == xmm_mask_xyzw)
#define xmm_test4_none(v)								(xmm_bits4(v) == 0)
#define xmm_test4_any(v)								(xmm_bits4(v) != 0)

// NOTE: < 0

#define xmm_isnegative1_all(v)							xmm_test1_all(v)
#define xmm_isnegative3_all(v)							xmm_test3_all(v)
#define xmm_isnegative4_all(v)							xmm_test4_all(v)

// NOTE: >= 0

#define xmm_ispositive1_all(v)							xmm_test1_none(v)
#define xmm_ispositive3_all(v)							xmm_test3_none(v)
#define xmm_ispositive4_all(v)							xmm_test4_none(v)

#define xmm_swizzle(v, x, y, z, w)						_mm_permute_ps(v, _MM_SHUFFLE(w, z, y, x))

#define xmm_shuffle(v0, v1, i0, j0, i1, j1)				_mm_shuffle_ps(v0, v1, _MM_SHUFFLE(j1, i1, j0, i0))

#define xmm_Azw_Bzw(a, b)								_mm_movehl_ps(a, b)
#define xmm_Axy_Bxy(a, b)								_mm_movelh_ps(a, b)
#define xmm_Ax_Byzw(a, b)								_mm_move_ss(b, a)
#define xmm_Az_Bz_Aw_Bw(a, b)							_mm_unpackhi_ps(a, b)
#define xmm_Ax_Bx_Ay_By(a, b)							_mm_unpacklo_ps(a, b)

#define xmm_get_x(x)									_mm_cvtss_f32(x)
#define xmm_store_x(ptr, x)								_mm_store_ss(ptr, x)

#define xmm_madd(a, b, c)								_mm_fmadd_ps(a, b, c)
#define xmm_msub(a, b, c)								_mm_fmsub_ps(a, b, c)
#define xmm_nmadd(a, b, c)								_mm_fnmadd_ps(a, b, c)

#define xmm_negate_comp(v, x, y, z, w)					_mm_xor_ps(v, _mm_castsi128_ps(_mm_setr_epi32(x ? 0x80000000 : 0, y ? 0x80000000 : 0, z ? 0x80000000 : 0, w ? 0x80000000 : 0)))
#define xmm_negate(v)									_mm_xor_ps(v, c_xmmSign)
#define xmm_abs(v)										_mm_andnot_ps(c_xmmSign, v)

#define xmm_greater(a, b)								_mm_cmpgt_ps(a, b)
#define xmm_less(a, b)									_mm_cmplt_ps(a, b)
#define xmm_gequal(a, b)								_mm_cmpge_ps(a, b)
#define xmm_lequal(a, b)								_mm_cmple_ps(a, b)
#define xmm_equal(a, b)									_mm_cmpeq_ps(a, b)
#define xmm_notequal(a, b)								_mm_cmpneq_ps(a, b)

#define xmm_greater0_all(a)								xmm_test4_all( xmm_greater(a, xmm_zero) )
#define xmm_gequal0_all(a)								xmm_test4_all( xmm_gequal(a, xmm_zero) )
#define xmm_not0_all(a)									xmm_test4_all( xmm_notequal(a, xmm_zero) )

#define xmm_rsqrt_(a)									_mm_rsqrt_ps(a)
#define xmm_rcp_(a)										_mm_rcp_ps(a)

#define xmm_hadd(a, b)									_mm_hadd_ps(a, b)
#define xmm_dot33(a, b)									_mm_dp_ps(a, b, xmm_mask_dp3)
#define xmm_dot44(a, b)									_mm_dp_ps(a, b, xmm_mask_dp4)
#define xmm_dot43(a, b)									_mm_dp_ps(a, xmm_setw1(b), xmm_mask_dp4)

#define xmm_round(x)									_mm_round_ps(x, _MM_FROUND_TO_NEAREST_INT | ROUNDING_EXEPTIONS_MASK)
#define xmm_floor(x)									_mm_round_ps(x, _MM_FROUND_FLOOR | ROUNDING_EXEPTIONS_MASK)
#define xmm_ceil(x)										_mm_round_ps(x, _MM_FROUND_CEIL | ROUNDING_EXEPTIONS_MASK)

#define xmm_select(a, b, mask)							_mm_blendv_ps(a, b, mask)

#define xmm_to_ymm(a)									_mm256_cvtps_pd(a)

#ifdef MATH_CHECK_W_IS_ZERO

	ENGINE_INLINE bool xmm_is_w_zero(const v4f& x)
	{
		v4f t = xmm_equal(x, xmm_zero);

		return (xmm_bits4(t) & xmm_mask(0, 0, 0, 1)) == xmm_mask(0, 0, 0, 1);
	}

#else

	#define xmm_is_w_zero(x)		(true)

#endif

ENGINE_INLINE v4f xmm_cross(const v4f& x, const v4f& y)
{
	v4f a = xmm_swizzle(x, 1, 2, 0, 3);
	v4f b = xmm_swizzle(y, 2, 0, 1, 3);
	v4f c = xmm_swizzle(x, 2, 0, 1, 3);
	v4f d = xmm_swizzle(y, 1, 2, 0, 3);
	
	c = _mm_mul_ps(c, d);

	return xmm_msub(a, b, c);
}

ENGINE_INLINE v4f xmm_sqrt(const v4f& r)
{
	DEBUG_Assert( xmm_gequal0_all(r) );

	return _mm_sqrt_ps(r);
}

ENGINE_INLINE v4f xmm_rsqrt(const v4f& r)
{
	DEBUG_Assert( xmm_greater0_all(r) );

	#ifdef MATH_NEWTONRAPHSON_APROXIMATION

		v4f c = xmm_rsqrt_(r);
		v4f a = _mm_mul_ps(c, _mm_set1_ps(0.5f));
		v4f t = _mm_mul_ps(r, c);
		v4f b = xmm_nmadd(t, c, _mm_set1_ps(3.0f));
		
		return _mm_mul_ps(a, b);

	#else

		return xmm_rsqrt_(r);

	#endif
}

ENGINE_INLINE v4f xmm_rcp(const v4f& r)
{
	DEBUG_Assert( xmm_not0_all(r) );

	#ifdef MATH_NEWTONRAPHSON_APROXIMATION

		v4f c = xmm_rcp_(r);
		v4f a = _mm_mul_ps(c, r);
		v4f b = _mm_add_ps(c, c);
		
		return xmm_nmadd(a, c, b);

	#else

		return xmm_rcp_(r);

	#endif
}

ENGINE_INLINE v4f xmm_sign(const v4f& x)
{
	// NOTE: 1 for +0, -1 for -0

	v4f v = _mm_and_ps(x, c_xmmSign);

	return _mm_or_ps(v, c_xmm1111);
}

ENGINE_INLINE v4f xmm_fract(const v4f& x)
{
	v4f flr0 = xmm_floor(x);
	v4f sub0 = _mm_sub_ps(x, flr0);

	return sub0;
}

ENGINE_INLINE v4f xmm_mod(const v4f& x, const v4f& y)
{
	v4f div = _mm_div_ps(x, y);
	v4f flr = xmm_floor(div);

	return xmm_nmadd(y, flr, x);
}

ENGINE_INLINE v4f xmm_clamp(const v4f& x, const v4f& vmin, const v4f& vmax)
{
	v4f min0 = _mm_min_ps(x, vmax);

	return _mm_max_ps(min0, vmin);
}

ENGINE_INLINE v4f xmm_saturate(const v4f& x)
{
	v4f min0 = _mm_min_ps(x, c_xmm1111);

	return _mm_max_ps(min0, xmm_zero);
}

ENGINE_INLINE v4f xmm_mix(const v4f& a, const v4f& b, const v4f& x)
{
	v4f sub0 = _mm_sub_ps(b, a);

	return xmm_madd(sub0, x, a);
}

ENGINE_INLINE v4f xmm_step(const v4f& edge, const v4f& x)
{
	v4f cmp = xmm_gequal(x, edge);

	return _mm_and_ps(c_xmm1111, cmp);
}

ENGINE_INLINE v4f xmm_linearstep(const v4f& edge0, const v4f& edge1, const v4f& x)
{
	v4f sub0 = _mm_sub_ps(x, edge0);
	v4f sub1 = _mm_sub_ps(edge1, edge0);
	v4f div0 = _mm_div_ps(sub0, sub1);

	return xmm_saturate(div0);
}

ENGINE_INLINE v4f xmm_smoothstep(const v4f& edge0, const v4f& edge1, const v4f& x)
{
	v4f b = xmm_linearstep(edge0, edge1, x);
	v4f c = xmm_nmadd(_mm_set1_ps(2.0f), b, _mm_set1_ps(3.0f));
	v4f t = _mm_mul_ps(b, b);
	
	return _mm_mul_ps(t, c);
}

ENGINE_INLINE v4f xmm_normalize(const v4f& x)
{
	v4f r = xmm_dot33(x, x);
	r = xmm_rsqrt(r);

	return _mm_mul_ps(x, r);
}

ENGINE_INLINE v4f xmm_length(const v4f& x)
{
	v4f r = xmm_dot33(x, x);

	return _mm_sqrt_ps(r);
}

ENGINE_INLINE v4i xmm_to_h4(const v4f& x)
{
	#pragma warning(push)
	#pragma warning(disable : 4556)

	return _mm_cvtps_ph(x, _MM_FROUND_TO_NEAREST_INT | ROUNDING_EXEPTIONS_MASK);

	#pragma warning(pop)
}

ENGINE_INLINE v4i xmmi_select(const v4i& x, const v4i& y, const v4i& mask)
{
	return _mm_or_si128(_mm_and_si128(mask, x), _mm_andnot_si128(mask, y));
}

// IMPORTANT: use Intel SVML compatible names

#ifndef ENGINE_INTEL_COMPILER

	v4f _mm_sin_ps(const v4f& x);
	v4f _mm_cos_ps(const v4f& x);
	v4f _mm_sincos_ps(v4f* pCos, const v4f& d);
	v4f _mm_tan_ps(const v4f& x);
	v4f _mm_atan_ps(const v4f& d);
	v4f _mm_atan2_ps(const v4f& y, const v4f& x);
	v4f _mm_asin_ps(const v4f& d);
	v4f _mm_acos_ps(const v4f& d);
	v4f _mm_log_ps(const v4f& d);
	v4f _mm_exp_ps(const v4f& d);

	ENGINE_INLINE v4f _mm_pow_ps(const v4f& x, const v4f& y)
	{
		v4f t = _mm_log_ps(x);
		t = _mm_mul_ps(t, y);

		return _mm_exp_ps(t);
	}

#endif

//======================================================================================================================
//														float2
//======================================================================================================================

class float2
{
	public:

		union
		{
			struct
			{
				v2i mm;
			};

			struct
			{
				float v[COORD_2D];
			};

			struct
			{
				float x, y;
			};
		};

	public:

		// NOTE: constructors

		ENGINE_INLINE float2()
		{
		}

		ENGINE_INLINE float2(float a, float b) : x(a), y(b)
		{
		}

		ENGINE_INLINE float2(float a) : x(a), y(a)
		{
		}

		ENGINE_INLINE float2(const float2& v) : x(v.x), y(v.y)
		{
		}

		// NOTE: set

		ENGINE_INLINE void Set0()
		{
			Set(0.0f);
		}

		ENGINE_INLINE void Set(const float2& v)
		{
			x = v.x;
			y = v.y;
		}

		ENGINE_INLINE void Set(float a, float b)
		{
			x = a;
			y = b;
		}

		ENGINE_INLINE void Set(float xy)
		{
			x = y = xy;
		}

		ENGINE_INLINE void operator = (const float2& v)
		{
			Set(v);
		}

		// NOTE: compare

		ENGINE_INLINE bool operator == (const float2& v) const
		{
			return x == v.x && y == v.y;
		}

		ENGINE_INLINE bool operator != (const float2& v) const
		{
			return x != v.x || y != v.y;
		}

		// NOTE: arithmetic

		ENGINE_INLINE float2 operator - () const
		{
			return float2(-x, -y);
		}

		ENGINE_INLINE float2 operator + (const float2& v) const
		{
			return float2(x + v.x, y + v.y);
		}

		ENGINE_INLINE float2 operator - (const float2& v) const
		{
			return float2(x - v.x, y - v.y);
		}

		ENGINE_INLINE float2 operator / (float u) const
		{
			DEBUG_Assert( u != 0.0f );

			float ou = 1.0f / u;

			return float2(x * ou, y * ou);
		}

		ENGINE_INLINE float2 operator * (float v) const
		{
			return float2(x * v, y * v);
		}

		ENGINE_INLINE float2 operator * (const float2& v) const
		{
			return float2(x * v.x, y * v.y);
		}

		ENGINE_INLINE void operator /= (const float2& v)
		{
			DEBUG_Assert( v.x != 0.0f );
			DEBUG_Assert( v.y != 0.0f );

			x /= v.x;
			y /= v.y;
		}

		ENGINE_INLINE void operator /= (float v)
		{
			DEBUG_Assert( v != 0.0f );

			float rv = 1.0f / v;

			x *= rv;
			y *= rv;
		}

		ENGINE_INLINE void operator *= (const float2& v)
		{
			x *= v.x;
			y *= v.y;
		}

		ENGINE_INLINE void operator *= (float v)
		{
			x *= v;
			y *= v;
		}

		ENGINE_INLINE void operator += (const float2& v)
		{
			x += v.x;
			y += v.y;
		}

		ENGINE_INLINE void operator -= (const float2& v)
		{
			x -= v.x;
			y -= v.y;
		}

		ENGINE_INLINE friend float2 operator * (float s, const float2& v)
		{
			return float2(v.x * s, v.y * s);
		}

		// NOTE: swizzle

		ENGINE_INLINE float2 xx() const { return float2(x, x); }
		ENGINE_INLINE float2 xy() const { return float2(x, y); }
		ENGINE_INLINE float2 yx() const { return float2(y, x); }
		ENGINE_INLINE float2 yy() const { return float2(y, y); }
};

ENGINE_INLINE float Dot22(const float2& a, const float2& b)
{
	return a.x * b.x + a.y * b.y;
}

ENGINE_INLINE float LengthSquared(const float2& x)
{
	return Dot22(x, x);
}

ENGINE_INLINE float Length(const float2& x)
{
	return Sqrt( LengthSquared(x) );
}

ENGINE_INLINE float2 Normalize(const float2& x)
{
	return x / Length(x);
}

ENGINE_INLINE float2 Perpendicular(const float2& a)
{
	return float2(-a.y, a.x);
}

ENGINE_INLINE float2 Rotate(const float2& v, float angle)
{
	float sa = Sin(angle);
	float ca = Cos(angle);

	float2 p;
	p.x = ca * v.x + sa * v.y;
	p.y = ca * v.y - sa * v.x;

	return p;
}

//======================================================================================================================
//														float3
//======================================================================================================================

class float3
{
	public:

		static const float3 ortx;
		static const float3 orty;
		static const float3 ortz;
		static const float3 one;

	public:

		union
		{
			struct
			{
				v4f xmm;
			};

			struct
			{
				float v[COORD_3D];
			};

			struct
			{
				float x, y, z;
			};
		};

	public:

		// NOTE: constructors

		ENGINE_INLINE float3()
		{
		}

		ENGINE_INLINE float3(float a, float b, float c) : xmm( xmm_set_4f(a, b, c, 0.0f) )
		{
		}

		ENGINE_INLINE float3(float a) : xmm( _mm_broadcast_ss(&a) )
		{
		}

		ENGINE_INLINE float3(const float* v3) : xmm( xmm_set_4f(v3[0], v3[1], v3[2], 0.0f) )
		{
		}

		ENGINE_INLINE float3(const float3& vec) : xmm( vec.xmm )
		{
		}

		ENGINE_INLINE float3(const v4f& m) : xmm(m)
		{
		}

		// FIXME: used only for template cExpression instantiation

		ENGINE_INLINE float3(uint a) : xmm( _mm_set1_ps(float(a)) )
		{
		}

		// NOTE: set

		ENGINE_INLINE void Set0()
		{
			xmm = xmm_zero;
		}

		ENGINE_INLINE void Set(const float* v3)
		{
			DEBUG_Assert(v3);

			xmm = xmm_set_4f(v3[0], v3[1], v3[2], 0.0f);
		}

		ENGINE_INLINE void Set(const float3& v)
		{
			Set(v.xmm);
		}

		ENGINE_INLINE void Set(float a, float b, float c)
		{
			xmm = xmm_set_4f(a, b, c, 0.0f);
		}

		ENGINE_INLINE void Set(float a)
		{
			xmm = _mm_broadcast_ss(&a);
		}

		ENGINE_INLINE void Set(const v4f& m)
		{
			xmm = m;
		}

		ENGINE_INLINE void operator = (const float3& vec)
		{
			Set(vec);
		}

		// NOTE: compare

		ENGINE_INLINE bool operator == (const float3& v) const
		{
			v4f r = xmm_equal(xmm, v.xmm);

			return xmm_test3_all(r);
		}

		ENGINE_INLINE bool operator != (const float3& v) const
		{
			v4f r = xmm_notequal(xmm, v.xmm);

			return xmm_test3_any(r);
		}

		// NOTE: arithmetic

		ENGINE_INLINE float3 operator - () const
		{
			return xmm_negate(xmm);
		}

		ENGINE_INLINE float3 operator + (const float3& v) const
		{
			return _mm_add_ps(xmm, v.xmm);
		}

		ENGINE_INLINE float3 operator - (const float3& v) const
		{
			return _mm_sub_ps(xmm, v.xmm);
		}

		ENGINE_INLINE float3 operator * (const float3& v) const
		{
			return _mm_mul_ps(xmm, v.xmm);
		}

		ENGINE_INLINE float3 operator / (const float3& v) const
		{
			DEBUG_Assert( xmm_not0_all(v.xmm) );

			return _mm_div_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator += (const float3& v)
		{
			xmm = _mm_add_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator -= (const float3& v)
		{
			xmm = _mm_sub_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator *= (const float3& v)
		{
			xmm = _mm_mul_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator /= (const float3& v)
		{
			DEBUG_Assert( xmm_not0_all(v.xmm) );

			xmm = _mm_div_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator *= (float s)
		{
			xmm = _mm_mul_ps(xmm, _mm_broadcast_ss(&s));
		}

		ENGINE_INLINE void operator /= (float s)
		{
			DEBUG_Assert( s != 0.0f );

			xmm = _mm_div_ps(xmm, _mm_broadcast_ss(&s));
		}

		ENGINE_INLINE float3 operator / (float s) const
		{
			DEBUG_Assert( s != 0.0f );

			return _mm_div_ps(xmm, _mm_broadcast_ss(&s));
		}

		ENGINE_INLINE float3 operator * (float s) const
		{
			return _mm_mul_ps(xmm, _mm_broadcast_ss(&s));
		}

		// NOTE: misc

		ENGINE_INLINE bool IsZero() const
		{
			v4f r = xmm_equal(xmm, xmm_zero);

			return xmm_test3_all(r);
		}

		// NOTE: swizzle

		ENGINE_INLINE float3 xxx() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_X, 0); }
		ENGINE_INLINE float3 xxy() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Y, 0); }
		ENGINE_INLINE float3 xxz() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Z, 0); }
		ENGINE_INLINE float3 xyx() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_X, 0); }
		ENGINE_INLINE float3 xyy() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Y, 0); }
		ENGINE_INLINE float3 xyz() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Z, 0); }
		ENGINE_INLINE float3 xzx() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_X, 0); }
		ENGINE_INLINE float3 xzy() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Y, 0); }
		ENGINE_INLINE float3 xzz() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Z, 0); }
		ENGINE_INLINE float3 yxx() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_X, 0); }
		ENGINE_INLINE float3 yxy() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Y, 0); }
		ENGINE_INLINE float3 yxz() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Z, 0); }
		ENGINE_INLINE float3 yyx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_X, 0); }
		ENGINE_INLINE float3 yyy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Y, 0); }
		ENGINE_INLINE float3 yyz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Z, 0); }
		ENGINE_INLINE float3 yzx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_X, 0); }
		ENGINE_INLINE float3 yzy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Y, 0); }
		ENGINE_INLINE float3 yzz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Z, 0); }
		ENGINE_INLINE float3 zxx() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_X, 0); }
		ENGINE_INLINE float3 zxy() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Y, 0); }
		ENGINE_INLINE float3 zxz() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Z, 0); }
		ENGINE_INLINE float3 zyx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_X, 0); }
		ENGINE_INLINE float3 zyy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Y, 0); }
		ENGINE_INLINE float3 zyz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Z, 0); }
		ENGINE_INLINE float3 zzx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_X, 0); }
		ENGINE_INLINE float3 zzy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Y, 0); }
		ENGINE_INLINE float3 zzz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Z, 0); }
};

ENGINE_INLINE float Dot33(const float3& a, const float3& b)
{
	v4f r = xmm_dot33(a.xmm, b.xmm);

	return xmm_get_x(r);
}

ENGINE_INLINE float LengthSquared(const float3& x)
{
	v4f r = xmm_dot33(x.xmm, x.xmm);

	return xmm_get_x(r);
}

ENGINE_INLINE float Length(const float3& x)
{
	v4f r = xmm_length(x.xmm);

	return xmm_get_x(r);
}

ENGINE_INLINE float3 Normalize(const float3& x)
{
	return xmm_normalize(x.xmm);
}

ENGINE_INLINE float3 Cross(const float3& x, const float3& y)
{
	return xmm_cross(x.xmm, y.xmm);
}

ENGINE_INLINE float3 Rcp(const float3& x)
{
	return xmm_rcp(x.xmm);
}

ENGINE_INLINE float3 Ceil(const float3& x)
{
	return xmm_ceil(x.xmm);
}

ENGINE_INLINE float3 Madd(const float3& a, const float3& b, const float3& c)
{
	return xmm_madd(a.xmm, b.xmm, c.xmm);
}

// NOTE: build basis from normal vector
// IMPORTANT: must be synchronized with GLSL version in std_lib.glsl

ENGINE_INLINE void BasisFromDirection_carmack(const float3& N, float3& T, float3& B)
{
	T.Set(N.z, -N.x, N.y);
	T -= N * Dot33(T, N);
	T = Normalize(T);
	
	B = Cross(N, T);
}

ENGINE_INLINE void BasisFromDirection_frisvad(const float3& N, float3& T, float3& B)
{
	/*
	// ORIGINAL:
	if( N.z < -0.9999999f )
	{
		T.Set(0.0f, -1.0f, 0.0f);
		B.Set(-1.0f, 0.0f, 0.0f);

		return;
	}
	*/

	DEBUG_Assert( N.z > -0.9999999f );

	float a = 1.0f / (1.0f + N.z);
	float3 c = float3(0.0f, 1.0f, 1.0f) - N.yxy() * N.xxy() * a;

	T.Set(c.y, c.x, -N.x);
	B.Set(c.x, c.z, -N.y);
}

//======================================================================================================================
//														float4
//======================================================================================================================

class float4
{
	public:

		union
		{
			struct
			{
				v4f xmm;
			};

			struct
			{
				float v[COORD_4D];
			};

			struct
			{
				float x, y, z, w;
			};
		};

	public:

		// NOTE: constructors

		ENGINE_INLINE float4()
		{
		}
		
		ENGINE_INLINE float4(float a) : xmm( _mm_broadcast_ss(&a) )
		{
		}
		
		ENGINE_INLINE float4(float a, float b, float c, float d) : xmm( xmm_set_4f(a, b, c, d) )
		{
		}

		ENGINE_INLINE float4(float a, float b, float c) : xmm( xmm_set_4f(a, b, c, 1.0f) )
		{
		}

		ENGINE_INLINE float4(const float* v4) : xmm( _mm_loadu_ps(v4) )
		{
		}

		ENGINE_INLINE float4(const float4& vec) : xmm( vec.xmm )
		{
		}

		ENGINE_INLINE float4(const v4f& m) : xmm(m)
		{
		}

		// NOTE: set

		ENGINE_INLINE void Set0001()
		{
			xmm = c_xmm0001;
		}

		ENGINE_INLINE void Set0()
		{
			xmm = xmm_zero;
		}

		ENGINE_INLINE void Set(float a, float b, float c, float d = 1.0f)
		{
			xmm = xmm_set_4f(a, b, c, d);
		}

		ENGINE_INLINE void Set(const float* v4)
		{
			DEBUG_Assert(v4);

			xmm = _mm_loadu_ps(v4);
		}

		ENGINE_INLINE void Set(const float4& v)
		{
			xmm = v.xmm;
		}

		ENGINE_INLINE void Set(const float3& v)
		{
			xmm = xmm_setw1(v.xmm);
		}

		ENGINE_INLINE void Set(const v4f& m)
		{
			xmm = m;
		}

		ENGINE_INLINE void operator = (const float4& vec)
		{
			Set(vec);
		}

		// NOTE: compare

		ENGINE_INLINE bool operator == (const float4& v) const
		{
			v4f r = xmm_equal(xmm, v.xmm);

			return xmm_test4_all(r);
		}

		ENGINE_INLINE bool operator != (const float4& v) const
		{
			v4f r = xmm_notequal(xmm, v.xmm);

			return xmm_test4_any(r);
		}

		// NOTE: arithmetic

		ENGINE_INLINE float4 operator - () const
		{
			return xmm_negate(xmm);
		}

		ENGINE_INLINE float4 operator + (const float4& v) const
		{
			return _mm_add_ps(xmm, v.xmm);
		}

		ENGINE_INLINE float4 operator - (const float4& v) const
		{
			return _mm_sub_ps(xmm, v.xmm);
		}

		ENGINE_INLINE float4 operator * (const float4& v) const
		{
			return _mm_mul_ps(xmm, v.xmm);
		}

		ENGINE_INLINE float4 operator / (const float4& v) const
		{
			DEBUG_Assert( xmm_not0_all(v.xmm) );

			return _mm_div_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator += (const float4& v)
		{
			xmm = _mm_add_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator -= (const float4& v)
		{
			xmm = _mm_sub_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator *= (const float4& v)
		{
			xmm = _mm_mul_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator /= (const float4& v)
		{
			DEBUG_Assert( xmm_not0_all(v.xmm) );

			xmm = _mm_div_ps(xmm, v.xmm);
		}

		ENGINE_INLINE void operator *= (float s)
		{
			xmm = _mm_mul_ps(xmm, _mm_broadcast_ss(&s));
		}

		ENGINE_INLINE void operator /= (float s)
		{
			DEBUG_Assert( s != 0.0f );

			xmm = _mm_div_ps(xmm, _mm_broadcast_ss(&s));
		}

		ENGINE_INLINE float4 operator / (float s) const
		{
			DEBUG_Assert( s != 0.0f );

			return _mm_div_ps(xmm, _mm_broadcast_ss(&s));
		}

		ENGINE_INLINE float4 operator * (float s) const
		{
			return _mm_mul_ps(xmm, _mm_broadcast_ss(&s));
		}

		// NOTE: misc

		ENGINE_INLINE float3 To3d() const
		{
			return xmm;
		}

		ENGINE_INLINE bool IsZero() const
		{
			v4f r = xmm_equal(xmm, xmm_zero);

			return xmm_test4_all(r);
		}

		// NOTE: swizzle

		ENGINE_INLINE float4 xxxx() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE float4 xxxy() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 xxxz() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 xxxw() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE float4 xxyx() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 xxyy() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 xxyz() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 xxyw() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 xxzx() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 xxzy() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 xxzz() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 xxzw() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 xxwx() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE float4 xxwy() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 xxwz() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 xxww() const { return xmm_swizzle(xmm, COORD_X, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE float4 xyxx() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE float4 xyxy() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 xyxz() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 xyxw() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE float4 xyyx() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 xyyy() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 xyyz() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 xyyw() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 xyzx() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 xyzy() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 xyzz() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 xyzw() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 xywx() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE float4 xywy() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 xywz() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 xyww() const { return xmm_swizzle(xmm, COORD_X, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE float4 xzxx() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE float4 xzxy() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 xzxz() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 xzxw() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE float4 xzyx() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 xzyy() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 xzyz() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 xzyw() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 xzzx() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 xzzy() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 xzzz() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 xzzw() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 xzwx() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE float4 xzwy() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 xzwz() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 xzww() const { return xmm_swizzle(xmm, COORD_X, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE float4 xwxx() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE float4 xwxy() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 xwxz() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 xwxw() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE float4 xwyx() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 xwyy() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 xwyz() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 xwyw() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 xwzx() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 xwzy() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 xwzz() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 xwzw() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 xwwx() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE float4 xwwy() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 xwwz() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 xwww() const { return xmm_swizzle(xmm, COORD_X, COORD_W, COORD_W, COORD_W); }
		ENGINE_INLINE float4 yxxx() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE float4 yxxy() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 yxxz() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 yxxw() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE float4 yxyx() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 yxyy() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 yxyz() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 yxyw() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 yxzx() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 yxzy() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 yxzz() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 yxzw() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 yxwx() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE float4 yxwy() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 yxwz() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 yxww() const { return xmm_swizzle(xmm, COORD_Y, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE float4 yyxx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE float4 yyxy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 yyxz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 yyxw() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE float4 yyyx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 yyyy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 yyyz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 yyyw() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 yyzx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 yyzy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 yyzz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 yyzw() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 yywx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE float4 yywy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 yywz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 yyww() const { return xmm_swizzle(xmm, COORD_Y, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE float4 yzxx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE float4 yzxy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 yzxz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 yzxw() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE float4 yzyx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 yzyy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 yzyz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 yzyw() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 yzzx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 yzzy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 yzzz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 yzzw() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 yzwx() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE float4 yzwy() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 yzwz() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 yzww() const { return xmm_swizzle(xmm, COORD_Y, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE float4 ywxx() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE float4 ywxy() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 ywxz() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 ywxw() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE float4 ywyx() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 ywyy() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 ywyz() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 ywyw() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 ywzx() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 ywzy() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 ywzz() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 ywzw() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 ywwx() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE float4 ywwy() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 ywwz() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 ywww() const { return xmm_swizzle(xmm, COORD_Y, COORD_W, COORD_W, COORD_W); }
		ENGINE_INLINE float4 zxxx() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE float4 zxxy() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 zxxz() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 zxxw() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE float4 zxyx() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 zxyy() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 zxyz() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 zxyw() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 zxzx() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 zxzy() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 zxzz() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 zxzw() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 zxwx() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE float4 zxwy() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 zxwz() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 zxww() const { return xmm_swizzle(xmm, COORD_Z, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE float4 zyxx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE float4 zyxy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 zyxz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 zyxw() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE float4 zyyx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 zyyy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 zyyz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 zyyw() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 zyzx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 zyzy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 zyzz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 zyzw() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 zywx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE float4 zywy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 zywz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 zyww() const { return xmm_swizzle(xmm, COORD_Z, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE float4 zzxx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE float4 zzxy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 zzxz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 zzxw() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE float4 zzyx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 zzyy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 zzyz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 zzyw() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 zzzx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 zzzy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 zzzz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 zzzw() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 zzwx() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE float4 zzwy() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 zzwz() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 zzww() const { return xmm_swizzle(xmm, COORD_Z, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE float4 zwxx() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE float4 zwxy() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 zwxz() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 zwxw() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE float4 zwyx() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 zwyy() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 zwyz() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 zwyw() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 zwzx() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 zwzy() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 zwzz() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 zwzw() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 zwwx() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE float4 zwwy() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 zwwz() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 zwww() const { return xmm_swizzle(xmm, COORD_Z, COORD_W, COORD_W, COORD_W); }
		ENGINE_INLINE float4 wxxx() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_X, COORD_X); }
		ENGINE_INLINE float4 wxxy() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 wxxz() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 wxxw() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_X, COORD_W); }
		ENGINE_INLINE float4 wxyx() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 wxyy() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 wxyz() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 wxyw() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 wxzx() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 wxzy() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 wxzz() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 wxzw() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 wxwx() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_W, COORD_X); }
		ENGINE_INLINE float4 wxwy() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 wxwz() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 wxww() const { return xmm_swizzle(xmm, COORD_W, COORD_X, COORD_W, COORD_W); }
		ENGINE_INLINE float4 wyxx() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_X, COORD_X); }
		ENGINE_INLINE float4 wyxy() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 wyxz() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 wyxw() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_X, COORD_W); }
		ENGINE_INLINE float4 wyyx() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 wyyy() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 wyyz() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 wyyw() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 wyzx() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 wyzy() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 wyzz() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 wyzw() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 wywx() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_W, COORD_X); }
		ENGINE_INLINE float4 wywy() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 wywz() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 wyww() const { return xmm_swizzle(xmm, COORD_W, COORD_Y, COORD_W, COORD_W); }
		ENGINE_INLINE float4 wzxx() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_X, COORD_X); }
		ENGINE_INLINE float4 wzxy() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 wzxz() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 wzxw() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_X, COORD_W); }
		ENGINE_INLINE float4 wzyx() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 wzyy() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 wzyz() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 wzyw() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 wzzx() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 wzzy() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 wzzz() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 wzzw() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 wzwx() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_W, COORD_X); }
		ENGINE_INLINE float4 wzwy() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 wzwz() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 wzww() const { return xmm_swizzle(xmm, COORD_W, COORD_Z, COORD_W, COORD_W); }
		ENGINE_INLINE float4 wwxx() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_X, COORD_X); }
		ENGINE_INLINE float4 wwxy() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_X, COORD_Y); }
		ENGINE_INLINE float4 wwxz() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_X, COORD_Z); }
		ENGINE_INLINE float4 wwxw() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_X, COORD_W); }
		ENGINE_INLINE float4 wwyx() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Y, COORD_X); }
		ENGINE_INLINE float4 wwyy() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Y, COORD_Y); }
		ENGINE_INLINE float4 wwyz() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Y, COORD_Z); }
		ENGINE_INLINE float4 wwyw() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Y, COORD_W); }
		ENGINE_INLINE float4 wwzx() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Z, COORD_X); }
		ENGINE_INLINE float4 wwzy() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Z, COORD_Y); }
		ENGINE_INLINE float4 wwzz() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Z, COORD_Z); }
		ENGINE_INLINE float4 wwzw() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_Z, COORD_W); }
		ENGINE_INLINE float4 wwwx() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_W, COORD_X); }
		ENGINE_INLINE float4 wwwy() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_W, COORD_Y); }
		ENGINE_INLINE float4 wwwz() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_W, COORD_Z); }
		ENGINE_INLINE float4 wwww() const { return xmm_swizzle(xmm, COORD_W, COORD_W, COORD_W, COORD_W); }
};

ENGINE_INLINE float Dot44(const float4& a, const float4& b)
{
	v4f r = xmm_dot44(a.xmm, b.xmm);

	return xmm_get_x(r);
}

ENGINE_INLINE float Dot43(const float4& a, const float3& b)
{
	v4f r = xmm_dot43(a.xmm, b.xmm);

	return xmm_get_x(r);
}

ENGINE_INLINE float4 Rcp(const float4& x)
{
	return xmm_rcp(x.xmm);
}

ENGINE_INLINE float4 Ceil(const float4& x)
{
	return xmm_ceil(x.xmm);
}

ENGINE_INLINE float4 Madd(const float4& a, const float4& b, const float4& c)
{
	return xmm_madd(a.xmm, b.xmm, c.xmm);
}

#endif
