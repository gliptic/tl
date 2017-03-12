#ifndef UUID_917ECCD8563741D5491DF687EA1F876D
#define UUID_917ECCD8563741D5491DF687EA1F876D

#include "../config.h"
#include "../cstdint.h"
#include "../std.h"
#include "../rect.hpp"
#include "../vec.hpp"

namespace tl {

struct Color {
	u32 v;

	Color() {}
	
	explicit Color(u32 v) : v(v) {}

	Color(u8 r, u8 g, u8 b, u8 a = 255) {
		// Little endian. TODO: Support big endian
		v = r | (g << 8) | (b << 16) | (a << 24);
	}

	u8 r() const { return v & 0xff; }
	u8 g() const { return (v >> 8) & 0xff; }
	u8 b() const { return (v >> 16) & 0xff; }
	u8 a() const { return (v >> 24) & 0xff; }

	Color blend_half(Color other) {
		return Color(
			((this->v & 0xfefefe) >> 1)
		   + ((other.v & 0xfefefe) >> 1)
		   + (this->v & other.v & 0x010101));
	}

	static Color read(u8 const* src) {
		Color r;
		memcpy(&r.v, src, 4);
		return r;
	}

	static void write(u8* dest, Color src) {
		memcpy(dest, &src.v, 4);
	}

	Color with_a(u8 a_new) const {
		return Color((this->v & 0xffffff) | (a_new << 24));
	}
};

struct Palette {
	Color entries[256];
};

struct Image;

struct ImageSlice {
	u8* pixels;
	VectorU2 dim;
	u32 pitch, bpp;

	enum {
		BlitTransparent = 1 << 0,
		Flip = 1 << 1,
		AsGrayscale = 1 << 2
	};

	template<typename PixelT>
	struct Range {
		u8* cur;
		u8* end;
		u32 line_size;
		u32 pitch_minus_line_size;
		u8* end_rows;

		u8* next() {
			if (cur == end) {
				cur += pitch_minus_line_size;
				if (cur == end_rows) {
					return 0;
				}

				end = cur + line_size;
			}

			u8* r = cur;
			cur += sizeof(PixelT);
			return r;
		}
	};

	template<typename PixelT>
	struct PixelsRange {
		u8* cur;
		u8* end;

		u32 size() const {
			return u32((end - cur) / sizeof(PixelT));
		}

		bool next(u8*& p) {
			if (this->cur == this->end) {
				return false;
			}

			p = this->cur;
			this->cur += sizeof(PixelT);
			return true;
		}
	};

	template<typename PixelT>
	struct LinesRange {
		u8* cur;
		u32 line_size;
		u32 pitch;
		u8* end_lines;

		bool next(PixelsRange<PixelT>& pr) {
			if (this->cur == this->end_lines) {
				return false;
			}

			pr.cur = this->cur;
			pr.end = this->cur + this->line_size;

			this->cur += this->pitch;
			return true;
		}
	};

	LinesRange<u8> lines_range() {
		LinesRange<u8> r;
		r.cur = this->pixels;
		r.line_size = this->dim.x * this->bpp;
		r.pitch = this->pitch;
		r.end_lines = r.cur + this->dim.y * this->pitch;
		return r;
	}

	Range<u8> range() {
		Range<u8> r;
		r.cur = this->pixels;
		r.line_size = this->dim.x * this->bpp;
		r.pitch_minus_line_size = this->pitch - r.line_size;
		r.end = r.cur + r.line_size;
		r.end_rows = r.cur + this->dim.y * this->pitch;

		return r;
	}

	ImageSlice()
		: pixels(0)
		, pitch(0)
		, bpp(0) {
	}

	ImageSlice(ImageSlice const& other)
		: pixels(other.pixels)
		, dim(other.dim)
		, pitch(other.pitch)
		, bpp(other.bpp) {
	}

	ImageSlice(u8* pixels, u32 w, u32 h, u32 pitch, u32 bpp)
		: pixels(pixels)
		, dim(w, h)
		, pitch(pitch)
		, bpp(bpp) {
	}

	void free();

	ImageSlice& operator=(ImageSlice const& other) = default;

