#include "wav.hpp"

TL_PACKED_STRUCT(WavHeader)
	u32 riff_hdr;
	u32 rounded_size; // this is - 8
	u32 wave_hdr;
	u32 fmt_hdr;
	u32 dummy0;

	u16 dummy1;
	u16 dummy2;
	u32 sample_rate;
	u32 bytes_per_second;
	u16 bytes_per_frame;
	u16 sample_bits;
	u32 data_hdr;
	u32 data_size;
TL_PACKED_STRUCT_END();

#define CHECK(c) if(!(c)) goto fail

using tl::read_le;

inline u32 quad(char a, char b, char c, char d) {
	return (u32)a + ((u32)b << 8) + ((u32)c << 16) + ((u32)d << 24);
}

inline i16 sample8_to_sample16(u8 s) {
	return (i16(s) - 128) * 30;
}

namespace tl {

int write_wav(Sink& dest, VecSlice<u8 const>& sound) {
	WavHeader header;

	header.riff_hdr = quad('R', 'I', 'F', 'F');
	header.wave_hdr = quad('W', 'A', 'V', 'E');
	header.fmt_hdr = quad('f', 'm', 't', ' ');
	header.dummy0 = 16;
	header.dummy1 = 1;
	header.dummy2 = 1;
	header.sample_rate = 22050;
	header.bytes_per_second = 22050 * 1 * 1;
	header.bytes_per_frame = 1 * 1;
	header.sample_bits = 8;
	header.data_hdr = quad('d', 'a', 't', 'a');
	header.data_size = sound.size() * 1 * 1;

	auto rounded_size = (sound.size() + 1) & ~1;

	CHECK(!dest.put_raw<WavHeader>(header));

	{
		auto w = dest.window(rounded_size);
		CHECK(!w.empty());

		u8* destp = w.begin();

		for (usize i = 0; i < sound.size(); ++i) {
			*destp++ = u8(sound[i] + 128);
		}

		if (rounded_size != sound.size()) {
			*destp++ = 0;
		}
	}

	return 0;

fail:
	return 1;
}

int read_wav(
	tl::Source src,
	tl::Vec<i16>& sound) {

	auto hdr = src.window(sizeof(WavHeader));

	CHECK(!hdr.empty());

	WavHeader const* header = (WavHeader const*)hdr.begin();

	CHECK(read_le(header->riff_hdr) == quad('R', 'I', 'F', 'F'));
	CHECK(read_le(header->wave_hdr) == quad('W', 'A', 'V', 'E'));

	CHECK(read_le(header->dummy0) == 16);
	CHECK(read_le(header->dummy1) == 1);
	CHECK(read_le(header->dummy2) == 1);

	CHECK(read_le(header->sample_rate) == 22050);
	CHECK(read_le(header->bytes_per_second) == 22050 * 1 * 1);
	CHECK(read_le(header->bytes_per_frame) == 1 * 1);
	CHECK(read_le(header->sample_bits) == 8);
	CHECK(read_le(header->data_hdr) == quad('d', 'a', 't', 'a'));

	u32 data_size = header->data_size;
	
	{
		auto data = src.window(data_size);
		CHECK(!data.empty());

		sound.clear();
		sound.reserve(data_size * 2);
	
		i16 prev = sample8_to_sample16(data[0]);
		i16* dest = sound.begin();
		*dest++ = prev;

		for (u32 i = 1; i < data_size; ++i) {
			i16 cur = sample8_to_sample16(data[i]);
			*dest++ = (prev + cur) / 2;
			*dest++ = cur;
			prev = cur;
		}

		sound.unsafe_set_size(data_size * 2);

		*dest++ = prev;
	}

	return 0;

fail:
	return 1;
}

}
