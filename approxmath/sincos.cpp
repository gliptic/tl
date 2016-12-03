#include "am.hpp"

#include "../bits.h"

namespace tl {

static double const PI4A = 7.85398125648498535156E-1;                             // 0x3fe921fb40000000, Pi/4 split into three parts
static double const PI4B = 3.77489470793079817668E-8;                             // 0x3e64442d00000000,
static double const PI4C = 2.69515142907905952645E-15;                            // 0x3ce8469898cc5170,
static double const M4PI = 1.273239544735162542821171882678754627704620361328125; // 4/pi

static double const sintab[6] = {
	1.58962301576546568060E-10, // 0x3de5d8fd1fd19ccd
	-2.50507477628578072866E-8, // 0xbe5ae5e5a9291f5d
	2.75573136213857245213E-6,  // 0x3ec71de3567d48a1
	-1.98412698295895385996E-4, // 0xbf2a01a019bfdf03
	8.33333333332211858878E-3,  // 0x3f8111111110f7d0
	-1.66666666666666307295E-1, // 0xbfc5555555555548
};

// cos coefficients
static double const costab[6] = {
	-1.13585365213876817300E-11, // 0xbda8fa49a0861a9b
	2.08757008419747316778E-9,   // 0x3e21ee9d7b4e3f05
	-2.75573141792967388112E-7,  // 0xbe927e4f7eac4bc6
	2.48015872888517045348E-5,   // 0x3efa01a019c844f5
	-1.38888888888730564116E-3,  // 0xbf56c16c16c14f91
	4.16666666666665929218E-2,   // 0x3fa555555555554b
};

VectorD2 sincos(double x) {
	f64 sin_sign = 1.0, cos_sign = 1.0;

	if (x < 0.0) {
		x = -x;
		sin_sign = -1.0;
	}

	int j = int(x * M4PI);
	j += j & 1;
	f64 y = j;

	j &= 7;
	if (j > 3) { // reflect in x axis
		j -= 4;
		sin_sign = -sin_sign;
		cos_sign = -cos_sign;
	}

	if (j > 1) {
		cos_sign = -cos_sign;
	}
	
	f64 z = ((x - y*PI4A) - y*PI4B) - y*PI4C; // Extended precision modular arithmetic
	f64 zz = z * z;
	f64 cos = 1.0 - 0.5*zz + zz*zz*((((((costab[0]*zz)+costab[1])*zz+costab[2])*zz+costab[3])*zz+costab[4])*zz+costab[5]);
	f64 sin = z + z*zz*((((((sintab[0]*zz)+sintab[1])*zz+sintab[2])*zz+sintab[3])*zz+sintab[4])*zz+sintab[5]);

	if (j == 1 || j == 2) {
		return VectorD2(sin * cos_sign, cos * sin_sign);
	} else {
		return VectorD2(cos * cos_sign, sin * sin_sign);
	}
}

#if 1 // TL_MSVCPP

alignas(16) u8 abs_mask[] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f
};

VectorI2 sincos_fixed(i32 x) {
	f64 const in_scale = 128.0 * 65536.0 / pi2;
	f64 const in_scalesq = in_scale * in_scale;
	f64 const out_scale = 65536.0;

	f64 const c1 = (out_scale * 1.27323954 / in_scale);
	f64 const c2 = (out_scale * -0.405284735 / in_scalesq);

	u64 xx = ((u64(x) << 32) | u64(u32(x) + (32 << 16)));
	__m128i v = _mm_srai_epi32(_mm_slli_epi32(_mm_set_epi64x(0, xx), 9), 9);
	__m128d f = _mm_cvtepi32_pd(v);
	__m128d s1 = _mm_set_pd(c1, c1);
	__m128d s2 = _mm_set_pd(c2, c2);

	auto sc = _mm_mul_pd(f, s1);
	auto af = _mm_and_pd(f, *(__m128d *)&abs_mask);
	auto r = _mm_add_pd(sc, _mm_mul_pd(_mm_mul_pd(f, s2), af));

	auto ri = _mm_cvttpd_epi32(r);

	return VectorI2(ri.m128i_i32[0], ri.m128i_i32[1]);
}