	ImageSlice crop(RectU rect) const {
		assert(rect.valid());
		assert(rect.x2 <= this->dim.x && rect.y2 <= this->dim.y);

		u32 lpitch = this->pitch;
		u32 lbpp = this->bpp;

		return ImageSlice(
			this->pixels + rect.x1*lbpp + rect.y1*lpitch,
			rect.width(),
			rect.height(),
			lpitch,
			lbpp);
	}

	ImageSlice crop_bottom(u32 amount) const {
		assert(this->dim.y >= amount);

		return ImageSlice(
			this->pixels,
			this->dim.x,
			this->dim.y - amount,
			this->pitch,
			this->bpp);
	}

	ImageSlice crop_square_sprite_v(u32 index) const {
		assert(index < this->dim.y / this->dim.x);

		u32 y = index * this->dim.x;
		return this->crop(RectU(0, y, this->dim.x, y + this->dim.x));
	}

	Image convert(u32 bpp, tl::Palette* pal = 0);

	inline int blit(ImageSlice const& from, Palette* pal = 0, i32 x = 0, i32 y = 0, u32 flags = 0);

	u8& unsafe_pixel8(u32 x, u32 y) {
		assert(x < this->dim.x && y < this->dim.y && this->bpp == 1);

		return *ptr(x, y, 1);
	}

	u8 unsafe_pixel8(u32 x, u32 y) const {
		assert(x < this->dim.x && y < this->dim.y && this->bpp == 1);

		return *ptr(x, y, 1);
	}

	u32& unsafe_pixel32(u32 x, u32 y) {
		assert(x < this->dim.x && y < this->dim.y && this->bpp == 4);

		return *(u32 *)ptr(x, y, 4);
	}

	u32 unsafe_pixel32(u32 x, u32 y) const {
		assert(x < this->dim.x && y < this->dim.y && this->bpp == 4);

		return *(u32 *)ptr(x, y, 4);
	}

	u32 width() const {
		return this->dim.x;
	}

	u32 height() const {
		return this->dim.y;
	}

	VectorU2 dimensions() const {
		return this->dim;
	}

	u32 bytespp() const {
		return this->bpp;
	}

	bool is_inside(tl::VectorI2 pos) const {
		return ((u32)pos.x < this->dim.x) && ((u32)pos.y < this->dim.y);
	}

	bool is_empty() const {
		return this->dim.x == 0;
	}

	u8* data() {
		return this->pixels;
	}

	u8 const* data() const {
		return this->pixels;
	}

	u32 size() const {
		return this->pitch * this->dim.y;
	}

	u32 row_pitch() const {
		return this->pitch;
	}

	u8* ptr(u32 x, u32 y, u32 lbpp) {
		return this->pixels + y*this->pitch + x*lbpp;
	}

	u8 const* ptr(u32 x, u32 y, u32 lbpp) const {
		return this->pixels + y*this->pitch + x*lbpp;
	}

	ImageSlice const& slice() const {
		return *this;
	}
};

struct Image : protected ImageSlice {

	using ImageSlice::width;
	using ImageSlice::height;
	using ImageSlice::dimensions;
	using ImageSlice::data;
	using ImageSlice::size;
	using ImageSlice::blit;
	using ImageSlice::convert;
	using ImageSlice::ptr;
	using ImageSlice::unsafe_pixel8;
	using ImageSlice::unsafe_pixel32;
	using ImageSlice::is_inside;
	using ImageSlice::is_empty;
	using ImageSlice::crop;
	using ImageSlice::crop_square_sprite_v;
	using ImageSlice::bytespp;
	using ImageSlice::lines_range;
	using ImageSlice::row_pitch;
	using ImageSlice::slice;

	Image() {
	}
	
	Image(Image&& other)
		: ImageSlice(other) {
		other.pixels = 0;
	}

	Image& operator=(Image&& other) {
		this->free();
		ImageSlice::operator=(other);
		other.pixels = 0;
		return *this;
	}

	TL_IMAGE_API Image(u32 w, u32 h, u32 bpp);

	~Image() {
		this->free();
	}

	// Warning: this will leak if the slice isn't manually freed
	ImageSlice to_slice() {
		ImageSlice sl(this->slice());
		this->pixels = 0;
		return sl;
	}

	Image(Image const& other) = delete;
	Image& operator=(Image const&) = delete;

