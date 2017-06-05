//#include "miniwindows.h"

TL_BEGIN_C

__declspec(dllexport) u8 TL_STDCALL SystemFunction036(void* in, u32 len);

TL_INLINE void tl_crypto_rand(void* in, u32 len) {
	SystemFunction036(in, len);
}

TL_END_C
