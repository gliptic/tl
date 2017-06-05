#ifndef UUID_644FCF12367F4D475780169F9A4EAE7C
#define UUID_644FCF12367F4D475780169F9A4EAE7C

#include <string.h>
#include "platform.h"
#include "cstdint.h"

#if TL_MSVCPP
# include <intrin.h>
# include <emmintrin.h>
# pragma intrinsic(_BitScanReverse)
# pragma intrinsic(_BitScanForward)

# if TL_X86_64
#  pragma intrinsic(_BitScanReverse64)
#  pragma intrinsic(_BitScanForward64)
# endif

unsigned short   _byteswap_ushort(unsigned short   val);
unsigned long    _byteswap_ulong (unsigned long    val);
unsigned __int64 _byteswap_uint64(unsigned __int64 val);

# define tl_byteswap16(x) _byteswap_ushort(x)
# define tl_byteswap32(x) _byteswap_ulong(x)
# define tl_byteswap64(x) _byteswap_uint64(x)

#endif

#if TL_CPP
extern "C" {
#endif

#if TL_MSVCPP

TL_INLINE i32 tl_ffs(u32 x) {
	unsigned long r; _BitScanForward(&r, x); return r;
}

TL_INLINE i32 tl_fls(u32 x) {
	unsigned long r; _BitScanReverse(&r, x); return r;
}

#if TL_X86_64
TL_INLINE i32 tl_fls64(u64 x) {
	unsigned long r; _BitScanReverse64(&r, x); return r;
}
# define TL_HAS_FLS64 1
#endif

TL_INLINE i32 tl_log2(u32 v) {
	unsigned long r;
	if(!_BitScanReverse(&r, v))
		r = 0;
	return r;
}

TL_INLINE i32 tl_top_bit(u32 v) {
	unsigned long r;
	if(!_BitScanReverse(&r, v))
		return -1;
	return r;
}

TL_INLINE i32 tl_bottom_bit(u32 v) {
	unsigned long r;
	if(!_BitScanForward(&r, v))
		return -1;
	return r;
}

#if TL_X86_64
TL_INLINE i32 tl_top_bit64(u64 v) {
	unsigned long r;
	if (!_BitScanReverse64(&r, v))
		return -1;
	return r;
}
# define TL_HAS_TOP_BIT64 1
#endif

#else // if !TL_MSVCPP

TL_INLINE uint16_t tl_byteswap16(uint16_t x) { return (x << 8) | (x >> 8); }
TL_INLINE u32 tl_byteswap32(u32 x) { return (x << 24) | (x >> 24) | ((x >> 8) & 0xff00) | ((x & 0xff00) << 8); }
// TODO: tl_byteswap64

// TODO: tl_log2, tl_top_bit, tl_bottom_bit, tl_ffs, tl_fls

#endif // elseif !TL_MSVCPP

#ifndef TL_HAS_FLS64
TL_INLINE i32 tl_fls64(u64 x) {
	return (x>>32) ? 32 + (i32)tl_fls((u32)(x>>32)) : (i32)tl_fls((u32)x);
}
#endif

#ifndef TL_HAS_TOP_BIT64
TL_INLINE i32 tl_top_bit64(u64 x) {
	return (x >> 32) ? 32 + (i32)tl_top_bit((u32)(x >> 32)) : (i32)tl_top_bit((u32)x);
}
#endif

TL_INLINE int tl_reverse_bits16(int n) {
	n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
	n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
	n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
	n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
	return n;
}

TL_INLINE unsigned int tl_reverse_bits32(unsigned int n) {
	n = ((n & 0xAAAAAAAA) >>  1) | ((n & 0x55555555) << 1);
	n = ((n & 0xCCCCCCCC) >>  2) | ((n & 0x33333333) << 2);
	n = ((n & 0xF0F0F0F0) >>  4) | ((n & 0x0F0F0F0F) << 4);
	n = ((n & 0xFF00FF00) >>  8) | ((n & 0x00FF00FF) << 8);
	return (n >> 16) | (n << 16);
}

#if TL_BIG_ENDIAN
# define tl_le64(x) tl_byteswap64(x)
# define tl_le32(x) tl_byteswap32(x)
# define tl_le16(x) tl_byteswap16(x)
// TODO: tl_lesize
# define tl_be64(x) (x)
# define tl_be32(x) (x)
# define tl_be16(x) (x)
# define tl_besize(x) (x)
# define TL_ENDIAN_LOHI(lo, hi) hi lo
#else
# define tl_le64(x) (x)
# define tl_le32(x) (x)
# define tl_le16(x) (x)
# define tl_lesize(x) (x)
# define tl_be64(x) tl_byteswap64(x)
# define tl_be32(x) tl_byteswap32(x)
# define tl_be16(x) tl_byteswap16(x)
// TODO: tl_besize
# define TL_ENDIAN_LOHI(lo, hi) lo hi
#endif

TL_INLINE u32 tl_rol32(u32 x, int s) {
	return (x << s) | (x >> (32-s));
}

TL_INLINE u32 tl_ror32(u32 x, int s) {
	return (x >> s) | (x << (32-s));
}

TL_INLINE u32 tl_ftourep(float f) {
	u32 v;
	memcpy(&v, &f, sizeof(u32));
	return v;
}

TL_INLINE f32 tl_ureptof(u32 u) {
	f32 v;
	memcpy(&v, &u, sizeof(f32));
	return v;
}

TL_INLINE u64 tl_dtourep(double* f) {
	u64 v;
	memcpy(&v, f, sizeof(u64));
	return v;
}

TL_INLINE double tl_ureptod(u64 u) {
	f64 v;
	memcpy(&v, &u, sizeof(f64));
	return v;
}

u32 tl_popcount(u32 a);

#if TL_CPP

} // extern "C"

namespace tl {

template<typename To, typename From>
To narrow(From v) {
	assert((From)(To)v == v);
	return (To)v;
}

template<typename To, typename From>
inline To from_bits(From v);

inline u64 bits_of(f64 v) {
#if TL_MSVCPP
	// Wow, this doesn't actually spill to memory in VC++!
	__m128d x = _mm_set_sd(v);
	return ((__m128i *)&x)->m128i_u64[0];
#else
	u64 u;
	memcpy(&u, &v, sizeof(u));
	return u;
#endif
}

template<>
inline f64 from_bits<f64, u64>(u64 u) {
	// I tried a bunch of things here, but I can't get VC++ to generate code
	// that doesn't spill the integer to memory. Sigh.
	f64 f;
	memcpy(&f, &u, sizeof(f));
	return f;
}

}

#endif

#endif // UUID_644FCF12367F4D475780169F9A4EAE7C
