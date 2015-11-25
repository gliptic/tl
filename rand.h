#ifndef TL_RAND_H
#define TL_RAND_H

#include "platform.h"
#include "cstdint.h"

typedef struct tl_xorshift {
	u32 v;
} tl_xorshift;

TL_INLINE uint32_t tl_xs_gen(tl_xorshift* self) {
	u32 v = self->v;
	v ^= v << 13;
	v ^= v >> 17;
	v ^= v >> 5;
	self->v = v;
	return v;
}

typedef struct tl_xorshift2 {
	u32 x, y;
} tl_xorshift2;

//static uint32_t x = 123456789, y = 362436069;

TL_INLINE u32 tl_xs2_init(tl_xorshift2* self) {
	self->x = 123456789;
	self->y = 362436069;
}

TL_INLINE u32 tl_xs2_gen(tl_xorshift2* self) {
	u32 x = self->x;
	u32 y = self->y;
	self->x = y;
	u32 t = x ^ (x << 10);
	y = (y ^ (y >> 10)) ^ (t ^ (t >> 13));
	self->y = y;
	return y + x;
}

#endif
