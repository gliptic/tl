#ifndef TL_STRING_SET_HPP
#define TL_STRING_SET_HPP

#include "vec.hpp"
#include "std.h"

#include <stdio.h>

namespace tl {

struct StringSet {

	struct Slot {
		u32 h, i;
	};

	u32 hshift, keycount;
	Slot* tab;
	tl::VecSlice<u8 const> source;

	StringSet();
	~StringSet();
	bool get(u32 hash, u8 const* str, u32 prel_offset, u32 len, u32* ret);
	u32 max_dtb();
	u32 count();
};

}

#endif // STRING_SET_HPP

