#pragma once
#ifndef _MATHLIB_H_
#define _MATHLIB_H_

//======================================================================================================================
//													Pre-header
//======================================================================================================================

// FIXME: move to platform.h

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

typedef unsigned										uint;
typedef unsigned char									uchar;
typedef unsigned short									ushort;
typedef __int64											int64;
typedef unsigned __int64								uint64;

#ifdef _DEBUG

	#define DEBUG_Assert(x)								assert( x )
	#define DEBUG_AssertMsg(x, msg)						(void)( (!!(x)) || (_wassert(_CRT_WIDE("\"") _CRT_WIDE(#x) _CRT_WIDE("\" - ") _CRT_WIDE(msg), _CRT_WIDE(__FILE__), __LINE__), 0) )

#else

	#define DEBUG_Assert(x)								((void)0)
	#define DEBUG_AssertMsg(x, msg)						((void)0)

#endif

#define DEBUG_StaticAssert(x)							static_assert(x, #x)
#define DEBUG_StaticAssertMsg(x, msg)					static_assert(x, msg)

#define ENGINE_UNUSED(x)								(void)x

#define ENGINE_INTRINSIC_SSE3							0		// NOTE: +SSSE3
#define ENGINE_INTRINSIC_SSE4							1
#define ENGINE_INTRINSIC_AVX1							2		// NOTE: +FP16C
#define ENGINE_INTRINSIC_AVX2							3		// NOTE: +FMA3

// NOTE: limits

#define ENGINE_MAX_CHAR									char(~(1 << (sizeof(char) * 8 - 1)))
#define ENGINE_MAX_SHORT								short(~(1 << (sizeof(short) * 8 - 1)))
#define ENGINE_MAX_INT									int(~(1 << (sizeof(int) * 8 - 1)))
#define ENGINE_MAX_INT64								int64(~(1LL << (sizeof(int64) * 8 - 1)))

#define ENGINE_MAX_UCHAR								uchar(~0)
#define ENGINE_MAX_USHORT								ushort(~0)
#define ENGINE_MAX_UINT									uint(~0)
#define ENGINE_MAX_UINT64								uint64(~0)

#define ENGINE_INF										1e20f

#ifdef PROPS_AVX2
	#define ENGINE_INTRINSIC							ENGINE_INTRINSIC_AVX2
#elif( defined PROPS_AVX1 )
	#define ENGINE_INTRINSIC							ENGINE_INTRINSIC_AVX1
#elif( defined PROPS_SSE4 )
	#define ENGINE_INTRINSIC							ENGINE_INTRINSIC_SSE4
#else
	#define ENGINE_INTRINSIC							ENGINE_INTRINSIC_SSE3
#endif

#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_AVX1 )
	#define ENGINE_ALIGN_CPU							32
#else
	#define ENGINE_ALIGN_CPU							16
#endif

#if( _MSC_VER < 1800 )
	#define _round(x)									floor(x + T(0.5))
#else
	#define _round(x)									round(x)
#endif

#define	ENGINE_INLINE									__forceinline

//======================================================================================================================

// NOTE: all random floating point functions doesn't return zero (because I hate zeroes)
//		 ranges: uf - (0; 1], sf - [-1; 0) (0; 1]

// NOTE: C standard rand() (15 bit, floating point with fixed step 1 / 0x7FFF)
// perf: 1x
// thread: safe
#define ENGINE_RND_CRT			0

// NOTE: extreme fast PRNG (32 bit)
// perf: 15x
// thread: NOT SAFE (but you can create your own instances of sFastRand in another threads)
#define ENGINE_RND_FAST			1

// NOTE: slower, hardware implemented CSPRNG (cryptographically secure), "rdrand" instruction support is required
// https://software.intel.com/sites/default/files/managed/4d/91/DRNG_Software_Implementation_Guide_2.0.pdf
// perf: 0.4x
// thread: safe
#define ENGINE_RND_HW			2

//======================================================================================================================
//													Settings
//======================================================================================================================

// NOTE: more precision (a little bit slower)
#define MATH_NEWTONRAPHSON_APROXIMATION

// NOTE: only for debug (not need if you are accurate in horizontal operations)
#define MATH_CHECK_W_IS_ZERO
#undef MATH_CHECK_W_IS_ZERO

// NOTE: only for debug (generate exeptions in rounding operations, only for SSE4)
#define MATH_EXEPTIONS
#undef MATH_EXEPTIONS

#define ENGINE_RND_MODE			ENGINE_RND_FAST

//======================================================================================================================
//														SIMD ASM
//======================================================================================================================

#include <intrin.h>

#ifdef MATH_EXEPTIONS
	#define ROUNDING_EXEPTIONS_MASK		_MM_FROUND_RAISE_EXC
#else
	#define ROUNDING_EXEPTIONS_MASK		_MM_FROUND_NO_EXC
#endif

#include "intrin_emu.h"

typedef __m128		v4f;
typedef __m256		v8f;

typedef __m64		v2i;
typedef __m128i		v4i;
typedef __m256i		v8i;

typedef __m128d		v2d;
typedef __m256d		v4d;

//======================================================================================================================
//													Enums
//======================================================================================================================

enum eClip : uchar
{
	CLIP_OUT,
	CLIP_IN,
	CLIP_PARTIAL,
};

enum eCmp : uchar
{
	CmpLess			= _CMP_LT_OQ,
	CmpLequal		= _CMP_LE_OQ,
	CmpGreater		= _CMP_GT_OQ,
	CmpGequal		= _CMP_GE_OQ,
	CmpEqual		= _CMP_EQ_OQ,
	CmpNotequal		= _CMP_NEQ_UQ,
};

enum eCoordinate : uint
{
	COORD_X				= 0,
	COORD_Y,
	COORD_Z,
	COORD_W,

	COORD_S				= 0,
	COORD_T,
	COORD_R,
	COORD_Q,

	COORD_2D			= 2,
	COORD_3D,
	COORD_4D,
};

enum ePlaneType : uint
{
	PLANE_LEFT,
	PLANE_RIGHT,
	PLANE_BOTTOM,
	PLANE_TOP,
	PLANE_NEAR,
	PLANE_FAR,

	PLANES_NUM,

	PLANE_MASK_L		= 1 << PLANE_LEFT,
	PLANE_MASK_R		= 1 << PLANE_RIGHT,
	PLANE_MASK_B		= 1 << PLANE_BOTTOM,
	PLANE_MASK_T		= 1 << PLANE_TOP,
	PLANE_MASK_N		= 1 << PLANE_NEAR,
	PLANE_MASK_F		= 1 << PLANE_FAR,

	PLANE_MASK_NONE		= 0,
	PLANE_MASK_LRBT		= PLANE_MASK_L | PLANE_MASK_R | PLANE_MASK_B | PLANE_MASK_T,
	PLANE_MASK_NF		= PLANE_MASK_N | PLANE_MASK_F,
	PLANE_MASK_LRBTNF	= PLANE_MASK_LRBT | PLANE_MASK_NF,
};

