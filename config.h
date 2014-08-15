#ifndef UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3
#define UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3

#include "platform.h"

#if TL_WINDOWS
#	ifdef TL_EXPORTS
#		define TL_API __declspec(dllexport)
#	else
#		define TL_API __declspec(dllimport)
#	endif
#else
#	define TL_API
#endif

#ifndef TL_INF_API
#define TL_INF_API
#endif

#ifndef TL_DEF_API
#define TL_DEF_API
#endif

#ifndef TL_PNG_API
#define TL_PNG_API
#endif

#ifndef TL_RECT_API
#define TL_RECT_API
#endif

#ifndef TL_SOCK_API
#define TL_SOCK_API
#endif

#ifndef TL_IMAGE_API
#define TL_IMAGE_API
#endif

#ifndef TL_TT_API
#define TL_TT_API
#endif

#ifndef TL_STREAM_API
#define TL_STREAM_API
#endif

#ifndef FDLIBM_API
#define FDLIBM_API
#endif

/* "Rarer" functions */
#ifndef FDLIBM2_API
#define FDLIBM2_API
#endif

/* Hyperbolic functions */
#ifndef FDLIBMH_API
#define FDLIBMH_API
#endif

#ifndef FDLIBM_INTERNAL
#define FDLIBM_INTERNAL
#endif

#ifndef TL_IEEE754_API
#define TL_IEEE754_API
#endif

#ifndef TL_AM_API
#define TL_AM_API
#endif

#ifndef TL_VORBIS_API
#define TL_VORBIS_API
#endif

#ifndef TL_RIFF_API
#define TL_RIFF_API
#endif

#ifndef TL_CODEC_API
#define TL_CODEC_API
#endif

#define STB_VORBIS_NO_STDIO

#endif // UUID_A338EA1A4A094F2C0DF52FB9D4A2FDA3
