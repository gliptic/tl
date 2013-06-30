#ifndef UUID_FDA91F49FD8F423968AA34AD0C51384B
#define UUID_FDA91F49FD8F423968AA34AD0C51384B

#include "config.h"
#include "platform.h"
#include <math.h>

#if !defined(TL_FORCE_SSE_FPU) && TL_FORCE_SSE2_FPU
#define TL_FORCE_SSE_FPU 1
#endif

#if TL_MSVCPP
/* We may want to use intrinsics */
#include <emmintrin.h>
//#pragma intrinsic(_mm_mul_sd, _mm_div_sd, _mm_mul_ss, _mm_div_ss)
#endif

#if TL_CPP
extern "C" {
#endif

#if TL_MSVCPP
#pragma fp_contract(off)
#pragma fenv_access(on)
#pragma float_control(except, off)
#pragma float_control(precise, on)
/* NOTE: For VC++, use these flags:
** For CPUs supporting SSE2: /arch:SSE2 /D "TL_X87=0" /D "TL_FORCE_SSE2=1"
** For compatibility: /D "TL_X87=1"
**
** The "TL_FORCE_SSE2=1" define may be omitted if you're absolutely sure
** that no multiplications or divisions are done in x87. It may worsen some
** optimizations if enabled.
** With the above pragmas and operations wrapped in functions, VC++ has a
** strong aversion towards generating x87 instructions for any floating point
** operation, but one can never be sure. The easiest way to verify is
** searching assembler output for "fi?mulp?" and "fi?divp?" (regex) and make
** sure nothing is found.
*/
#elif TL_GCC
/* NOTE: For gcc on x86, use these flags:
** For CPUs supporting SSE2: -frounding-math -msse2 -mfpmath=sse -DTL_X87=0
** For compatibility: -frounding-math -DTL_X87=1
**
** Flags that are recommended:
**     -fno-math-errno
**
**         Some platforms do not set errno anyway, so it's best if it's not set anywhere.
**         This can also increase performance.
*/
#endif

/* Setup FPU for IEEE compliant operation. Allows
** the g* functions to produce reproducible results. NOTE: If this
** function is used, optimizations that assume that certain FPU flags are
** set need to be disabled.
*/
TL_IEEE754_API void tl_init_ieee();

TL_API unsigned char const tl_scaleup[10];
TL_API unsigned char const tl_scaledown[10];

#if TL_X87

TL_INLINE double gM(double x, double y) {
#if TL_MSVCPP
	/* To avoid incorrectly rounded subnormals due to
	** double rounding, we must force double precision
	** subnormals to also be extended precision
	** subnormals after the multiply. We do that by
	** multiplying one of the operands with 1 / 2^(16383 - 1023)
	** before the multiply and multiply the result
	** with 2^(16383 - 1023) afterwards.
	*/
	double r;

	__asm
	{
		fld y /* y = ST(0) */
		fld x /* x = ST(0), y = ST(1) */
		fld TBYTE PTR tl_scaledown /* tl_scaledown = ST(0), x = ST(1), y = ST(2) */
		fmulp ST(1), ST(0)      /* x*tl_scaledown = ST(0), y = ST(1) */
		fmulp ST(1), ST(0)      /* (x*tl_scaledown)*y = ST(0) */
		fld TBYTE PTR tl_scaleup   /* tl_scaleup = ST(0), (x*tl_scaledown)*y = ST(1) */
		fmulp ST(1), ST(0)      /* ((x*tl_scaledown)*y)*tl_scaleup = ST(0) */
		fstp r                  /* r = R(((x*tl_scaledown)*y)*tl_scaleup) */
	}

	return r;
#elif TL_GCC
	/* GCC supports 80-bit floats, so we can use them instead of
	** inline asm to hopefully get a bit better code generation. */
	long double xe, ye;
#if !TL_GCC_FLOAT_STORE && !TL_TRUST_GCC_ROUND_FROM_LONG_DOUBLE /* We can skip volatile if we can trust that GCC rounds the long double to double assignment */
	volatile /* volatile to force store and load */
#endif
	double res;
	xe = x;
	ye = y;

	res = ((xe * *((long double*)tl_scaledown)) * ye) * (*(long double*)tl_scaleup);
	return res;
#else
#error "Not implemented"
#endif
}

TL_INLINE double gD(double x, double y) {
#if TL_MSVCPP
	/* Similarly to the case with gM, we do this
	** to avoid incorrectly rounded subnormals. */
	double r;

	__asm
	{
		fld TBYTE PTR tl_scaledown /* tl_scaledown = ST(0) */
		fmul x                  /* x*tl_scaledown = ST(0) */
		fdiv y                  /* (x*tl_scaledown)/y = ST(0) */
		fld TBYTE PTR tl_scaleup   /* tl_scaleup = ST(0), (x*tl_scaledown)/y = ST(1) */
		fmulp ST(1), ST(0)      /* ((x*tl_scaledown)/y)*tl_scaleup = ST(0) */
		fstp r                  /* r = R(((x*tl_scaledown)/y)*tl_scaleup) */
	}

	return r;
#elif TL_GCC
	/* GCC supports 80-bit floats, so we can use them instead of
	** inline asm to hopefully get a bit better code generation. */
	long double xe, ye;
#if !TL_GCC_FLOAT_STORE && !TL_TRUST_GCC_ROUND_FROM_LONG_DOUBLE /* We can skip volatile if we can trust that GCC rounds the long double to double assignment */
	volatile /* volatile to force store and load */
#endif
	double res; /* volatile to force store and load */
	xe = x;
	ye = y;

	res = ((xe * *((long double*)tl_scaledown)) / ye) * (*(long double*)tl_scaleup);
	return res;
#else
#error "Not implemented"
#endif
}

/* Addition and subtraction do not suffer from incorrectly rounded
** subnormals. */

TL_INLINE double gA(double x, double y) {
#if TL_GCC
#if !TL_GCC_FLOAT_STORE
	volatile /* volatile to force store and load */
#endif
	double res;
	res = x + y;
	return res;
#else
	return x + y;
#endif
}

TL_INLINE double gS(double x, double y) {
#if TL_GCC
#if !TL_GCC_FLOAT_STORE
	volatile /* volatile to force store and load */
#endif
	double res;
	res = x - y;
	return res;
#else
	return x - y;
#endif
}

TL_INLINE double gSqrt(double x) {
#if TL_GCC
#if !TL_GCC_FLOAT_STORE
	volatile /* volatile to force store and load */
#endif
	double res;
	res = sqrt(x);
	return res;
#else
	return sqrt(x);
#endif
}

#else /* if !TL_X87 */

TL_FORCE_INLINE double gM(double x, double y) {
#if TL_MSVCPP && TL_FORCE_SSE2_FPU && !TL_X86_64
	/* Use SSE2 intrinsics to make sure VC++ doesn't cause
	** incorrect subnormal rounding here. NOTE: This might worsen optimization
	** somewhat. */
	double r;
	_mm_store_sd(&r, _mm_mul_sd(_mm_load_sd(&x), _mm_load_sd(&y)));
	return r;
#else
	return x * y;
#endif
}

TL_FORCE_INLINE double gD(double x, double y) {
#if TL_MSVCPP && TL_FORCE_SSE2_FPU && !TL_X86_64
	/* Use SSE2 intrinsics to make sure VC++ doesn't cause
	** incorrect subnormal rounding here. NOTE: This might worsen optimization
	** somewhat. */
	double r;
	_mm_store_sd(&r, _mm_div_sd(_mm_load_sd(&x), _mm_load_sd(&y)));
	return r;
#else
	return x / y;
#endif
}

TL_INLINE double gA(double x, double y) {
	/* We don't force SSE2 for VC++ here even with TL_FORCE_SSE2_FPU, because
	** addition doesn't suffer from incorrect subnormal rounding. */
	double res;
	res = x + y;
	return res;
}

TL_INLINE double gS(double x, double y) {
	/* We don't force SSE2 for VC++ here, because addition doesn't suffer from
	** incorrect subnormal rounding. */
	double res;
	res = x - y;
	return res;
}

TL_INLINE double gSqrt(double x) {
	double res;
	res = sqrt(x);
	return res;
}

// NOTE: g*f variants are currently only available with SSE or SSE2 enabled and
// not with x87 (except for gSqrtf).


TL_INLINE float gMf(float x, float y) {
#if TL_MSVCPP && TL_FORCE_SSE2_FPU && !TL_X86_64
	/* Use SSE intrinsics to make sure VC++ doesn't cause
	** incorrect subnormal rounding here. NOTE: This might worsen optimization
	** somewhat. */
	float r;
	_mm_store_ss(&r, _mm_mul_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
	return r;
#else
	return x * y;
#endif
}

TL_INLINE float gDf(float x, float y) {
#if TL_MSVCPP && TL_FORCE_SSE_FPU && !TL_X86_64
	/* Use SSE intrinsics to make sure VC++ doesn't cause
	** incorrect subnormal rounding here. NOTE: This might worsen optimization
	** somewhat. */
	float r;
	_mm_store_ss(&r, _mm_div_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
	return r;
#else
	return x / y;
#endif
}

TL_INLINE float gAf(float x, float y) {
#if TL_MSVCPP && TL_FORCE_SSE_FPU && !TL_X86_64
	// VC++ generates stupid code for just (x+y) in x86. Intrinsics are actually faster!
	float r;
	_mm_store_ss(&r, _mm_add_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
	return r;
#else
	return x + y;
#endif
}

TL_INLINE float gSf(float x, float y) {
#if TL_MSVCPP && TL_FORCE_SSE_FPU && !TL_X86_64
	// VC++ generates stupid code for just (x-y) in x86. Intrinsics are actually faster!
	float r;
	_mm_store_ss(&r, _mm_sub_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
	return r;
#else
	return x - y;
#endif
}

#endif // !x87

TL_INLINE float gSqrtf(float x) {
#if TL_MSVCPP
	float r;
	_mm_store_ss(&r, _mm_sqrt_ss(_mm_load_ss(&x)));
	return r;
#else
	float r;
	r = sqrtf(x);
	return r;
#endif
}

#define gM3(x,y,z) gM(gM((x),(y)),(z))
#define gD3(x,y,z) gD(gD((x),(y)),(z))
#define gA3(x,y,z) gA(gA((x),(y)),(z))
#define gS3(x,y,z) gS(gS((x),(y)),(z))

TL_INLINE float gdtof(double x) {
	return (float)x;
}

#if TL_CPP

} // extern "C"

namespace tl {

// C++ specific features
struct rdouble {
	rdouble()
	: value(0.0) {
	}
	
	rdouble(double value)
	: value(value) {

	}

	rdouble& operator+=(rdouble b) {
		value = gA(value, b.value);
		return *this;
	}

	rdouble& operator-=(rdouble b) {
		value = gS(value, b.value);
		return *this;
	}

	rdouble& operator*=(rdouble b) {
		value = gM(value, b.value);
		return *this;
	}

	rdouble& operator/=(rdouble b) {
		value = gD(value, b.value);
		return *this;
	}

	rdouble operator-() const
	{ return -value; }

	double value;
};

inline rdouble operator+(rdouble a, rdouble b)
{ return gA(a.value, b.value); }

inline rdouble operator-(rdouble a, rdouble b)
{ return gS(a.value, b.value); }

inline rdouble operator*(rdouble a, rdouble b)
{ return gM(a.value, b.value); }

inline rdouble operator/(rdouble a, rdouble b)
{ return gD(a.value, b.value); }

inline bool operator<(rdouble a, rdouble b)
{ return a.value < b.value; }

inline bool operator<=(rdouble a, rdouble b)
{ return a.value <= b.value; }

inline bool operator>(rdouble a, rdouble b)
{ return a.value > b.value; }

inline bool operator>=(rdouble a, rdouble b)
{ return a.value >= b.value; }

inline bool operator!=(rdouble a, rdouble b)
{ return a.value != b.value; }

inline bool operator==(rdouble a, rdouble b)
{ return a.value == b.value; }

inline rdouble sqrt(rdouble x)
{ return gSqrt(x.value); }

#if !TL_X87

struct rfloat {
	rfloat()
	//: value(0.f)
	{
	}
	
	rfloat(float value)
	: value(value) {

	}

	explicit rfloat(double value_d)
	: value(gdtof(value_d)) {
	}


	float value;
};


TL_FORCE_INLINE rfloat& operator+=(rfloat& a, rfloat const& b) {
	a.value = gAf(a.value, b.value);
	return a;
}

TL_FORCE_INLINE rfloat& operator-=(rfloat& a, rfloat const& b) {
	a.value = gSf(a.value, b.value);
	return a;
}

TL_FORCE_INLINE rfloat& operator*=(rfloat& a, rfloat const& b) {
	a.value = gMf(a.value, b.value);
	return a;
}

TL_FORCE_INLINE rfloat& operator/=(rfloat& a, rfloat const& b) {
	a.value = gDf(a.value, b.value);
	return a;
}

TL_FORCE_INLINE rfloat operator-(rfloat const& a)
{ return -a.value; }

TL_FORCE_INLINE rfloat operator+(rfloat const& a, rfloat const& b)
{ return gAf(a.value, b.value); }

TL_FORCE_INLINE rfloat operator-(rfloat const& a, rfloat const& b)
{ return gSf(a.value, b.value); }

TL_FORCE_INLINE rfloat operator*(rfloat const& a, rfloat const& b)
{ return gMf(a.value, b.value); }

TL_FORCE_INLINE rfloat operator/(rfloat const& a, rfloat const& b)
{ return gDf(a.value, b.value); }

TL_FORCE_INLINE bool operator<(rfloat const& a, rfloat const& b)
{ return a.value < b.value; }

TL_FORCE_INLINE bool operator<=(rfloat const& a, rfloat const& b)
{ return a.value <= b.value; }

TL_FORCE_INLINE bool operator>(rfloat const& a, rfloat const& b)
{ return a.value > b.value; }

TL_FORCE_INLINE bool operator>=(rfloat const& a, rfloat const& b)
{ return a.value >= b.value; }

TL_FORCE_INLINE bool operator!=(rfloat const& a, rfloat const& b)
{ return a.value != b.value; }

TL_FORCE_INLINE bool operator==(rfloat const& a, rfloat const& b)
{ return a.value == b.value; }


TL_FORCE_INLINE rfloat sqrt(rfloat x)
{ return gSqrtf(x.value); }

#endif

} // namespace tl

#if !TL_X87
inline float to_float(tl::rfloat v)
{ return v.value; }

inline float to_float(float v)
{ return v; }
#endif

#endif // TL_CPP

#endif /* UUID_FDA91F49FD8F423968AA34AD0C51384B */

