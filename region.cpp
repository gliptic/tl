#include "region.hpp"

#define TL_REGION_BLOCK_OVERHEAD TL_ALIGN_SIZE(sizeof(RegionBlock*), TL_REGION_MAX_ALIGN)

namespace tl {

void* Region::alloc_fallback(usize rounded_size) {
	usize block_size = TL_ALIGN_SIZE(rounded_size + TL_REGION_BLOCK_OVERHEAD, PageSize);
	RegionBlock* bl = (RegionBlock *)malloc(block_size); // TODO: The following assumes that the alignment of bl is at least TL_REGION_MAX_ALIGN

	bl->prev = this->last_block;
	this->last_block = bl;

	// TODO: We can avoid wasting space in the current block by not updating
	// these if there's a significant amount left.
	this->cur = bl->mem;
	this->end = (u8 *)bl + block_size;

	assert(TL_IS_ALIGNED(this->cur, TL_REGION_MAX_ALIGN));

	u8* ret = this->cur;
	this->cur += rounded_size;
	return ret;
}

Region::~Region() {
	RegionBlock* bl = this->last_block;

	while (bl) {
		RegionBlock* prev = bl->prev;
		free(bl);
		bl = prev;
	}
}

}
