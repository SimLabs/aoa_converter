#pragma once

#include "mathlib.h"

// http://www.cs.rit.edu/usr/local/pub/wrc/courses/cg/doc/opengl/redbook/AppJ.pdf
// http://glm.g-truc.net glm/core/type_half.inl

constexpr unsigned F16_M_BITS = 10;
constexpr unsigned F16_E_BITS = 5;
constexpr unsigned F16_S_MASK = 0x8000;

constexpr unsigned UF11_M_BITS = 6;
constexpr unsigned UF11_E_BITS = 5;
constexpr unsigned UF11_S_MASK = 0x0;

constexpr unsigned UF10_M_BITS = 5;
constexpr unsigned UF10_E_BITS = 5;
constexpr unsigned UF10_S_MASK = 0x0;

// f - float
// h - half
// i - int
// s - short
// b - byte
// u - unsigned

namespace Packed
{

struct float4
{
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

    ENGINE_INLINE float4(float a) : xmm(_mm_broadcast_ss(&a))
    {
    }

    ENGINE_INLINE float4(float a, float b, float c, float d) : xmm(xmm_set_4f(a, b, c, d))
    {
    }

    ENGINE_INLINE float4(float a, float b, float c) : xmm(xmm_set_4f(a, b, c, 1.0f))
    {
    }

    ENGINE_INLINE float4(const float* v4) : xmm(_mm_loadu_ps(v4))
    {
    }

    ENGINE_INLINE float4(const float4& vec) : xmm(vec.xmm)
    {
    }

    ENGINE_INLINE float4(const v4f& m) : xmm(m)
    {
    }
};

// NOTE: pack / unpack float to f16, f11, f10

template<int M_BITS, int E_BITS, int S_MASK> uint ToPacked(float val)
{
	const uint E_MASK = (1 << E_BITS) - 1;
	const uint INF = E_MASK << M_BITS;
	const int BIAS = E_MASK >> 1;
	const int ROUND = 1 << (23 - M_BITS - 1);

	// decompose float
	uint f32 = *(uint*)&val;
	uint packed = (f32 >> 16) & S_MASK;
	int e = ((f32 >> 23) & 0xFF) - 127 + BIAS;
	int m = f32 & 0x007FFFFF;

	if( e == 128 + BIAS )
	{
		// Inf
		packed |= INF;

		if( m )
		{
			// NaN
			m >>= 23 - M_BITS;
			packed |= m | (m == 0);
		}
	}
	else if( e > 0 )
	{
		// round to nearest, round "0.5" up
		if( m & ROUND )
		{
			m += ROUND << 1;

			if( m & 0x00800000 )
			{
				// mantissa overflow
				m = 0;
				e++;
			}
		}
		
		if( e >= E_MASK )
		{
			// exponent overflow - flush to Inf
			packed |= INF;
		}
		else
		{
			// representable value
			m >>= 23 - M_BITS;
			packed |= (e << M_BITS) | m;
		}
	}
	else
	{
		// denormalized or zero
		m = ((m | 0x00800000) >> (1 - e)) + ROUND;
		m >>= 23 - M_BITS;
		packed |= m;
	}

	return packed;
}

template<int M_BITS, int E_BITS, int S_MASK> float FromPacked(uint x)
{
	uFloat f;

	const uint E_MASK = (1 << E_BITS) - 1;
	const int BIAS = E_MASK >> 1;
	const float DENORM_SCALE = 1.0f / (1 << (14 + M_BITS));
	const float NORM_SCALE = 1.0f / float(1 << M_BITS);

	int s = (x & S_MASK) << 15;
	int e = (x >> M_BITS) & E_MASK;
	int m = x & ((1 << M_BITS) - 1);

	if( e == 0 )
		f.f = DENORM_SCALE * m;
	else if( e == E_MASK )
		f.i = s | 0x7F800000 | (m << (23 - M_BITS));
	else
	{
		f.f = 1.0f + float(m) * NORM_SCALE;

		if( e < BIAS )
			f.f /= float(1 << (BIAS - e));
		else
			f.f *= float(1 << (e - BIAS));
	}

	if( s )
		f.f = -f.f;

	return f.f;
}

// NOTE: pack / unpack [0; 1] to uint with required bits

template<uint BITS> uint ToUint(float x)
{
	DEBUG_Assert( x >= 0.0f && x <= 1.0f );

	const float scale = float((1ull << BITS) - 1ull);

	uint y = (uint)Round(x * scale);

	return y;
}

template<uint BITS> float FromUint(uint x)
{
	const float scale = 1.0f / float((1ull << BITS) - 1ull);

	float y = float(x) * scale;

	DEBUG_Assert( y >= 0.0f && y <= 1.0f );

	return y;
}

// NOTE: pack / unpack [-1; 1] to uint with required bits

template<uint BITS> uint ToInt(float x)
{
	DEBUG_Assert( x >= -1.0f && x <= 1.0f );

	const float scale = float((1ull << (BITS - 1ull)) - 1ull);
	const uint mask = uint((1ull << BITS) - 1ull);

	int i = (int)Round(x * scale);
	uint y = i & mask;

	return y;
}

template<uint BITS> float FromInt(uint x)
{
	const uint sign = uint(1ull << (BITS - 1ull));
	const uint range = sign - 1u;
	const float scale = 1.0f / range;

	if( x & sign )
		x |= ~range;

	int i = x;
	float y = Max(float(i) * scale, -1.0f);

	DEBUG_Assert( y >= -1.0f && y <= 1.0f );

	return y;
}

// NOTE: complex packing (unsigned)

template<uint Rbits, uint Gbits, uint Bbits, uint Abits> uint uf4_to_uint(const float4& v)
{
	DEBUG_StaticAssert(Rbits + Gbits + Bbits + Abits <= 32);
	DEBUG_StaticAssert(Rbits && Gbits && Bbits && Abits);

	const uint Rmask = (1 << Rbits) - 1;
	const uint Gmask = (1 << Gbits) - 1;
	const uint Bmask = (1 << Bbits) - 1;
	const uint Amask = (1 << Abits) - 1;

	const uint Gshift = Rbits;
	const uint Bshift = Gshift + Gbits;
	const uint Ashift = Bshift + Bbits;

	const v4f scale = xmm_set_4f(float(Rmask), float(Gmask), float(Bmask), float(Amask));

	v4f t = _mm_mul_ps(v.xmm, scale);
	v4i i = _mm_cvtps_epi32(t);

	uint p = _mm_cvtsi128_si32( i );
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(1, 1, 1, 1)) ) << Gshift;
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(2, 2, 2, 2)) ) << Bshift;
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(3, 3, 3, 3)) ) << Ashift;

	return p;
}

