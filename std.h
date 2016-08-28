#ifndef TL_STD_H
#define TL_STD_H

#include "config.h"
#include "platform.h"

#include "cstdint.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h> // We include assert.h in order to override it

#if TL_CPP
extern "C" {
#endif

#define memalloc malloc
TL_INLINE void* memrealloc(void* p, usize news, usize olds) {
	TL_UNUSED(olds);
	return realloc(p, news);
}
#define memfree free

//TL_API void* memalloc(size_t s);
//TL_API void* memrealloc(void* p, size_t news, size_t olds);
//TL_API void memfree(void* p);

TL_API void panic();
TL_API u64 tl_get_ticks();
TL_API void tl_crypto_rand(void* data, u32 amount);

#define TL_TIME(name, c) u64 name = tl_get_ticks(); c; name = tl_get_ticks() - name;

// It's not certain that the assert is declared in a compatible way in assert.h
#ifdef assert
#  undef assert
#endif

#ifdef NDEBUG
#define assert(e) ((void)0)
#else

void assert_fail(char const* expr, char const* file, int line);
#define assert(e) if (!(e)) assert_fail(#e, __FILE__, __LINE__); else (void)0

#endif

#if TL_CPP
}

#define TL_DEFAULT_CTORS(name) \
	name() = default; \
	name(name&&) = default; \
	name(name const&) = delete; \
	name& operator=(name const&) = delete; \
	name& operator=(name&&) = default;

namespace tl {

template<typename T>
T max(T const& a, T const& b) {
	return a < b ? b : a;
}

template<typename T>
T min(T const& a, T const& b) {
	return a < b ? a : b;
}

struct non_null {};

template<typename F>
inline u64 timer(F f) {
	u64 start = tl_get_ticks();
	f();
	return tl_get_ticks() - start;
}

}

inline void* operator new(size_t, void* p, tl::non_null) {
	TL_ASSUME_NOT_NULL(p);
	return p;
}

#endif

#endif // TL_STD_H