enum eProjectionData
{
	PROJ_ZNEAR,
	PROJ_ZFAR,
	PROJ_ASPECT,
	PROJ_FOVX,
	PROJ_FOVY,
	PROJ_MINX,
	PROJ_MAXX,
	PROJ_MINY,
	PROJ_MAXY,
	PROJ_DIRX,
	PROJ_DIRY,
	PROJ_ANGLEMINX,
	PROJ_ANGLEMAXX,
	PROJ_ANGLEMINY,
	PROJ_ANGLEMAXY,
	PROJ_ISORTHO,

	PROJ_NUM,
};

//======================================================================================================================
//														Misc
//======================================================================================================================

//#include "zbuffer.h"

template<class T> ENGINE_INLINE T Sign(const T& x);
template<class T> ENGINE_INLINE T Abs(const T& x);
template<class T> ENGINE_INLINE T Floor(const T& x);
template<class T> ENGINE_INLINE T Round(const T& x);
template<class T> ENGINE_INLINE T Fract(const T& x);
template<class T> ENGINE_INLINE T Mod(const T& x, const T& y);
template<class T> ENGINE_INLINE T Snap(const T& x, const T& step);
template<class T> ENGINE_INLINE T Min(const T& x, const T& y);
template<class T> ENGINE_INLINE T Max(const T& x, const T& y);
template<class T> ENGINE_INLINE T Clamp(const T& x, const T& a, const T& b);
template<class T> ENGINE_INLINE T Saturate(const T& x);
template<class T> ENGINE_INLINE T Mix(const T& a, const T& b, const T& x);
template<class T> ENGINE_INLINE T Smoothstep(const T& a, const T& b, const T& x);
template<class T> ENGINE_INLINE T Linearstep(const T& a, const T& b, const T& x);
template<class T> ENGINE_INLINE T Step(const T& edge, const T& x);

template<class T> ENGINE_INLINE T Sin(const T& x);
template<class T> ENGINE_INLINE T Cos(const T& x);
template<class T> ENGINE_INLINE T SinCos(const T& x, T* pCos);
template<class T> ENGINE_INLINE T Tan(const T& x);
template<class T> ENGINE_INLINE T Asin(const T& x);
template<class T> ENGINE_INLINE T Acos(const T& x);
template<class T> ENGINE_INLINE T Atan(const T& x);
template<class T> ENGINE_INLINE T Atan(const T& y, const T& x);
template<class T> ENGINE_INLINE T Sqrt(const T& x);
template<class T> ENGINE_INLINE T Rsqrt(const T& x);
template<class T> ENGINE_INLINE T Pow(const T& x, const T& y);
template<class T> ENGINE_INLINE T Log(const T& x);
template<class T> ENGINE_INLINE T Exp(const T& x);

template<class T> ENGINE_INLINE T Pi(const T& mul = T(1));
template<class T> ENGINE_INLINE T RadToDeg(const T& a);
template<class T> ENGINE_INLINE T DegToRad(const T& a);

template<class T> ENGINE_INLINE void Swap(T& x, T& y);

template<eCmp cmp, class T> ENGINE_INLINE bool All(const T& x, const T& y);
template<eCmp cmp, class T> ENGINE_INLINE bool Any(const T& x, const T& y);

#include "mathlib_float.h"
#include "mathlib_double.h"

//======================================================================================================================
//														Conversions
//======================================================================================================================

ENGINE_INLINE float3 ToFloat(const double3& x)
{
	v4f r = ymm_to_xmm(x.ymm);

	return r;
}

ENGINE_INLINE float4 ToFloat(const double4& x)
{
	v4f r = ymm_to_xmm(x.ymm);

	return r;
}



ENGINE_INLINE double3 ToDouble(const float3& x)
{
	return xmm_to_ymm(x.xmm);
}

ENGINE_INLINE double4 ToDouble(const float4& x)
{
	return xmm_to_ymm(x.xmm);
}

//======================================================================================================================
//														cPosition
//======================================================================================================================

class cPosition
{
	public:

		double3 m_vdOrigin;

	public:

		ENGINE_INLINE cPosition() : m_vdOrigin(0.0)
		{
		}

		ENGINE_INLINE float3 GetRelative(const double3& vdCenter) const
		{
			double3 vdRelative = m_vdOrigin - vdCenter;

			return ToFloat(vdRelative);
		}

		ENGINE_INLINE float3 GetRelative(const double3& vdCenter, const float3& vLocal) const
		{
			float3 vRelative = GetRelative(vdCenter);

			return vRelative + vLocal;
		}

		ENGINE_INLINE float3 GetGlobal() const
		{
			return ToFloat(m_vdOrigin);
		}

		ENGINE_INLINE float3 GetGlobal(const float3& vLocal) const
		{
			double3 vdGlobal = m_vdOrigin + ToDouble(vLocal);

			return ToFloat(vdGlobal);
		}

		ENGINE_INLINE double3 GetGlobalPrecise(const float3& vLocal) const
		{
			return m_vdOrigin + ToDouble(vLocal);
		}
};

//======================================================================================================================
// Floating point tricks
//======================================================================================================================

union uFloat
{
    float f;
    uint i;

	ENGINE_INLINE uFloat()
	{
	}

	ENGINE_INLINE uFloat(float x) : f(x)
	{
	}

	ENGINE_INLINE uFloat(uint x) : i(x)
	{
	}

    ENGINE_INLINE void Abs()
	{
		i &= ~(1 << 31);
	}

    ENGINE_INLINE bool IsNegative() const
	{
		return (i >> 31) != 0;
	}

    ENGINE_INLINE uint Mantissa() const
	{
		return i & ((1 << 23) - 1);
	}

    ENGINE_INLINE uint Exponent() const
	{
		return (i >> 23) & 255;
	}

	ENGINE_INLINE bool IsInf() const
	{
		return Exponent() == 255 && Mantissa() == 0;
	}

	ENGINE_INLINE bool IsNan() const
	{
		return Exponent() == 255 && Mantissa() != 0;
	}

	static ENGINE_INLINE float PrecisionGreater(float x)
	{
		uFloat y(x);
		y.i++;

		return y.f - x;
	}

	static ENGINE_INLINE float PrecisionLess(float x)
	{
		uFloat y(x);
		y.i--;

		return y.f - x;
	}
};

union uDouble
{
    double f;
    uint64 i;

	ENGINE_INLINE uDouble()
	{
	}

	ENGINE_INLINE uDouble(double x) : f(x)
	{
	}

	ENGINE_INLINE uDouble(uint64 x) : i(x)
	{
	}

    ENGINE_INLINE bool IsNegative() const
	{
		return (i >> 63) != 0;
	}

    ENGINE_INLINE void Abs()
	{
		i &= ~(1ULL << 63);
	}

    ENGINE_INLINE uint64 Mantissa() const
	{
		return i & ((1ULL << 52) - 1);
	}

    ENGINE_INLINE uint64 Exponent() const
	{
		return (i >> 52) & 2047;
	}

	ENGINE_INLINE bool IsInf() const
	{
		return Exponent() == 2047 && Mantissa() == 0;
	}

	ENGINE_INLINE bool IsNan() const
	{
		return Exponent() == 2047 && Mantissa() != 0;
	}

