#include "tga.hpp"
#include "../memory.h"

namespace tl {

TL_PACKED_STRUCT(TgaHeader)
	u8 id_len;
	u8 dummy0;
	u8 dummy1;
	u16 pal_beg;
	u16 pal_end;
	u8 pal_bits;

	u16 dummy2;
	u16 dummy3;
	u16 width;
	u16 height;
	u8 dummy4;
	u8 dummy5;
TL_PACKED_STRUCT_END();

TL_STATIC_ASSERT(sizeof(TgaHeader) == 3 + 5 + 4 + 4 + 2);

#define CHECK(c) if(!(c)) goto fail

int write_tga(Sink& sink, ImageSlice img, Palette const& pal) {
	TgaHeader header;
	header.id_len = 0;
	header.dummy0 = 1;
	header.dummy1 = 1;
	header.pal_beg = 0;
	header.pal_end = 256;
	header.pal_bits = 24;
	header.dummy2 = 0;
	header.dummy3 = 0;
	header.width = narrow<u16>(img.width());
	header.height = narrow<u16>(img.height());
	header.dummy4 = 8;
	header.dummy5 = 0;

	sink.put<TgaHeader>(header);

	u32 image_width = header.width;
	u32 image_height = header.height;
	u32 row_size = image_width * 1;

	auto img_data = sink.window(0 + 256 * 3 + row_size * image_height);
	CHECK(!img_data.empty());

	u8* p = img_data.begin();
	img_data.unsafe_cut_front(256 * 3);

	for (auto& entry : pal.entries) {
		*p++ = entry.b();
		*p++ = entry.g();
		*p++ = entry.r();
	}

	// TODO: This is only for 8-bit
	u8 const* pixels = img.ptr(0, image_height - 1, 1);
	for (u32 y = image_height; y-- > 0; ) {

		memcpy(img_data.begin(), pixels, row_size);

		pixels -= img.pitch;
		img_data.unsafe_cut_front(row_size);
	}

	return 0;

fail:
	return 1;
}

int read_tga(
	Source& src,
	Image& img,
	Palette& pal) {

	auto hdr = src.window(sizeof(TgaHeader));

	CHECK(!hdr.empty());

	TgaHeader const* header = (TgaHeader const*)hdr.begin();
	
	u8 id_len = read_le(header->id_len);
	CHECK(read_le(header->dummy0) == 1);
	CHECK(read_le(header->dummy1) == 1);

	CHECK(read_le(header->pal_beg) == 0);
	CHECK(read_le(header->pal_end) == 256);
	CHECK(read_le(header->pal_bits) == 24);

	CHECK(read_le(header->dummy2) == 0);
	CHECK(read_le(header->dummy3) == 0);

	u16 image_width = read_le(header->width),
		image_height = read_le(header->height);

	CHECK(read_le(header->dummy4) == 8);
	CHECK(read_le(header->dummy5) == 0);
	
	{
		u32 row_size = image_width * 1;
		auto img_data = src.window(id_len + 256 * 3 + row_size * image_height);
		CHECK(!img_data.empty());

		img_data.unsafe_cut_front(id_len);

		u8 const* p = img_data.begin();

		for (auto& entry : pal.entries) {
			u8 b = *p++;
			u8 g = *p++;
			u8 r = *p++;
			entry = tl::Color(r, g, b);
		}

		img_data.unsafe_cut_front(256 * 3);

		img.alloc_uninit(image_width, image_height, 1);

		// TODO: This is only for 8-bit
		u8* pixels = img.data();
		for (u32 y = image_height; y-- > 0; ) {
		
			u8* dest = &pixels[y * row_size];
			memcpy(dest, img_data.begin(), row_size);

			img_data.unsafe_cut_front(row_size);
		}
	}

	return 0;

fail:
	return 1;
}

}