uint uf4_to_packed111110(const float4& v)
{
	const v4f scale = xmm_set_4f(2047., 2047., 1023., 0.0);

	v4f t = _mm_mul_ps(v.xmm, scale);
	v4i i = _mm_cvtps_epi32(t);

	uint p = _mm_cvtsi128_si32(i);
	p |= _mm_cvtsi128_si32(_mm_shuffle_epi32(i, _MM_SHUFFLE(1, 1, 1, 1))) << 11;
	p |= _mm_cvtsi128_si32(_mm_shuffle_epi32(i, _MM_SHUFFLE(2, 2, 2, 2))) << 22;
	return p;
}

template<> uint uf4_to_uint<8, 8, 8, 8>(const float4& v)
{
	v4f t = _mm_mul_ps(v.xmm, _mm_set1_ps(255.0f));
	v4i i = _mm_cvtps_epi32(t);
	i = _mm_shuffle_epi8(i, _mm_set1_epi32(0x0C080400));

	return _mm_cvtsi128_si32(i);
}

uint uf2_to_uint1616(float x, float y)
{
	v4f t = xmm_set_4f(x, y, 0.0f, 0.0f);
	t = _mm_mul_ps(t, _mm_set1_ps(65535.0f));
	v4i i = _mm_cvtps_epi32(t);

	uint p = _mm_cvtsi128_si32( i );
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(1, 1, 1, 1)) ) << 16;

	return p;
}

