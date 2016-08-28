#ifndef TL_RAND_HPP
#define TL_RAND_HPP 1

#include "std.h"
#include "vector.hpp"

namespace tl {

struct XorShift {
	u32 v;

	XorShift(u32 v = 1) : v(v) {
	}

	u32 next() {
		u32 lv = this->v;
		lv ^= lv << 13;
		lv ^= lv >> 17;
		lv ^= lv >> 5;
		return (this->v = lv);
	}

	f64 get_u01() {
		return this->next() * 0.00000000023283064365386962890625;
	}

	u32 get_unsigned(u32 max) {
		return u32((u64(next()) * max) >> 32);
	}

	i32 get_i32(i32 max) {
		double r = this->next() * (1.0 / (1ull << 32));
		double r2 = r * max;
		return i32(r2);
	}

	VectorI2 get_vectori2(i32 max) {
		i32 x = get_i32(max);
		i32 y = get_i32(max);
		return VectorI2(x, y);
	}
};

struct LcgPair {
	u32 s[2];

	LcgPair() {
		tl_crypto_rand(&s, sizeof(s));
	}

	LcgPair(u32 a_init, u32 b_init) {
		s[0] = a_init;
		s[1] = b_init;
	}

	u32 next() {
		u32 x = s[0];
		s[0] = s[0] * 29943829 + 0xffff;
		return x;
	}

	static i32 lim(u32 v, i32 max) {
		double r = v * (1.0 / (1ull << 32));
		double r2 = r * max;
		return i32(r2);
	}

	static i32 lim_f64(u32 v, f64 scaled_max) {
		double r = (i32)v * scaled_max;
		return i32(r);
	}

	i32 get_i32(i32 max) {
		return lim(this->next(), max);
	}

	i32 get_i32(f64 scaled_max) {
		return lim_f64(this->next(), scaled_max);
	}

	VectorI2 get_vectori2_f64(f64 scaled_max) {
		u32 x = s[0], y = s[1];
		s[0] = s[0] * 29943829 + 0xffff;
		s[1] = s[1] * 29943829 + 0xffff;

		return VectorI2(lim_f64(x, scaled_max), lim_f64(y, scaled_max));
	}

	VectorI2 get_vectori2_f64(VectorD2 scaled_max) {
		u32 x = s[0], y = s[1];
		s[0] = s[0] * 29943829 + 0xffff;
		s[1] = s[1] * 29943829 + 0xffff;

		return VectorI2(lim_f64(x, scaled_max.x), lim_f64(y, scaled_max.y));
	}
};

}

#endif // TL_RAND_HPP
