#ifndef TL_RAND_H
#define TL_RAND_H

#include "platform.h"
#include <stdint.h>

typedef struct tl_xorshift {
	uint32_t v;
} tl_xorshift;

TL_INLINE uint32_t tl_xs_gen(tl_xorshift* self) {
	uint32_t v = self->v;
	v ^= v << 2;
	v ^= v >> 9;
	v ^= v >> 15;
	self->v = v;
	return v;
}

#endif