	TL_IMAGE_API void alloc_uninit(u32 w, u32 h, u32 bpp);
	TL_IMAGE_API void copy_from(Image const& other);
	TL_IMAGE_API void blit_unsafe(Image const& from, u32 x, u32 y);
	
	TL_IMAGE_API int pad(Image const& from);

};

struct Cursor {
	u32 pitch, bpp;
	u8 *pixels;
};

template<bool KeepOffset>
struct DestOffset {
	void set_offset(tl::VectorU2 /*offset*/) { /* Nothing */ }
};

template<>
struct DestOffset<true> {
	tl::VectorU2 offset;

	void set_offset(tl::VectorU2 offset_new) { this->offset = offset_new; }
};

struct BaseBlitContext {
	VectorU2 dim;

	void clip(i32& x, i32& y, VectorU2& src, VectorU2 todim);
	void init_blit_context(
		tl::Cursor* sources, ImageSlice* to, usize target_count,
		tl::Cursor* targets, ImageSlice* from, usize source_count, i32& x, i32& y);
};

struct UnsafeNoClip {};

template<u32 TargetCount, u32 SourceCount, bool KeepOffset = false>
struct BasicBlitContext : BaseBlitContext, DestOffset<KeepOffset> {
	
	Cursor targets[TargetCount];
	Cursor sources[SourceCount];

	BasicBlitContext(ImageSlice (&to)[TargetCount], ImageSlice (&from)[SourceCount], i32 x, i32 y);
#if 0
	BasicBlitContext(ImageSlice (&to)[TargetCount], ImageSlice (&from)[SourceCount], UnsafeNoClip);
#endif

	static BasicBlitContext one_source(ImageSlice to, ImageSlice from, i32 x, i32 y) {
		ImageSlice t[1] = { to };
		ImageSlice s[1] = { from };

		return BasicBlitContext(t, s, x, y);
	}

	int blit(Palette* pal, u32 flags);
};

#if 0 // TODO: When we find use
template<u32 TargetCount, u32 SourceCount, bool KeepOffset>
BasicBlitContext<TargetCount, SourceCount, KeepOffset>::BasicBlitContext(ImageSlice (&to)[TargetCount], ImageSlice (&from)[SourceCount], UnsafeNoClip) {
	this->dim = from[0].dim;

	// TODO: Verify that other sources have the same w/h
	// TODO: Verify that other targets have the same w/h

	for (u32 i = 0; i < SourceCount; ++i) {
		u32 fbpp = from[i].bpp;
		sources[i].pixels = from[i].data();
		sources[i].pitch = from[i].pitch;
		sources[i].bpp = fbpp;
	}

	for (u32 i = 0; i < TargetCount; ++i) {
		u32 tbpp = to[i].bpp;
		targets[i].pixels = to[i].data();
		targets[i].pitch = to[i].pitch;
		targets[i].bpp = tbpp;
	}
}
#endif

template<u32 TargetCount, u32 SourceCount, bool KeepOffset>
BasicBlitContext<TargetCount, SourceCount, KeepOffset>::BasicBlitContext(ImageSlice (&to)[TargetCount], ImageSlice (&from)[SourceCount], i32 x, i32 y) {
	
	this->init_blit_context(sources, to, TargetCount, targets, from, SourceCount, x, y);
	this->set_offset(tl::VectorU2((u32)x, (u32)y));
}

typedef BasicBlitContext<1, 1> BlitContext;

inline int ImageSlice::blit(ImageSlice const& from, Palette* pal, i32 x, i32 y, u32 flags) {
	return BlitContext::one_source(*this, from, x, y).blit(pal, flags);
}


#if 0

template<typename T>
struct MallocContainer : T {
	void* realloc(void* p, usize sz) {
		return realloc(p, sz);
	}

	~MallocContainer() {
		this->T::destroy();
	}
};

template<template<typename T> Container>
void alloc_uninit(Container<ImageSlice>& image, u32 w, u32 h, u32 bpp_init) {
	image.pixels = (u8 *)image.realloc(image.pixels, w * h * bpp_init, image.dim.x * image.dim.y * image.bpp);
	image.dim = VectorU2(w, h);
	image.pitch = w * bpp_init;
	image.bpp = bpp_init;
}

#endif

}

#endif // UUID_917ECCD8563741D5491DF687EA1F876D