uint uf3_to_packed111110(const float3& v)
{
	DEBUG_Assert( v.x >= 0.0f && v.y >= 0.0f && v.z >= 0.0f );

	uint r = ToPacked<UF11_M_BITS, UF11_E_BITS, UF11_S_MASK>(v.x);
	r |= ToPacked<UF11_M_BITS, UF11_E_BITS, UF11_S_MASK>(v.y) << 11;
	r |= ToPacked<UF10_M_BITS, UF10_E_BITS, UF10_S_MASK>(v.z) << 22;

	return r;
}

// NOTE: complex packing (signed)

template<uint Rbits, uint Gbits, uint Bbits, uint Abits> uint sf4_to_int(const float4& v)
{
	DEBUG_StaticAssert( Rbits + Gbits + Bbits + Abits <= 32 );

	const uint Rmask = (1 << Rbits) - 1;
	const uint Gmask = (1 << Gbits) - 1;
	const uint Bmask = (1 << Bbits) - 1;
	const uint Amask = (1 << Abits) - 1;

	const uint Gshift = Rbits;
	const uint Bshift = Gshift + Gbits;
	const uint Ashift = Bshift + Bbits;

	const uint Rrange = (1 << (Rbits - 1)) - 1;
	const uint Grange = (1 << (Gbits - 1)) - 1;
	const uint Brange = (1 << (Bbits - 1)) - 1;
	const uint Arange = (1 << (Abits - 1)) - 1;
	
	const v4f scale = xmm_set_4f(float(Rrange), float(Grange), float(Brange), float(Arange));
	const v4i mask = _mm_setr_epi32(Rmask, Gmask, Bmask, Amask);

	v4f t = _mm_mul_ps(v.xmm, scale);
	v4i i = _mm_cvtps_epi32(t);
	i = _mm_and_si128(i, mask);

	uint p = _mm_cvtsi128_si32( i );
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(1, 1, 1, 1)) ) << Gshift;
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(2, 2, 2, 2)) ) << Bshift;
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(3, 3, 3, 3)) ) << Ashift;

	return p;
}

template<> uint sf4_to_int<8, 8, 8, 8>(const float4& v)
{
	v4f t = _mm_mul_ps(v.xmm, _mm_set1_ps(127.0f));
	v4i i = _mm_cvtps_epi32(t);
	i = _mm_shuffle_epi8(i, _mm_set1_epi32(0x0C080400));

	return _mm_cvtsi128_si32(i);
}

uint sf2_to_int1616(float x, float y)
{
	v4f t = xmm_set_4f(x, y, 0.0f, 0.0f);
	t = _mm_mul_ps(t, _mm_set1_ps(32767.0f));
	v4i i = _mm_cvtps_epi32(t);
	i = _mm_and_si128(i, _mm_setr_epi32(65535, 65535, 0, 0));

	uint p = _mm_cvtsi128_si32( i );
	p |= _mm_cvtsi128_si32( _mm_shuffle_epi32(i, _MM_SHUFFLE(1, 1, 1, 1)) ) << 16;

	return p;
}

uint sf2_to_h2(float x, float y)
{
	#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_AVX1 )

		v4f v = xmm_set_4f(x, y, 0.0f, 0.0f);
		v4i p = xmm_to_h4(v);

		uint r = _mm_cvtsi128_si32(p);

	#else
		
		uint r = ToPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(x);
		r |= ToPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(y) << 16;

	#endif

	return r;
}

void sf4_to_h4(const float4& v, uint* pu2)
{
	#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_AVX1 )

		v4i p = xmm_to_h4(v.xmm);

		//*(v2i*)pu2 = _mm_movepi64_pi64(p);
		_mm_storel_epi64((v4i*)pu2, p);

	#else

		pu2[0] = Packed::sf2_to_h2(v.x, v.y);
		pu2[1] = Packed::sf2_to_h2(v.z, v.w);

	#endif
}

// NOTE: complex unpacking (unsigned)

