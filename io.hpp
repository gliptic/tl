#ifndef TL_IO_H
#define TL_IO_H

#include "config.h"
#include "platform.h"
#include "cstdint.h"
#include "string.hpp"

namespace tl {

TL_API void sprint(tl::StringSlice str);

TL_BEGIN_C

TL_API void sprint(char const* s);
TL_API void uprint(uint32_t v);
TL_API void iprint(int32_t v);
TL_API void fprint(double v);

TL_END_C

#if TL_WINDOWS
typedef usize Win_HANDLE;

struct Handle {
	Win_HANDLE h;

	Handle() {}

	Handle(Win_HANDLE h) : h(h) {
	}
};
#else
struct Handle {
	int h;

	Handle() {}

	Handle(int h) : h(h) {
	}
};
#endif

}

#endif