	static ENGINE_INLINE double PrecisionGreater(double x)
	{
		uDouble y(x);
		y.i++;

		return y.f - x;
	}

	static ENGINE_INLINE double PrecisionLess(double x)
	{
		uDouble y(x);
		y.i--;

		return y.f - x;
	}
};

float DoubleToGequal(double dValue);
float DoubleToLequal(double dValue);

//======================================================================================================================
//														Errors
//======================================================================================================================

class cError
{
	public:

		double dMeanAbsError;		// NOTE: mean absolute error
		double dMaxAbsError;		// NOTE: max absolute error
		double dMeanSqrError;		// NOTE: mean squared error
		double dRootMeanSqrError;	// NOTE: root mean squared error
		double dPsnr;				// NOTE: peak signal to noise ratio in dB

		uint uiSamples;

	public:

		ENGINE_INLINE cError()
		{
			Zero();
		}

		ENGINE_INLINE void Zero()
		{
			memset(this, 0, sizeof(*this));
		}

		ENGINE_INLINE void AddSample(double dSample)
		{
			double v = fabs(dSample);

			dMeanAbsError += v;
			dMaxAbsError = Max(dMaxAbsError, v);
			dMeanSqrError += v * v;

			uiSamples++;
		}

		ENGINE_INLINE void Done(double dScale)
		{
			if( uiSamples )
			{
				double rs = 1.0 / double(uiSamples);

				dMeanAbsError *= rs;
				dMeanSqrError *= rs;
				dRootMeanSqrError = Sqrt(dMeanSqrError);
				dPsnr = (dRootMeanSqrError < c_fEps) ? 999.0 : 20.0 * log10(dScale / dRootMeanSqrError);
			}
		}
};

class cNormalError
{
	public:

		double dAngDevError;			// NOTE: angular deviation error
		double dMeanSqrError;			// NOTE: mean squared error
		double dRootMeanSqrError;		// NOTE: root mean squared error
		double dPsnr;					// NOTE: peak signal to noise ratio in dB

		uint uiSamples;

	public:

		ENGINE_INLINE cNormalError()
		{
			Zero();
		}

		ENGINE_INLINE void Zero()
		{
			memset(this, 0, sizeof(*this));
		}

		ENGINE_INLINE void AddSample(const float3& n1, const float3& n2, float scale)
		{
			float dot = Dot33(n1, n2);

			dAngDevError += Acos( Clamp(dot, -1.0f, 1.0f) );

			float3 v = (n1 - n2) * (scale * 0.5f);
			dMeanSqrError += LengthSquared(v);

			uiSamples++;
		}

		ENGINE_INLINE void Done(double dScale)
		{
			if( uiSamples )
			{
				double rs = 1.0 / double(uiSamples);

				dAngDevError *= rs;
				dMeanSqrError *= rs / 3.0;
				dRootMeanSqrError = Sqrt(dMeanSqrError);
				dPsnr = (dRootMeanSqrError < c_fEps) ? 999.0 : 20.0 * log10(dScale / dRootMeanSqrError);
			}
		}
};

//======================================================================================================================
//														Random
//======================================================================================================================

struct sFastRand
{
	// NOTE: vector part
	// Super-Fast MWC1616 Pseudo-Random Number Generator for Intel/AMD Processors (using SSE or SSE4 instruction set)
	// Copyright (c) 2012, Ivan Dimkovic, http://www.digicortex.net/node/22
	// MWC1616: http://www.helsbreth.org/random/rng_mwc1616.html

	v4i m_a;
	v4i m_b;

	// NOTE: scalar part
	// https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor

	uint m_uiSeed;

	ENGINE_INLINE sFastRand()
	{
		Seed( rand() );
	}

	ENGINE_INLINE void Seed(uint uiSeed)
	{
		m_uiSeed = uiSeed;

		uint a1 = ui1();
		uint a2 = ui1();
		uint a3 = ui1();
		uint a4 = ui1();
		uint b1 = ui1();
		uint b2 = ui1();
		uint b3 = ui1();
		uint b4 = ui1();

		m_a = _mm_set_epi32(a4, a3, a2, a1);
		m_b = _mm_set_epi32(b4, b3, b2, b1);
	}

	ENGINE_INLINE uint ui1()
	{
		m_uiSeed = 214013 * m_uiSeed + 2531011;

		return m_uiSeed;
	}

	ENGINE_INLINE v4i ui4()
	{
		const v4i mask = _mm_set1_epi32(0xFFFF);
		const v4i m1 = _mm_set1_epi32(0x4650);
		const v4i m2 = _mm_set1_epi32(0x78B7);

		v4i a = m_a;
		v4i b = m_b;

		#if( ENGINE_INTRINSIC < ENGINE_INTRINSIC_SSE4 )

			v4i ashift = _mm_srli_epi32(a, 0x10);
			v4i amask = _mm_and_si128(a, mask);
			v4i amullow = _mm_mullo_epi16(amask, m1);
			v4i amulhigh = _mm_mulhi_epu16(amask, m1);
			v4i amulhigh_shift = _mm_slli_epi32(amulhigh, 0x10);
			v4i amul = _mm_or_si128(amullow, amulhigh_shift);
			v4i anew = _mm_add_epi32(amul, ashift);

			v4i bshift = _mm_srli_epi32(b, 0x10);
			v4i bmask = _mm_and_si128(b, mask);
			v4i bmullow = _mm_mullo_epi16(bmask, m2);
			v4i bmulhigh = _mm_mulhi_epu16(bmask, m2);
			v4i bmulhigh_shift = _mm_slli_epi32(bmulhigh, 0x10);
			v4i bmul = _mm_or_si128(bmullow, bmulhigh_shift);
			v4i bnew = _mm_add_epi32(bmul, bshift);

		#else

			v4i amask = _mm_and_si128(a, mask);
			v4i ashift = _mm_srli_epi32(a, 0x10);
			v4i amul = _mm_mullo_epi32(amask, m1);
			v4i anew = _mm_add_epi32(amul, ashift);

			v4i bmask = _mm_and_si128(b, mask);
			v4i bshift = _mm_srli_epi32(b, 0x10);
			v4i bmul = _mm_mullo_epi32(bmask, m2);
			v4i bnew = _mm_add_epi32(bmul, bshift);

		#endif

		m_a = anew;
		m_b = bnew;

		v4i bmasknew = _mm_and_si128(bnew, mask);
		v4i ashiftnew = _mm_slli_epi32(anew, 0x10);
		v4i res = _mm_add_epi32(ashiftnew, bmasknew);

		return res;
	}

	ENGINE_INLINE float4 sf4()
	{
		v4i rnd = ui4();
		v4f r;

#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

		r = _mm_cvtepi32_ps(rnd);
		r = xmm_madd(r, _mm_broadcast_ss(&m2), _mm_broadcast_ss(&a2));

#else

		v4f sign = _mm_and_ps(_mm_castsi128_ps(rnd), c_xmmSign);
		r = _mm_castsi128_ps(_mm_srli_epi32(rnd, 8));
		r = _mm_or_ps(r, c_xmm1111);
		r = _mm_sub_ps(_mm_set1_ps(2.0f), r);
		r = _mm_or_ps(r, sign);

#endif

		return r;
	}
};

