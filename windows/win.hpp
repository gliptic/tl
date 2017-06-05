#ifndef WIN_HPP
#define WIN_HPP 1

#undef  NOGDI
#define NOGDI
#undef  NOMINMAX
#define NOMINMAX
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#undef  NONAMELESSUNION
#define NONAMELESSUNION
#undef  NOKERNEL
#define NOKERNEL
#undef  NONLS
#define NONLS
#ifndef POINTER_64
#define POINTER_64 // Needed for bugged headers
#endif

#undef WINVER
#undef _WIN32_WINNT
#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

#include <windows.h>

#undef  NOGDI
#undef  NOMINMAX
#undef  WIN32_LEAN_AND_MEAN
#undef  NONAMELESSUNION
#undef  NOKERNEL
#undef  NONLS

#endif // WIN_HPP