template<uint Rbits, uint Gbits, uint Bbits, uint Abits> float4 uint_to_uf4(uint p)
{
	DEBUG_StaticAssert( Rbits + Gbits + Bbits + Abits <= 32 );

	const uint Rmask = (1 << Rbits) - 1;
	const uint Gmask = (1 << Gbits) - 1;
	const uint Bmask = (1 << Bbits) - 1;
	const uint Amask = (1 << Abits) - 1;

	const uint Gshift = Rbits;
	const uint Bshift = Gshift + Gbits;
	const uint Ashift = Bshift + Bbits;

	const v4f scale = xmm_set_4f(1.0f / Rmask, 1.0f / Gmask, 1.0f / Bmask, 1.0f / Amask);

	v4i i = _mm_setr_epi32(p & Rmask, (p >> Gshift) & Gmask, (p >> Bshift) & Bmask, (p >> Ashift) & Amask);
	v4f t = _mm_cvtepi32_ps(i);
	t = _mm_mul_ps(t, scale);

	return t;
}

template<> float4 uint_to_uf4<8, 8, 8, 8>(uint p)
{
	#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_SSE4 )
		v4i i = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(p));
	#else
		v4i i = _mm_set_epi32(p >> 24, (p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF);
	#endif

	v4f t = _mm_cvtepi32_ps(i);
	t = _mm_mul_ps(t, _mm_set1_ps(1.0f / 255.0f));

	return t;
}

float3 packed111110_to_uf3(uint p)
{
	float3 v;
	v.x = FromPacked<UF11_M_BITS, UF11_E_BITS, UF11_S_MASK>( p & ((1 << 11) - 1) );
	v.y = FromPacked<UF11_M_BITS, UF11_E_BITS, UF11_S_MASK>( (p >> 11) & ((1 << 11) - 1) );
	v.z = FromPacked<UF10_M_BITS, UF10_E_BITS, UF10_S_MASK>( (p >> 22) & ((1 << 10) - 1) );

	return v;
}

template<uint Rbits, uint Gbits, uint Bbits, uint Abits> void uint_to_4ui(uint p, int* v)
{
	DEBUG_StaticAssert( Rbits + Gbits + Bbits + Abits <= 32 );

	const uint Rmask = (1 << Rbits) - 1;
	const uint Gmask = (1 << Gbits) - 1;
	const uint Bmask = (1 << Bbits) - 1;
	const uint Amask = (1 << Abits) - 1;

	const uint Gshift = Rbits;
	const uint Bshift = Gshift + Gbits;
	const uint Ashift = Bshift + Bbits;

	v[0] = p & Rmask;
	v[1] = (p >> Gshift) & Gmask;
	v[2] = (p >> Bshift) & Bmask;
	v[3] = (p >> Ashift) & Amask;
}

// NOTE: complex unpacking (signed)

template<uint Rbits, uint Gbits, uint Bbits, uint Abits> float4 int_to_sf4(uint p)
{
	DEBUG_StaticAssert( Rbits + Gbits + Bbits + Abits <= 32 );

	const uint Rmask = (1 << Rbits) - 1;
	const uint Gmask = (1 << Gbits) - 1;
	const uint Bmask = (1 << Bbits) - 1;
	const uint Amask = (1 << Abits) - 1;

	const uint Gshift = Rbits;
	const uint Bshift = Gshift + Gbits;
	const uint Ashift = Bshift + Bbits;

	const uint Rsign = (1 << (Rbits - 1));
	const uint Gsign = (1 << (Gbits - 1));
	const uint Bsign = (1 << (Bbits - 1));
	const uint Asign = (1 << (Abits - 1));
	
	const v4i sign = _mm_setr_epi32(Rsign, Gsign, Bsign, Asign);
	const v4i or = _mm_setr_epi32(~(Rsign - 1), ~(Gsign - 1), ~(Bsign - 1), ~(Asign - 1));
	const v4f scale = xmm_set_4f(1.0f / (Rsign - 1), 1.0f / (Gsign - 1), 1.0f / (Bsign - 1), 1.0f / (Asign - 1));

	v4i i = _mm_setr_epi32(p & Rmask, (p >> Gshift) & Gmask, (p >> Bshift) & Bmask, (p >> Ashift) & Amask);

	v4i mask = _mm_and_si128(i, sign);
	v4i ii = _mm_or_si128(i, or);
	i = xmmi_select(i, ii, _mm_cmpeq_epi32(mask, _mm_setzero_si128()));

	v4f t = _mm_cvtepi32_ps(i);
	t = _mm_mul_ps(t, scale);
	t = _mm_max_ps(t, _mm_set1_ps(-1.0f));

	return t;
}

