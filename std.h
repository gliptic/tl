#ifndef TL_STD_H
#define TL_STD_H

#include "config.h"

#include <stdint.h>

TL_API void* memalloc(size_t s);
TL_API void* memrealloc(void* p, size_t news, size_t olds);
TL_API void memfree(void* p);
TL_API void panic();

TL_API void mcpy(void* dst, void const* src, size_t len);
TL_API void mset(void* dst, uint8_t v, size_t len);
TL_API size_t slen(char const* s);

#ifdef NDEBUG
#define assert(e) ((void)0)
#else

void assert_fail(char const* expr, char const* file, int line);
#define assert(e) (void)(!!(e) || assert_fail(#e, __FILE__, __LINE__))

#endif

#endif // TL_STD_H

