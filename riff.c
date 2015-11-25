#include "riff.h"

u64 riff_push_hdr(tl_byte_sink_pushable* self, u32 sign, u32 len)
{
	tl_bs_push32_le(self, sign);
	tl_bs_push32_le(self, len);
	return tl_bs_tell_sink(self);
}

u64 riff_push_riff_hdr(tl_byte_sink_pushable* self, u32 sign, u32 len)
{
	u64 p = riff_push_hdr(self, RIFF_SIGN('R','I','F','F'), len + 4);
	tl_bs_push32_le(self, sign);
	return p;
}

u64 riff_push_list_hdr(tl_byte_sink_pushable* self, u32 sign, u32 len)
{
	u64 p = riff_push_hdr(self, RIFF_SIGN('L','I','S','T'), len + 4);
	tl_bs_push32_le(self, sign);
	return p;
}

void riff_patch_hdr_len(tl_byte_sink_pushable* self, u64 org)
{
	u64 cur = tl_bs_tell_sink(self);
	u64 diff = cur - org;
	tl_bs_seek_sink(self, org - 4);
	tl_bs_push32_le(self, (u32)diff);
	tl_bs_seek_sink(self, cur);
}
