#ifndef UUID_888421EF4BF64B115A93C69A521234CB
#define UUID_888421EF4BF64B115A93C69A521234CB

#include "config.h"
#include "io/stream.h"

#define RIFF_SIGN(a, b, c, d) ((a) + ((b)<<8) + ((c)<<16) + ((d)<<24))

TL_RIFF_API u64 riff_push_hdr(tl_byte_sink_pushable* self, u32 sign, u32 len);
TL_RIFF_API u64 riff_push_riff_hdr(tl_byte_sink_pushable* self, u32 sign, u32 len);
TL_RIFF_API u64 riff_push_list_hdr(tl_byte_sink_pushable* self, u32 sign, u32 len);

TL_RIFF_API void riff_patch_hdr_len(tl_byte_sink_pushable* self, u64 org);

#endif // UUID_888421EF4BF64B115A93C69A521234CB
