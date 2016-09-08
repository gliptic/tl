#ifndef TL_RUNTIME_H
#define TL_RUNTIME_H 1

#include "platform.h"

#if TL_MSVCPP
# include "windows/runtime_vc.c"
# if TL_CPP
#  include "windows/runtime_vcpp.cpp"
# endif
#else
# error "Unsupported platform"
#endif

#endif // TL_RUNTIME_H
