#ifndef TL_REGION_H
#define TL_REGION_H

#include <stddef.h>
#include <stdint.h>

#include "platform.h"
#include "config.h"

#ifndef TL_REGION_MAX_ALIGN
#  define TL_REGION_MAX_ALIGN (8)
#endif

#ifndef TL_REGION_PAGE_SIZE
#  define TL_REGION_PAGE_SIZE (4096) // Must be a power of two
#endif

typedef struct tl_region_block {
	struct tl_region_block* prev;
	uint8_t mem[0];
} tl_region_block;

typedef struct tl_allocator {
	uint8_t* cur;
	uint8_t* end;
	void* (*alloc)(struct tl_allocator* self, size_t s);
} tl_allocator;

typedef struct tl_region {
	tl_allocator alloc;
	tl_region_block* last_block;
	uint8_t  mem[TL_REGION_PAGE_SIZE];
} tl_region;

TL_STATIC_ASSERT((TL_REGION_PAGE_SIZE & (TL_REGION_PAGE_SIZE - 1)) == 0);

TL_REGION_API void* tl_region_alloc_newblock(tl_allocator* r, size_t rounded_size);
TL_REGION_API void tl_region_free(tl_region* r);

#define TL_ALIGN_SIZE(size, alignment) (((size) + ((alignment)-1)) & ~((size_t)(alignment) - 1))
//#define TL_ALIGN_SIZE(size, alignment) ((((size) - 1) | ((size_t)(alignment) - 1)) + 1)

#define TL_ALIGN_PTR(p, alignment) ((uint8_t*)TL_ALIGN_SIZE((size_t)(p), alignment))
#define TL_IS_ALIGNED(p, alignment) (((size_t)(p) & ((size_t)(alignment) - 1)) == 0)

TL_INLINE void* tl_alloc(tl_allocator* r, size_t size) {
	size_t rounded_size = TL_ALIGN_SIZE(size, TL_REGION_MAX_ALIGN);

	if((size_t)(r->end - r->cur) >= rounded_size) {
		void* ret = r->cur;
		r->cur += rounded_size;
		return ret;
	}

	return r->alloc(r, rounded_size);
}

TL_INLINE void tl_region_init(tl_region* r) {
	// TODO: Init allocator
	r->alloc.alloc = tl_region_alloc_newblock;
	r->alloc.end = TL_ALIGN_PTR(r->mem, TL_REGION_MAX_ALIGN);
	r->alloc.cur = r->mem + TL_REGION_PAGE_SIZE;
	r->last_block = NULL;
}

#endif // TL_REGION_H