template<> float4 int_to_sf4<8, 8, 8, 8>(uint p)
{
	#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_SSE4 )
		v4i i = _mm_cvtepi8_epi32(_mm_cvtsi32_si128(p));
	#else
		v4i i = _mm_set_epi32(char(p >> 24), char((p >> 16) & 0xFF), char((p >> 8) & 0xFF), char(p & 0xFF));
	#endif

	v4f t = _mm_cvtepi32_ps(i);
	t = _mm_mul_ps(t, _mm_set1_ps(1.0f / 127.0f));
	t = _mm_max_ps(t, _mm_set1_ps(-1.0f));

	return t;
}

float2 h2_to_sf2(uint ui)
{
	float2 r;

	#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_AVX1 )

		v4i p = _mm_cvtsi32_si128(ui);
		v4f f = _mm_cvtph_ps(p);

		_mm_storel_pi(&r.mm, f);

	#else

		r.x = FromPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(ui & 0xFFFF);
		r.y = FromPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(ui >> 16);

	#endif

	return r;
}

float4 h4_to_sf4(const uint* pu2)
{
	float4 f;

	#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_AVX1 )

		v4i p = _mm_loadl_epi64((const v4i*)pu2);
		f.xmm = _mm_cvtph_ps(p);

	#else

		f.x = FromPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(pu2[0] & 0xFFFF);
		f.y = FromPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(pu2[0] >> 16);
		f.z = FromPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(pu2[1] & 0xFFFF);
		f.w = FromPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(pu2[1] >> 16);

	#endif

	return f;
}

template<uint Rbits, uint Gbits, uint Bbits, uint Abits> void int_to_4i(uint p, int* v)
{
	DEBUG_StaticAssert( Rbits + Gbits + Bbits + Abits <= 32 );

	const uint Rmask = (1 << Rbits) - 1;
	const uint Gmask = (1 << Gbits) - 1;
	const uint Bmask = (1 << Bbits) - 1;
	const uint Amask = (1 << Abits) - 1;

	const uint Gshift = Rbits;
	const uint Bshift = Gshift + Gbits;
	const uint Ashift = Bshift + Bbits;

	const uint Rsign = (1 << (Rbits - 1));
	const uint Gsign = (1 << (Gbits - 1));
	const uint Bsign = (1 << (Bbits - 1));
	const uint Asign = (1 << (Abits - 1));
	
	const v4i sign = _mm_setr_epi32(Rsign, Gsign, Bsign, Asign);
	const v4i or = _mm_setr_epi32(~(Rsign - 1), ~(Gsign - 1), ~(Bsign - 1), ~(Asign - 1));

	v4i i = _mm_setr_epi32(p & Rmask, (p >> Gshift) & Gmask, (p >> Bshift) & Bmask, (p >> Ashift) & Amask);

	v4i mask = _mm_and_si128(i, sign);
	v4i ii = _mm_or_si128(i, or);
	i = xmmi_select(i, ii, _mm_cmpeq_epi32(mask, _mm_setzero_si128()));

	_mm_storeu_si128((v4i*)v, i);
}

};

struct half_float
{
	ushort us;

	half_float()
	{
	}

	half_float(float x)
	{
		#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_AVX1 )

			v4f v = _mm_set_ss(x);
			v4i p = xmm_to_h4(v);

			us = (ushort)_mm_cvtsi128_si32(p);

		#else
		
			us = (ushort)Packed::ToPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(x);

		#endif
	}

	half_float(ushort x) :
		us(x)
	{
	}

	operator float() const
	{
		#if( ENGINE_INTRINSIC >= ENGINE_INTRINSIC_AVX1 )

			v4i p = _mm_cvtsi32_si128(us);
			v4f f = _mm_cvtph_ps(p);
			
			return _mm_cvtss_f32(f);

		#else

			return Packed::FromPacked<F16_M_BITS, F16_E_BITS, F16_S_MASK>(us);

		#endif
	}
};