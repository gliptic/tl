#ifndef TL_REGION_HPP
#define TL_REGION_HPP 1

#include "platform.h"
#include "vec.hpp"

namespace tl {

#ifndef TL_REGION_MAX_ALIGN
#  define TL_REGION_MAX_ALIGN (8)
#endif

// TODO: Change these to functions
#define TL_ALIGN_SIZE(size, alignment) (((size) + ((alignment)-1)) & ~((usize)(alignment) - 1))

#define TL_ALIGN_PTR(p, alignment) ((u8 *)TL_ALIGN_SIZE((usize)(p), alignment))
#define TL_IS_ALIGNED(p, alignment) (((usize)(p) & ((usize)(alignment) - 1)) == 0)

struct RegionBlock {
	RegionBlock* prev;
	u8 mem[];
};

struct Allocator {
	u8 *cur, *end;

	void* alloc(usize size) {
		usize rounded_size = TL_ALIGN_SIZE(size, TL_REGION_MAX_ALIGN);

		if (usize(this->end - this->cur) >= rounded_size) {
			void* ret = this->cur;
			this->cur += rounded_size;
			return ret;
		}

		return this->alloc_fallback(rounded_size);
	}

	virtual void* alloc_fallback(usize rounded_size) = 0;
};

/*
template<usize Size>
struct FixedSizeAllocator {
	u8 *cur, *end;

	static usize const RoundedSize = TL_ALIGN_SIZE(Size, TL_REGION_MAX_ALIGN);

	void* alloc() {
		if (usize(this->end - this->cur) >= RoundedSize) {
			void* ret = this->cur;
			this->cur += RoundedSize;
			return ret;
		}

		return this->alloc_fallback(rounded_size);
	}

	virtual void* alloc_fallback() = 0;
};
*/

struct Region : Allocator {
	static usize const PageSize = 4096;

	Region()
		: last_block(0) {

		this->cur = TL_ALIGN_PTR(this->mem, TL_REGION_MAX_ALIGN);
		this->end = this->mem + PageSize;
	}

	void* alloc_fallback(usize rounded_size);
	~Region();

	RegionBlock* last_block;
	u8 mem[PageSize];
};

struct BasicFreelist {
	BasicFreelist()
		: top(NULL) {
	}

	void* top;
};

template<usize Size>
struct Freelist : BasicFreelist {
	static_assert(Size >= sizeof(void*), "Freelist size must be at least as large as a pointer");
	static_assert(TL_IS_ALIGNED(Size, TL_REGION_MAX_ALIGN), "Freelist size must be aligned");

	Freelist() {
	}

	void* try_alloc() {
		void* p = this->top;
		if (p) {
			this->top = *(void **)p;
		}

		return p;
	}

	void free(void* p) {
		assert(TL_IS_ALIGNED(p, TL_REGION_MAX_ALIGN));

		*(void **)p = this->top;
		this->top = p;
	}

	
};

struct FreelistDelete {
	
	FreelistDelete()
		: ptrToTop(0) {
	}

	FreelistDelete(BasicFreelist& freelist)
		: ptrToTop(&freelist.top) {
	}

	void operator()(void* p) {
		*(void **)p = *ptrToTop;
		*ptrToTop = p;
	}

	void** ptrToTop;
};

template<typename T>
struct PooledRegion : Region {
	static usize const RoundedSize = TL_ALIGN_SIZE(sizeof(T), TL_REGION_MAX_ALIGN);
	Freelist<RoundedSize> freelist;

	void* alloc() {
		return this->alloc(RoundedSize);
	}

	T* alloc_from_pool() {
		void* p = this->freelist.try_alloc();
		if (p) return (T *)p;
		return this->alloc();
	}

	void free_to_pool(T* p) {
		this->freelist.free(p);
	}
};

}

#endif // TL_REGION_HPP
