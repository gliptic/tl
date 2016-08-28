#ifndef TL_REGION_HPP
#define TL_REGION_HPP 1

#include "platform.h"
#include "vec.hpp"

namespace tl {

struct Allocator {

	VecSlice<u8> mem;
};

struct Region : Allocator {

};

}

#endif // TL_REGION_HPP