extern sFastRand g_frand;

namespace Rand
{
	const float m1 = 1.0f / 32768.0f;
	const float m2 = 1.0f / 16384.0f;
	const float a2 = 0.5f / 16384.0f - 1.0f;

	ENGINE_INLINE void Seed(uint seed)
	{
		#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			srand(seed);

		#elif( ENGINE_RND_MODE == ENGINE_RND_FAST )

			g_frand.Seed(seed);

		#else

			DEBUG_AssertMsg(false, "HW RND cannot be seeded!");
			ENGINE_UNUSED(seed);

		#endif
	}

	// NOTE: special

	ENGINE_INLINE float uf1_from_rawbits(uint rawbits)
	{
		uFloat rnd(rawbits);
		rnd.i >>= 9;
		rnd.i |= uFloat(1.0f).i;
		rnd.f = 2.0f - rnd.f;

		return rnd.f;
	}

	ENGINE_INLINE float sf1_from_rawbits(uint rawbits)
	{
		uFloat rnd(rawbits);
		uint sign = rnd.i & 0x80000000;
		rnd.i >>= 8;
		rnd.i |= uFloat(1.0f).i;
		rnd.f = 2.0f - rnd.f;
		rnd.i |= sign;

		return rnd.f;
	}

	// NOTE: scalar

	ENGINE_INLINE uint ui1()
	{
		uint rnd;

		#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			rnd = rand();

		#elif( ENGINE_RND_MODE == ENGINE_RND_FAST )

			rnd = g_frand.ui1();

		#else

			int res = _rdrand32_step(&rnd);
			DEBUG_Assert( res == 1 );
			ENGINE_UNUSED(res);

		#endif

		return rnd;
	}

	ENGINE_INLINE ushort us1()
	{
		ushort rnd;

		#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			rnd = (ushort)rand();

		#elif( ENGINE_RND_MODE == ENGINE_RND_FAST )

			uint t = ui1();
			rnd = ushort(t >> 16);

		#else

			int res = _rdrand16_step(&rnd);
			DEBUG_Assert( res == 1 );
			ENGINE_UNUSED(res);

		#endif

		return rnd;
	}

	ENGINE_INLINE float uf1()
	{
		#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			return float(rand() + 1) * m1;

		#else

			return uf1_from_rawbits( ui1() );

		#endif
	}

	ENGINE_INLINE float sf1()
	{
		#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			return rand() * m2 + a2;

		#else

			return sf1_from_rawbits( ui1() );

		#endif
	}

	// NOTE: vector4

	ENGINE_INLINE v4i ui4()
	{
		v4i r;

		#if( ENGINE_RND_MODE == ENGINE_RND_CRT || ENGINE_RND_MODE == ENGINE_RND_HW )

			int a = ui1();
			int b = ui1();
			int c = ui1();
			int d = ui1();

			r = _mm_set_epi32(a, b, c, d);

		#else

			r = g_frand.ui4();

		#endif

		return r;
	}

	ENGINE_INLINE float4 uf4()
	{
		v4i rnd = ui4();
		v4f r;

		#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			rnd = _mm_add_epi32(rnd, _mm_set1_epi32(1));
			r = _mm_cvtepi32_ps(rnd);
			r = _mm_mul_ps(r, _mm_broadcast_ss(&m1));

		#else

			r = _mm_castsi128_ps(_mm_srli_epi32(rnd, 9));
			r = _mm_or_ps(r, c_xmm1111);
			r = _mm_sub_ps(_mm_set1_ps(2.0f), r);

		#endif

		return r;
	}

	ENGINE_INLINE float4 sf4()
	{
		v4i rnd = ui4();
		v4f r;

		#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			r = _mm_cvtepi32_ps(rnd);
			r = xmm_madd(r, _mm_broadcast_ss(&m2), _mm_broadcast_ss(&a2));

		#else

			v4f sign = _mm_and_ps(_mm_castsi128_ps(rnd), c_xmmSign);
			r = _mm_castsi128_ps(_mm_srli_epi32(rnd, 8));
			r = _mm_or_ps(r, c_xmm1111);
			r = _mm_sub_ps(_mm_set1_ps(2.0f), r);
			r = _mm_or_ps(r, sign);

		#endif

		return r;
	}

	ENGINE_INLINE float3 uf3()
	{
		return uf4().xmm;
	}

	ENGINE_INLINE float3 sf3()
	{
		return sf4().xmm;
	}

	struct cFastRandCached : public sFastRand
	{
//		v4i		m_uiRndCached;
		float4		m_fRndCached;
		uint	m_id;

		cFastRandCached() { m_id = 4; }

		ENGINE_INLINE float4 uf4()
		{
			v4i rnd = ui4();
			v4f r;

#if( ENGINE_RND_MODE == ENGINE_RND_CRT )

			rnd = _mm_add_epi32(rnd, _mm_set1_epi32(1));
			r = _mm_cvtepi32_ps(rnd);
			r = _mm_mul_ps(r, _mm_broadcast_ss(&m1));

#else

			r = _mm_castsi128_ps(_mm_srli_epi32(rnd, 9));
			r = _mm_or_ps(r, c_xmm1111);
			r = _mm_sub_ps(_mm_set1_ps(2.0f), r);

#endif

			return r;
		}

		inline float Randomf() 
		{
			if(m_id == 4)
			{
				m_fRndCached = uf4();
				m_id = 0;
			}
			return m_fRndCached.v[m_id++];
		}
		inline float Randomf(float x) 
		{
			return Randomf()*x;
		}
	};

}

//======================================================================================================================
//														Misc
//======================================================================================================================

template<class T> ENGINE_INLINE T Sign(const T& x)
{
	return x < T(0) ? T(-1) : T(1);
}

template<> ENGINE_INLINE float3 Sign(const float3& x)
{
	return xmm_sign(x.xmm);
}

template<> ENGINE_INLINE float4 Sign(const float4& x)
{
	return xmm_sign(x.xmm);
}

template<> ENGINE_INLINE double3 Sign(const double3& x)
{
	return ymm_sign(x.ymm);
}

