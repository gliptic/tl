#ifndef UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3
#define UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3

#include "platform.h"

#define TL_STATIC_API

#if TL_WINDOWS
#	ifdef TL_EXPORTS
#		define TL_API __declspec(dllexport)
#	else
#		define TL_API __declspec(dllimport)
#	endif
#else
#	define TL_API
#endif

#ifndef TL_TREAP_API
#define TL_TREAP_API TL_API
#endif

#endif // UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3