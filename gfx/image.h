#ifndef UUID_917ECCD8563741D5491DF687EA1F876D
#define UUID_917ECCD8563741D5491DF687EA1F876D

#include "../config.h"
#include "../cstdint.h"

typedef struct tl_image {
	u8* pixels;
	u32 w, h;
	u32 pitch;
	u32 bpp;
} tl_image;

TL_IMAGE_API void tl_blit_unsafe(tl_image* to, tl_image* from, int x, int y);
TL_IMAGE_API int  tl_image_convert(tl_image* to, tl_image* from);
TL_IMAGE_API int  tl_image_pad(tl_image* to, tl_image* from);
TL_IMAGE_API void tl_image_init(tl_image* self, u32 w, u32 h, int bpp);

#endif // UUID_917ECCD8563741D5491DF687EA1F876D