template<> ENGINE_INLINE double4 Sign(const double4& x)
{
	return ymm_sign(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Abs(const T& x)
{
	static_assert(std::is_integral_v<T>, "Non-integral value in generic Abs");
    return (T)abs(x);
}

template<> ENGINE_INLINE float Abs(const float& x)
{
	uFloat f(x);
	f.Abs();

	return f.f;
}

template<> ENGINE_INLINE double Abs(const double& x)
{
	uDouble f(x);
	f.Abs();

	return f.f;
}

template<> ENGINE_INLINE float3 Abs(const float3& x)
{
	return xmm_abs(x.xmm);
}

template<> ENGINE_INLINE float4 Abs(const float4& x)
{
	return xmm_abs(x.xmm);
}

template<> ENGINE_INLINE double3 Abs(const double3& x)
{
	return ymm_abs(x.ymm);
}

template<> ENGINE_INLINE double4 Abs(const double4& x)
{
	return ymm_abs(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Floor(const T& x)
{
	return (T)floor(x);
}

template<> ENGINE_INLINE float3 Floor(const float3& x)
{
	return xmm_floor(x.xmm);
}

template<> ENGINE_INLINE float4 Floor(const float4& x)
{
	return xmm_floor(x.xmm);
}

template<> ENGINE_INLINE double3 Floor(const double3& x)
{
	return ymm_floor(x.ymm);
}

template<> ENGINE_INLINE double4 Floor(const double4& x)
{
	return ymm_floor(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Round(const T& x)
{
	return (T)_round(x);
}

template<> ENGINE_INLINE float3 Round(const float3& x)
{
	return xmm_round(x.xmm);
}

template<> ENGINE_INLINE float4 Round(const float4& x)
{
	return xmm_round(x.xmm);
}

template<> ENGINE_INLINE double3 Round(const double3& x)
{
	return ymm_round(x.ymm);
}

template<> ENGINE_INLINE double4 Round(const double4& x)
{
	return ymm_round(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Fract(const T& x)
{
	return x - Floor(x);
}

template<> ENGINE_INLINE float3 Fract(const float3& x)
{
	return xmm_fract(x.xmm);
}

template<> ENGINE_INLINE float4 Fract(const float4& x)
{
	return xmm_fract(x.xmm);
}

template<> ENGINE_INLINE double3 Fract(const double3& x)
{
	return ymm_fract(x.ymm);
}

template<> ENGINE_INLINE double4 Fract(const double4& x)
{
	return ymm_fract(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Mod(const T& x, const T& y)
{
	DEBUG_Assert( y != T(0) );

	return (T)fmod(x, y);
}

template<> ENGINE_INLINE uint Mod(const uint& x, const uint& y)
{
	DEBUG_Assert( y != 0 );

	return x % y;
}

template<> ENGINE_INLINE int Mod(const int& x, const int& y)
{
	DEBUG_Assert( y != 0 );

	return x % y;
}

template<> ENGINE_INLINE float3 Mod(const float3& x, const float3& y)
{
	return xmm_mod(x.xmm, y.xmm);
}

template<> ENGINE_INLINE float4 Mod(const float4& x, const float4& y)
{
	return xmm_mod(x.xmm, y.xmm);
}

template<> ENGINE_INLINE double3 Mod(const double3& x, const double3& y)
{
	return ymm_mod(x.ymm, y.ymm);
}

template<> ENGINE_INLINE double4 Mod(const double4& x, const double4& y)
{
	return ymm_mod(x.ymm, y.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Snap(const T& x, const T& step)
{
	DEBUG_Assert( step > T(0) );

	return Round(x / step) * step;
}

template<> ENGINE_INLINE int Snap(const int& x, const int& step)
{
	DEBUG_Assert( step > 0 );

	return step * ((x + (step >> 1)) / step);
}

template<> ENGINE_INLINE uint Snap(const uint& x, const uint& step)
{
	DEBUG_Assert( step > 0 );

	return step * ((x + (step >> 1)) / step);
}

template<> ENGINE_INLINE float3 Snap(const float3& x, const float3& step)
{
	return Round(x / step) * step;
}

template<> ENGINE_INLINE double3 Snap(const double3& x, const double3& step)
{
	return Round(x / step) * step;
}

//======================================================================================================================

template<eCmp cmp, class T> ENGINE_INLINE bool All(const T&, const T&)
{
	DEBUG_StaticAssertMsg(false, "All::only vector types supported");

	return false;
}

template<eCmp cmp> ENGINE_INLINE bool All(const float3& x, const float3& y)
{
	v4f t = _mm_cmp_ps(x.xmm, y.xmm, cmp);

	return xmm_test3_all(t);
}

template<eCmp cmp> ENGINE_INLINE bool All(const float4& x, const float4& y)
{
	v4f t = _mm_cmp_ps(x.xmm, y.xmm, cmp);

	return xmm_test4_all(t);
}

template<eCmp cmp> ENGINE_INLINE bool All(const double3& x, const double3& y)
{
	v4d t = _mm256_cmp_pd(x.ymm, y.ymm, cmp);

	return ymm_test3_all(t);
}

template<eCmp cmp> ENGINE_INLINE bool All(const double4& x, const double4& y)
{
	v4d t = _mm256_cmp_pd(x.ymm, y.ymm, cmp);

	return ymm_test4_all(t);
}

//======================================================================================================================

template<eCmp cmp, class T> ENGINE_INLINE bool Any(const T&, const T&)
{
	DEBUG_StaticAssertMsg(false, "Any::only vector types supported");

	return false;
}

template<eCmp cmp> ENGINE_INLINE bool Any(const float3& x, const float3& y)
{
	v4f t = _mm_cmp_ps(x.xmm, y.xmm, cmp);

	return xmm_test3_any(t);
}

template<eCmp cmp> ENGINE_INLINE bool Any(const float4& x, const float4& y)
{
	v4f t = _mm_cmp_ps(x.xmm, y.xmm, cmp);

	return xmm_test4_any(t);
}

template<eCmp cmp> ENGINE_INLINE bool Any(const double3& x, const double3& y)
{
	v4d t = _mm256_cmp_pd(x.ymm, y.ymm, cmp);

	return ymm_test3_any(t);
}

template<eCmp cmp> ENGINE_INLINE bool Any(const double4& x, const double4& y)
{
	v4d t = _mm256_cmp_pd(x.ymm, y.ymm, cmp);

	return ymm_test4_any(t);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Min(const T& x, const T& y)
{
	return x < y ? x : y;
}

template<> ENGINE_INLINE float3 Min(const float3& x, const float3& y)
{
	return _mm_min_ps(x.xmm, y.xmm);
}

template<> ENGINE_INLINE float4 Min(const float4& x, const float4& y)
{
	return _mm_min_ps(x.xmm, y.xmm);
}

template<> ENGINE_INLINE double3 Min(const double3& x, const double3& y)
{
	return _mm256_min_pd(x.ymm, y.ymm);
}

template<> ENGINE_INLINE double4 Min(const double4& x, const double4& y)
{
	return _mm256_min_pd(x.ymm, y.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Max(const T& x, const T& y)
{
	return x > y ? x : y;
}

template<> ENGINE_INLINE float3 Max(const float3& x, const float3& y)
{
	return _mm_max_ps(x.xmm, y.xmm);
}

template<> ENGINE_INLINE float4 Max(const float4& x, const float4& y)
{
	return _mm_max_ps(x.xmm, y.xmm);
}

template<> ENGINE_INLINE double3 Max(const double3& x, const double3& y)
{
	return _mm256_max_pd(x.ymm, y.ymm);
}

template<> ENGINE_INLINE double4 Max(const double4& x, const double4& y)
{
	return _mm256_max_pd(x.ymm, y.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Clamp(const T& x, const T& a, const T& b)
{
	return x < a ? a : (x > b ? b : x);
}

template<> ENGINE_INLINE float3 Clamp(const float3& x, const float3& vMin, const float3& vMax)
{
	return xmm_clamp(x.xmm, vMin.xmm, vMax.xmm);
}

template<> ENGINE_INLINE float4 Clamp(const float4& x, const float4& vMin, const float4& vMax)
{
	return xmm_clamp(x.xmm, vMin.xmm, vMax.xmm);
}

template<> ENGINE_INLINE double3 Clamp(const double3& x, const double3& vMin, const double3& vMax)
{
	return ymm_clamp(x.ymm, vMin.ymm, vMax.ymm);
}

template<> ENGINE_INLINE double4 Clamp(const double4& x, const double4& vMin, const double4& vMax)
{
	return ymm_clamp(x.ymm, vMin.ymm, vMax.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Saturate(const T& x)
{
	return Clamp(x, T(0), T(1));
}

template<> ENGINE_INLINE float3 Saturate(const float3& x)
{
	return xmm_saturate(x.xmm);
}

template<> ENGINE_INLINE float4 Saturate(const float4& x)
{
	return xmm_saturate(x.xmm);
}

template<> ENGINE_INLINE double3 Saturate(const double3& x)
{
	return ymm_saturate(x.ymm);
}

template<> ENGINE_INLINE double4 Saturate(const double4& x)
{
	return ymm_saturate(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Mix(const T& a, const T& b, const T& x)
{
	return a + (b - a) * x;
}

template<> ENGINE_INLINE float3 Mix(const float3& a, const float3& b, const float3& x)
{
	return xmm_mix(a.xmm, b.xmm, x.xmm);
}

template<> ENGINE_INLINE float4 Mix(const float4& a, const float4& b, const float4& x)
{
	return xmm_mix(a.xmm, b.xmm, x.xmm);
}

template<> ENGINE_INLINE double3 Mix(const double3& a, const double3& b, const double3& x)
{
	return ymm_mix(a.ymm, b.ymm, x.ymm);
}

template<> ENGINE_INLINE double4 Mix(const double4& a, const double4& b, const double4& x)
{
	return ymm_mix(a.ymm, b.ymm, x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Smoothstep(const T& a, const T& b, const T& x)
{
	T t = Saturate((x - a) / (b - a));

	return t * t * (T(3) - T(2) * t);
}

template<> ENGINE_INLINE float3 Smoothstep(const float3& a, const float3& b, const float3& x)
{
	return xmm_smoothstep(a.xmm, b.xmm, x.xmm);
}

template<> ENGINE_INLINE float4 Smoothstep(const float4& a, const float4& b, const float4& x)
{
	return xmm_smoothstep(a.xmm, b.xmm, x.xmm);
}

template<> ENGINE_INLINE double3 Smoothstep(const double3& a, const double3& b, const double3& x)
{
	return ymm_smoothstep(a.ymm, b.ymm, x.ymm);
}

template<> ENGINE_INLINE double4 Smoothstep(const double4& a, const double4& b, const double4& x)
{
	return ymm_smoothstep(a.ymm, b.ymm, x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Linearstep(const T& a, const T& b, const T& x)
{
	return Saturate((x - a) / (b - a));
}

template<> ENGINE_INLINE float3 Linearstep(const float3& a, const float3& b, const float3& x)
{
	return xmm_linearstep(a.xmm, b.xmm, x.xmm);
}

template<> ENGINE_INLINE float4 Linearstep(const float4& a, const float4& b, const float4& x)
{
	return xmm_linearstep(a.xmm, b.xmm, x.xmm);
}

template<> ENGINE_INLINE double3 Linearstep(const double3& a, const double3& b, const double3& x)
{
	return ymm_linearstep(a.ymm, b.ymm, x.ymm);
}

template<> ENGINE_INLINE double4 Linearstep(const double4& a, const double4& b, const double4& x)
{
	return ymm_linearstep(a.ymm, b.ymm, x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Step(const T& edge, const T& x)
{
	return x < edge ? T(0) : T(1);
}

template<> ENGINE_INLINE float3 Step(const float3& edge, const float3& x)
{
	return xmm_step(edge.xmm, x.xmm);
}

template<> ENGINE_INLINE float4 Step(const float4& edge, const float4& x)
{
	return xmm_step(edge.xmm, x.xmm);
}

template<> ENGINE_INLINE double3 Step(const double3& edge, const double3& x)
{
	return ymm_step(edge.ymm, x.ymm);
}

template<> ENGINE_INLINE double4 Step(const double4& edge, const double4& x)
{
	return ymm_step(edge.ymm, x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Sin(const T& x)
{
	return (T)sin(x);
}

template<> ENGINE_INLINE float3 Sin(const float3& x)
{
	return _mm_sin_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Sin(const float4& x)
{
	return _mm_sin_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Sin(const double3& x)
{
	return _mm256_sin_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Sin(const double4& x)
{
	return _mm256_sin_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Cos(const T& x)
{
	return (T)cos(x);
}

template<> ENGINE_INLINE float3 Cos(const float3& x)
{
	return _mm_cos_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Cos(const float4& x)
{
	return _mm_cos_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Cos(const double3& x)
{
	return _mm256_cos_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Cos(const double4& x)
{
	return _mm256_cos_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T SinCos(const T& x, T* pCos)
{
	*pCos = (T)cos(x);

	return (T)sin(x);
}

template<> ENGINE_INLINE float3 SinCos(const float3& x, float3* pCos)
{
	return _mm_sincos_ps(&pCos->xmm, x.xmm);
}

template<> ENGINE_INLINE float4 SinCos(const float4& x, float4* pCos)
{
	return _mm_sincos_ps(&pCos->xmm, x.xmm);
}

template<> ENGINE_INLINE double3 SinCos(const double3& x, double3* pCos)
{
	return _mm256_sincos_pd(&pCos->ymm, x.ymm);
}

template<> ENGINE_INLINE double4 SinCos(const double4& x, double4* pCos)
{
	return _mm256_sincos_pd(&pCos->ymm, x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Tan(const T& x)
{
	return (T)tan(x);
}

template<> ENGINE_INLINE float3 Tan(const float3& x)
{
	return _mm_tan_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Tan(const float4& x)
{
	return _mm_tan_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Tan(const double3& x)
{
	return _mm256_tan_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Tan(const double4& x)
{
	return _mm256_tan_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Asin(const T& x)
{
	DEBUG_Assert( x >= T(-1) && x <= T(1) );

	return (T)asin(x);
}

template<> ENGINE_INLINE float3 Asin(const float3& x)
{
	DEBUG_Assert( All<CmpGequal>(x, float3(-1.0f)) && All<CmpLequal>(x, float3(1.0f)) );

	return _mm_asin_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Asin(const float4& x)
{
	DEBUG_Assert( All<CmpGequal>(x, float4(-1.0f)) && All<CmpLequal>(x, float4(1.0f)) );

	return _mm_asin_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Asin(const double3& x)
{
	DEBUG_Assert( All<CmpGequal>(x, double3(-1.0)) && All<CmpLequal>(x, double3(1.0)) );

	return _mm256_asin_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Asin(const double4& x)
{
	DEBUG_Assert( All<CmpGequal>(x, double4(-1.0)) && All<CmpLequal>(x, double4(1.0)) );

	return _mm256_asin_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Acos(const T& x)
{
	DEBUG_Assert( x >= T(-1) && x <= T(1) );

	return (T)acos(x);
}

template<> ENGINE_INLINE float3 Acos(const float3& x)
{
	DEBUG_Assert( All<CmpGequal>(x, float3(-1.0f)) && All<CmpLequal>(x, float3(1.0f)) );

	return _mm_acos_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Acos(const float4& x)
{
	DEBUG_Assert( All<CmpGequal>(x, float4(-1.0f)) && All<CmpLequal>(x, float4(1.0f)) );

	return _mm_acos_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Acos(const double3& x)
{
	DEBUG_Assert( All<CmpGequal>(x, double3(-1.0)) && All<CmpLequal>(x, double3(1.0)) );

	return _mm256_acos_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Acos(const double4& x)
{
	DEBUG_Assert( All<CmpGequal>(x, double4(-1.0)) && All<CmpLequal>(x, double4(1.0)) );

	return _mm256_acos_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Atan(const T& x)
{
	return (T)atan(x);
}

template<> ENGINE_INLINE float3 Atan(const float3& x)
{
	return _mm_atan_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Atan(const float4& x)
{
	return _mm_atan_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Atan(const double3& x)
{
	return _mm256_atan_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Atan(const double4& x)
{
	return _mm256_atan_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Atan(const T& y, const T& x)
{
	return (T)atan2(y, x);
}

template<> ENGINE_INLINE float3 Atan(const float3& y, const float3& x)
{
	return _mm_atan2_ps(y.xmm, x.xmm);
}

template<> ENGINE_INLINE float4 Atan(const float4& y, const float4& x)
{
	return _mm_atan2_ps(y.xmm, x.xmm);
}

template<> ENGINE_INLINE double3 Atan(const double3& y, const double3& x)
{
	return _mm256_atan2_pd(y.ymm, x.ymm);
}

template<> ENGINE_INLINE double4 Atan(const double4& y, const double4& x)
{
	return _mm256_atan2_pd(y.ymm, x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Sqrt(const T& x)
{
	DEBUG_Assert( x >= T(0) );

	return (T)sqrt(x);
}

template<> ENGINE_INLINE float3 Sqrt(const float3& x)
{
	return xmm_sqrt(x.xmm);
}

template<> ENGINE_INLINE float4 Sqrt(const float4& x)
{
	return xmm_sqrt(x.xmm);
}

template<> ENGINE_INLINE double3 Sqrt(const double3& x)
{
	return ymm_sqrt(x.ymm);
}

template<> ENGINE_INLINE double4 Sqrt(const double4& x)
{
	return ymm_sqrt(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Rsqrt(const T& x)
{
	DEBUG_Assert( x >= T(0) );

	return T(1) / T(sqrt(x));
}

template<> ENGINE_INLINE float3 Rsqrt(const float3& x)
{
	return xmm_rsqrt(x.xmm);
}

template<> ENGINE_INLINE float4 Rsqrt(const float4& x)
{
	return xmm_rsqrt(x.xmm);
}

template<> ENGINE_INLINE double3 Rsqrt(const double3& x)
{
	return ymm_rsqrt(x.ymm);
}

template<> ENGINE_INLINE double4 Rsqrt(const double4& x)
{
	return ymm_rsqrt(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Pow(const T& x, const T& y)
{
	DEBUG_Assert( x >= T(0) );

	return pow(x, y);
}

template<> ENGINE_INLINE float3 Pow(const float3& x, const float3& y)
{
	return _mm_pow_ps(x.xmm, y.xmm);
}

template<> ENGINE_INLINE float4 Pow(const float4& x, const float4& y)
{
	return _mm_pow_ps(x.xmm, y.xmm);
}

template<> ENGINE_INLINE double3 Pow(const double3& x, const double3& y)
{
	return _mm256_pow_pd(x.ymm, y.ymm);
}

template<> ENGINE_INLINE double4 Pow(const double4& x, const double4& y)
{
	return _mm256_pow_pd(x.ymm, y.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Log(const T& x)
{
	DEBUG_Assert( x >= T(0) );

	return (T)log(x);
}

template<> ENGINE_INLINE float3 Log(const float3& x)
{
	return _mm_log_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Log(const float4& x)
{
	return _mm_log_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Log(const double3& x)
{
	return _mm256_log_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Log(const double4& x)
{
	return _mm256_log_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Exp(const T& x)
{
	return (T)exp(x);
}

template<> ENGINE_INLINE float3 Exp(const float3& x)
{
	return _mm_exp_ps(x.xmm);
}

template<> ENGINE_INLINE float4 Exp(const float4& x)
{
	return _mm_exp_ps(x.xmm);
}

template<> ENGINE_INLINE double3 Exp(const double3& x)
{
	return _mm256_exp_pd(x.ymm);
}

template<> ENGINE_INLINE double4 Exp(const double4& x)
{
	return _mm256_exp_pd(x.ymm);
}

//======================================================================================================================

template<class T> ENGINE_INLINE T Pi(const T& mul)
{
	DEBUG_StaticAssertMsg(false, "Pi::only floating point types are supported!");
	return mul;
}

template<class T> ENGINE_INLINE T RadToDeg(const T& a)
{
	DEBUG_StaticAssertMsg(false, "RadToDeg::only floating point types are supported!");
	return a;
}

template<class T> ENGINE_INLINE T DegToRad(const T& a)
{
	DEBUG_StaticAssertMsg(false, "DegToRad::only floating point types are supported!");
	return a;
}

template<> ENGINE_INLINE float Pi(const float& mul)				{ return mul * acos(-1.0f); }
template<> ENGINE_INLINE float3 Pi(const float3& mul)			{ return mul * acos(-1.0f); }
template<> ENGINE_INLINE float4 Pi(const float4& mul)			{ return mul * acos(-1.0f); }
template<> ENGINE_INLINE double Pi(const double& mul)			{ return mul * acos(-1.0); }
template<> ENGINE_INLINE double3 Pi(const double3& mul)			{ return mul * acos(-1.0); }
template<> ENGINE_INLINE double4 Pi(const double4& mul)			{ return mul * acos(-1.0); }

template<> ENGINE_INLINE float RadToDeg(const float& x)			{ return x * (180.0f / Pi(1.0f)); }
template<> ENGINE_INLINE float3 RadToDeg(const float3& x)		{ return x * (180.0f / Pi(1.0f)); }
template<> ENGINE_INLINE float4 RadToDeg(const float4& x)		{ return x * (180.0f / Pi(1.0f)); }
template<> ENGINE_INLINE double RadToDeg(const double& x)		{ return x * (180.0 / Pi(1.0)); }
template<> ENGINE_INLINE double3 RadToDeg(const double3& x)		{ return x * (180.0 / Pi(1.0)); }
template<> ENGINE_INLINE double4 RadToDeg(const double4& x)		{ return x * (180.0 / Pi(1.0)); }

template<> ENGINE_INLINE float DegToRad(const float& x)			{ return x * (Pi(1.0f) / 180.0f); }
template<> ENGINE_INLINE float3 DegToRad(const float3& x)		{ return x * (Pi(1.0f) / 180.0f); }
template<> ENGINE_INLINE float4 DegToRad(const float4& x)		{ return x * (Pi(1.0f) / 180.0f); }
template<> ENGINE_INLINE double DegToRad(const double& x)		{ return x * (Pi(1.0) / 180.0); }
template<> ENGINE_INLINE double3 DegToRad(const double3& x)		{ return x * (Pi(1.0) / 180.0); }
template<> ENGINE_INLINE double4 DegToRad(const double4& x)		{ return x * (Pi(1.0) / 180.0); }

//======================================================================================================================

template<class T> ENGINE_INLINE void Swap(T& x, T& y)
{
	T t = x;
	x = y;
	y = t;
}

template<> ENGINE_INLINE void Swap(uint& x, uint& y)
{
	x ^= y;
	y ^= x;
	x ^= y;
}

template<class T> ENGINE_INLINE void Swap(T* x, T* y)
{
	// NOTE: just swap memory, skip constructor/destructor...

	const uint N = sizeof(T);
	uchar temp[N];

	memcpy(temp, x, N);
	memcpy(x, y, N);
	memcpy(y, temp, N);
}

//======================================================================================================================
//														ctRect
//======================================================================================================================

template<class T> class ctRect
{
	public:

		union
		{
			struct
			{
				T vMin[COORD_2D];
			};

			struct
			{
				T minx;
				T miny;
			};
		};

		union
		{
			struct
			{
				T vMax[COORD_2D];
			};

			struct
			{
				T maxx;
				T maxy;
			};
		};

	public:

		ENGINE_INLINE ctRect()
		{
			Clear();
		}

		ENGINE_INLINE void Clear()
		{
			minx = miny = T(1 << 30);
			maxx = maxy = T(-(1 << 30));
		}

		ENGINE_INLINE bool IsValid() const
		{
			return maxx > minx && maxy > miny;
		}

		ENGINE_INLINE void Add(T px, T py)
		{
			minx = Min(minx, px);
			maxx = Max(maxx, px);
			miny = Min(miny, py);
			maxy = Max(maxy, py);
		}

		ENGINE_INLINE void Add(const T* pPoint2)
		{
			Add(pPoint2[0], pPoint2[1]);
		}

		ENGINE_INLINE bool IsIntersectWith(const T* pMin, const T* pMax) const
		{
			DEBUG_Assert( IsValid() );

			if( maxx < pMin[0] || maxy < pMin[1] || minx > pMax[0] || miny > pMax[1] )
				return false;

			return true;
		}

		ENGINE_INLINE bool IsIntersectWith(const ctRect<T>& rRect) const
		{
			return IsIntersectWith(rRect.vMin, rRect.vMax);
		}

		ENGINE_INLINE eClip GetIntersectionStateWith(const T* pMin, const T* pMax) const
		{
			DEBUG_Assert( IsValid() );

			if( !IsIntersectWith(pMin, pMax) )
				return CLIP_OUT;

			if( minx < pMin[0] && maxx > pMax[0] && miny < pMin[1] && maxy > pMax[1] )
				return CLIP_IN;

			return CLIP_PARTIAL;
		}

		ENGINE_INLINE eClip GetIntersectionStateWith(const ctRect<T>& rRect) const
		{
			return GetIntersectionStateWith(rRect.vMin, rRect.vMax);
		}
};

//======================================================================================================================
//														Frustum
//======================================================================================================================

class cFrustum
{
	private:

		float4 m_vPlane[PLANES_NUM];
		v4f m_vMask[PLANES_NUM];

	public:

		ENGINE_INLINE cFrustum()
		{
		}

		void Translate(const float3& vPos);

		bool Check(const float3& center, float fRadius) const;
		bool CheckMask(const float3& center, float fRadius, uint mask) const;
		bool Check(const float3& minv, const float3& maxv) const;
		bool CheckMask(const float3& minv, const float3& maxv, uint mask) const;
		bool Check(const float3& capsule_start, const float3& capsule_axis, float capsule_radius) const;

		eClip CheckState(const float3& center, float fRadius) const;
		eClip CheckState(const float3& center, float fRadius, uint& mask) const;
		eClip CheckState(const float3& minv, const float3& maxv) const;
		eClip CheckState(const float3& minv, const float3& maxv, uint& mask) const;
		eClip CheckState(const float3& capsule_start, const float3& capsule_axis, float capsule_radius) const;

		ENGINE_INLINE void SetNearFar(float zNearNeg, float zFarNeg)
		{
			m_vPlane[PLANE_NEAR].w = zNearNeg;
			m_vPlane[PLANE_FAR].w = -zFarNeg;
		}

		ENGINE_INLINE void SetFar(float zFarNeg)
		{
			m_vPlane[PLANE_FAR].w = -zFarNeg;
		}
};

//======================================================================================================================
//														NUMERICAL
//======================================================================================================================

ENGINE_INLINE float SplitZ_Logarithmic(uint i, uint splits, float fZnear, float fZfar)
{
	float ratio = fZfar / fZnear;
	float k = float(i) / float(splits);
	float z = fZnear * Pow(ratio, k);

    return z;
}

ENGINE_INLINE float SplitZ_Uniform(uint i, uint splits, float fZnear, float fZfar)
{
	float delta = fZfar - fZnear;
	float k = float(i) / float(splits);
	float z = fZnear + delta * k;

    return z;
}

ENGINE_INLINE float SplitZ_Mixed(uint i, uint splits, float fZnear, float fZfar, float lambda)
{
	float z_log = SplitZ_Logarithmic(i, splits, fZnear, fZfar);
	float z_uni = SplitZ_Uniform(i, splits, fZnear, fZfar);
	float z = Mix(z_log, z_uni, lambda);

    return z;
}

uint greatest_common_divisor(uint a, uint b);

ENGINE_INLINE uint least_common_multiple(uint a, uint b)
{
	return (a * b) / greatest_common_divisor(a, b);
}

ENGINE_INLINE uint ReverseBits(uint bits)
{
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
	bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);

	return bits;
}

//======================================================================================================================
//														Misc
//======================================================================================================================

// NOTE: 1D cubic interpolator, assumes x > 0.0, constraint parameter = -1
float CubicFilter(float x, float i1, float i2, float i3, float v4i);

void Hammersley(float* pXyz, uint n);

ENGINE_INLINE float2 Hammersley(uint i, uint uiSamples, uint rnd0 = 0, uint rnd1 = 0)
{
	float E1 = Fract( float(i) / float(uiSamples) + float(rnd0 & 0xffff) / float(1 << 16) );
	float E2 = float( ReverseBits(i) ^ rnd1 ) * 2.3283064365386963e-10f;

	return float2(E1, E2);
}

// NOTE: overlapping axis-aligned boundary box and triangle (center - aabb center, extents - half size)
bool IsOverlapBoxTriangle(const float3& center, const float3& extents, const float3& tri0, const float3& tri1, const float3& tri2);

// NOTE: barycentric ray-triangle test by Tomas Akenine-Moller
bool IsIntersectRayTriangle(const float3& origin, const float3& dir, const float3& v1, const float3& v2, const float3& v3, float3& out_tuv);
bool IsIntersectRayTriangle(const float3& from, const float3& to, const float3& v1, const float3& v2, const float3& v3, float3& out_intersection, float3& out_normal);

#include "packed.h"

#endif
