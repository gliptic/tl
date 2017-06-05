#include "vec.hpp"

namespace tl {

AlwaysTrue VecAbstract::reserve_bytes(usize new_cap) {
	
	u8* new_b = this->begin_bytes();
	if (new_cap <= usize(c - new_b))
		return AlwaysTrue();

	usize size = this->size_bytes();

	if ((new_b = (u8 *)realloc(new_b, new_cap)) == 0) {
		free(new_b);
		new_b = 0;
	}

	VecSliceAbstract& base = *this;
	base = VecSliceAbstract(new_b, new_b + size);
	c = new_b + new_cap;
	return AlwaysTrue();
}

AlwaysTrue VecAbstract::enlarge(usize extra) {
	usize size = this->size_bytes();
	usize new_cap = 2 * size + extra;

	// TODO: Overflow check new_cap
	
	this->reserve_bytes(new_cap);
	return AlwaysTrue();
}

}
