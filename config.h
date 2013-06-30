#ifndef UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3
#define UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3

#include "platform.h"

#define TL_STATIC_API

#ifdef TL_DEFAULT_API
#	define TL_API TL_DEFAULT_API
#elif TL_WINDOWS
#	if TL_EXPORTS
#		define TL_API __declspec(dllexport)
#   elif TL_STATIC
#		define TL_API extern
#	else
#		define TL_API __declspec(dllimport)
#	endif
#else
#	define TL_API TL_NOAPI
#endif

#ifndef TL_NOAPI
#define TL_NOAPI extern
#endif

#ifndef TL_TREAP_API
#define TL_TREAP_API
#endif

#ifndef TL_IEEE754_API
#define TL_IEEE754_API
#endif

#ifndef TL_REGION_API
#define TL_REGION_API
#endif

#ifndef TL_STREAM_API
#define TL_STREAM_API
#endif

#ifndef TL_CODEC_API
#define TL_CODEC_API
#endif

#endif // UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3