VectorI2 sincos_fixed2(i32 x) {
	// in_scale = 128.0 * 65536.0 / 2pi;
	// in_scalesq = in_scale * in_scale;
	// out_scale = 65536.0;

	// p1 = 1.27323954;
	// p2 = -0.405284735;
	// p3 = .225;

	f64 const c1 = 0.04843749981986151501589604;     // ((1.0 - p3) * out_scale * p1 / in_scale);
	f64 const c2 = -1.1548399937503074015843065e-8;  // ((1.0 - p3) * out_scale * p2 / in_scalesq)
	f64 const c3 = 5.716091636316337148803329864e-6; // (p3 / out_scale) / (1.0 - p3) / (1.0 - p3)

	__m128d s1 = _mm_set_pd(c1, c1);
	__m128d s2 = _mm_set_pd(c2, c2);
	__m128d s3 = _mm_set_pd(c3, c3);
	__m128d abs = *(__m128d *)&abs_mask;

	u64 xx = ((u64(x) << 32) | u64(u32(x) + (32 << 16)));
	__m128i v = _mm_srai_epi32(_mm_slli_epi32(_mm_set_epi64x(0, xx), 9), 9);
	__m128d f = _mm_cvtepi32_pd(v);  // f = (x, x + pi/2)

	auto sc = _mm_mul_pd(f, s1); // sc = s1 * f

	sc = _mm_add_pd(sc, _mm_mul_pd(_mm_mul_pd(f, s2), _mm_and_pd(f, abs))); // sc += s2 * f * abs(f)

	sc = _mm_add_pd(sc, _mm_mul_pd(_mm_mul_pd(sc, _mm_and_pd(sc, abs)), s3)); // r += s3 * r * abs(r)

	auto ri = _mm_cvttpd_epi32(sc);

	return VectorI2(ri.m128i_i32[0], ri.m128i_i32[1]);
}

VectorD2 sincos_f64(i32 x) {
	f64 in_scale = 128.0 * 65536.0 / tl::pi2;
	f64 in_scalesq = in_scale * in_scale;
	f64 out_scale = 65536.0;

	f64 const p1 = 1.27323954;
	f64 const p2 = -0.405284735;
	f64 const p3 = .225;

	f64 const c1 = ((1.0 - p3) * out_scale * p1 / in_scale);
	f64 const c2 = ((1.0 - p3) * out_scale * p2 / in_scalesq);
	f64 const c3 = (p3 / out_scale) / (1.0 - p3) / (1.0 - p3);

	__m128d s1 = _mm_set_pd(c1, c1);
	__m128d s2 = _mm_set_pd(c2, c2);
	__m128d s3 = _mm_set_pd(c3, c3);
	__m128d abs = *(__m128d *)&abs_mask;

	u64 xx = ((u64(x) << 32) | u64(u32(x) + (32 << 16)));
	__m128i v = _mm_srai_epi32(_mm_slli_epi32(_mm_set_epi64x(0, xx), 9), 9);
	__m128d f = _mm_cvtepi32_pd(v);  // f = (x, x + pi/2)

	auto sc = _mm_mul_pd(f, s1); // sc = s1 * f

	sc = _mm_add_pd(sc, _mm_mul_pd(_mm_mul_pd(f, s2), _mm_and_pd(f, abs))); // sc += s2 * f * abs(f)

	sc = _mm_add_pd(sc, _mm_mul_pd(_mm_mul_pd(sc, _mm_and_pd(sc, abs)), s3)); // r += s3 * r * abs(r)

	//auto ri = _mm_cvttpd_epi32(sc);

	return VectorD2(sc.m128d_f64[0], sc.m128d_f64[1]);
}

#else

inline i32 clamp_ang(i32 x) {
	return (x << (16 - 7)) >> (16 - 7);
}

VectorI2 sincos_fixed(i32 x) {
	f64 const in_scale = 128.0 * 65536.0 / pi2;
	f64 const in_scalesq = in_scale * in_scale;
	f64 const out_scale = 65536.0;

	f64 const c1 = (out_scale * 1.27323954 / in_scale);
	f64 const c2 = (out_scale * -0.405284735 / in_scalesq);

	i32 sin_x = clamp_ang(x);
	i32 cos_x = clamp_ang(x + (32 << 16));
	f64 sin_f = sin_x;
	f64 cos_f = cos_x;

	f64 sin = c1 * sin_f;
	f64 cos = c1 * cos_f;

	sin += c2 * sin_f * fabs(sin_f);
	cos += c2 * cos_f * fabs(cos_f);

	return VectorI2(i32(cos), i32(sin));
}

VectorI2 sincos_fixed2(i32 x) {
	// in_scale = 128.0 * 65536.0 / 2pi;
	// in_scalesq = in_scale * in_scale;
	// out_scale = 65536.0;

	// p1 = 1.27323954;
	// p2 = -0.405284735;
	// p3 = .225;

	f64 const c1 = 0.04843749981986151501589604;     // ((1.0 - p3) * out_scale * p1 / in_scale);
	f64 const c2 = -1.1548399937503074015843065e-8;  // ((1.0 - p3) * out_scale * p2 / in_scalesq)
	f64 const c3 = 5.716091636316337148803329864e-6; // (p3 / out_scale) / (1.0 - p3) / (1.0 - p3)

	i32 sin_x = clamp_ang(x);
	i32 cos_x = clamp_ang(x + (32 << 16));
	f64 sin_f = sin_x;
	f64 cos_f = cos_x;

	f64 sin = c1 * sin_f;
	f64 cos = c1 * cos_f;

	sin += c2 * sin_f * fabs(sin_f);
	cos += c2 * cos_f * fabs(cos_f);

	sin += c3 * sin * fabs(sin);
	cos += c3 * cos * fabs(cos);
	
	return VectorI2(i32(cos), i32(sin));
}

#endif

}
