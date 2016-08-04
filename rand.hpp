#ifndef TL_RAND_HPP
#define TL_RAND_HPP 1

#include "std.h"

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
};

}

#endif // TL_RAND_HPP
