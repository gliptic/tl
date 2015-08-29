#include "region.h"
#include "std.h"

#define TL_REGION_BLOCK_OVERHEAD TL_ALIGN_SIZE(sizeof(tl_region_block*), TL_REGION_MAX_ALIGN)

// TODO: Use page allocators

void* tl_region_alloc_newblock(tl_allocator* alloc, size_t rounded_size) {
	tl_region* r = (tl_region*)alloc;
	size_t block_size = TL_ALIGN_SIZE(rounded_size + TL_REGION_BLOCK_OVERHEAD, TL_REGION_PAGE_SIZE); 
	tl_region_block* bl = memalloc(block_size); // TODO: The following assumes that the alignment of bl is at least TL_REGION_MAX_ALIGN
	uint8_t* ret;

	bl->prev = r->last_block;
	r->last_block = bl;

	// TODO: We can avoid wasting space in the current block by not updating
	// these if there's a significant amount left.
	r->alloc.cur = (uint8_t*)bl + TL_REGION_BLOCK_OVERHEAD;
	r->alloc.end = (uint8_t*)bl + block_size;

	assert(TL_IS_ALIGNED(r->cur, TL_REGION_MAX_ALIGN));
	
	ret = r->alloc.cur;
	r->alloc.cur += rounded_size;
	return ret;
}

void tl_region_free(tl_region* r) {
	tl_region_block* bl = r->last_block;

	while(bl) {
		tl_region_block* prev = bl->prev;
		memfree(bl);
		bl = prev;
	}
}

#undef ALIGN_PTR

