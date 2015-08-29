#ifndef UUID_7218AF0FB69C484F46F1C185F0A5EF73
#define UUID_7218AF0FB69C484F46F1C185F0A5EF73

#include "../config.h"

typedef struct am_pair { float first, second; } am_pair;

#define AM_PI			(3.14159265358979323846)
#define AM_PI2          (6.28318530717958647693)
#define AM_PI_O_2       (1.57079632679489661923)
#define AM_PI_O_4       (0.785398163397448309616)

TL_AM_API am_pair am_sincosf(float x);
TL_AM_API float   am_sinf(float x);
TL_AM_API float   am_cosf(float x);
/*TL_AM_API*/ float   am_asinf(float x);
/*TL_AM_API*/ float   am_acosf(float x);

/*TL_AM_API*/ float am_sinf_2(float x);

TL_AM_API float am_tanf(float x);
/*TL_AM_API*/ float am_tanf_2(float x);

TL_AM_API float am_expf(float x);
TL_AM_API float am_exp2f(float x);
/*TL_AM_API*/ float am_expf_2(float x);
/*TL_AM_API*/ float cephes_expf(float x);

TL_AM_API float am_powf(float x, float y);

float am_sinf_inline(float x);
float am_sinf_intrin(float x);

#endif // UUID_7218AF0FB69C484F46F1C185F0A5EF73
