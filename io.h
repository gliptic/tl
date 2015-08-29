#ifndef TL_IO_H
#define TL_IO_H

#include "config.h"
#include <stdint.h>

TL_API void sprint(char const* s);
TL_API void uprint(uint32_t v);
TL_API void iprint(int32_t v);
TL_API void fprint(double v);

#endif
