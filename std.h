#ifndef TL_STD_H
#define TL_STD_H

#include "config.h"
#include "platform.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define memalloc malloc
#define memrealloc(p, news, olds) realloc(p, news)
#define memfree free

//TL_API void* memalloc(size_t s);
//TL_API void* memrealloc(void* p, size_t news, size_t olds);
//TL_API void memfree(void* p);

TL_API void panic();

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

#endif // TL_STD_H
