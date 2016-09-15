#include "image.hpp"

#include <utility>
#include <algorithm>

namespace tl {

void ImageSlice::free() {
	::free(this->pixels);
}

void Image::blit_unsafe(Image const& from, u32 x, u32 y) {
	assert(x <= this->dim.x && x + from.dim.x <= this->dim.x);
	assert(y <= this->dim.y && y + from.dim.y <= this->dim.y);
	assert(this->bpp == from.bpp);

	u8* tp = this->ptr(x, y, this->bpp);
	u8* fp = from.pixels;
	u32 tpitch = this->pitch;
	u32 fline = from.dim.x * from.bpp;
	u32 fpitch = from.pitch;
	u32 hleft = from.dim.y;

	while (hleft-- > 0)	{
		memcpy(tp, fp, fline);
		tp += tpitch;
		fp += fpitch;
	}
}

enum ConvertMode {
	Copy,
	ExpandToAlpha,
	ExpandPalette,
	ExpandGrayscale,
	Transparent32,
	Flip
};

template<>
int BasicBlitContext<1, 1>::blit(Palette* pal, u32 flags) {
	u32 hleft = this->dim.y;
	u32 w = this->dim.x;
	u32 tbpp = this->targets[0].bpp;
	u32 fbpp = this->sources[0].bpp;
	u8* tp = this->targets[0].pixels;
	u8* fp = this->sources[0].pixels;
	u32 tpitch = this->targets[0].pitch;
	u32 fpitch = this->sources[0].pitch;
	u32 i;
	
	//if (this->w != from.w || this->h != from.h)
	//	return -1;

	assert(fbpp >= 1 && fbpp <= 4);
	assert(tbpp >= 1 && tbpp <= 4);

	{
		u32 id = ((fbpp << 2) + tbpp) - ((1<<2)+1);
		// id is a value in [0, 16)

		#define FT(f,t) ((((f)-1)<<2)+((t)-1))

		ConvertMode mode;
		if (fbpp == tbpp) {
			if (flags & ImageSlice::Flip) {
				mode = Flip;
			} else if (flags & ImageSlice::BlitTransparent) {
				mode = Transparent32;
			} else {
				mode = Copy;
			}
		} else switch (id) {
			case FT(1,4): {
				if (flags & ImageSlice::AsGrayscale) {
					mode = ExpandGrayscale;
				} else if (pal) {
					mode = ExpandPalette;
				} else {
					mode = ExpandToAlpha;
				}

				break;
			}

			default:
				return 1;
		}

		#undef FT

		for(; hleft-- > 0; tp += tpitch, fp += fpitch) {
			switch (mode) {
			case Copy:
				memcpy(tp, fp, w * fbpp);
				break;

			case Transparent32:
				for (i = 0; i < w; ++i) {
					auto c = Color::read(fp + i*4);
					if (c.a()) {
						Color::write(tp + i*4, c);
					}
				}
				break;

			case ExpandGrayscale:
				for (i = 0; i < w; ++i) {
					u8 f = fp[i];
					tp[i*4  ] = f;
					tp[i*4+1] = f;
					tp[i*4+2] = f;
					tp[i*4+3] = 255;
				}
				break;

			case ExpandToAlpha:
				for (i = 0; i < w; ++i) {
					u8 f = fp[i];
					tp[i*4  ] = 255;
					tp[i*4+1] = 255;
					tp[i*4+2] = 255;
					tp[i*4+3] = f;
				}
				break;

			case ExpandPalette:
				for (i = 0; i < w; ++i) {
					u8 f = fp[i];
					u32* t = (u32 *)&tp[i*4];
					*t = pal->entries[f].with_a(f).v;
				}
				break;
			
			case Flip:
				for (i = 0; i < w; ++i) {
					u8* f = fp + i*fbpp;
					u8* t = tp + (w - i - 1)*tbpp;
					memcpy(t, f, fbpp);
				}
				break;
			}
		}
	}

	return 0;

	// 1 L
	// 2 L A
	// 3 R G B
	// 4 R G B A


	// L       -> L *
	// L       -> L L L
	// L       -> L L L *
	// L A     -> L
	// L A     -> L L L
	// L A     -> L L L A
	// R G B   -> M
	// R G B   -> M *
	// R G B   -> R G B *
	// R G B A -> M
	// R G B A -> M *
	// R G B A -> R G B
}

void BaseBlitContext::clip(i32& x, i32& y, VectorU2& src, VectorU2 todim) {
	if (y < 0) {
		if (y < -(i32)dim.y) {
			dim.y = 0;
		} else {
			src.y = (u32)-y;
			dim.y += y;
			y = 0;
		}
	}
	
	if (y + (i32)dim.y > (i32)todim.y) {
		if (y >= (i32)todim.y) {
			dim.y = 0;
		} else {
			dim.y = todim.y - y;
		}
	}

	if (x < 0) {
		if (x < -(i32)dim.x) {
			dim.x = 0;
		} else {
			src.x = (u32)-x;
			dim.x += x;
			x = 0;
		}
	}

	if (x + (i32)dim.x > (i32)todim.x) {
		if (x >= (i32)todim.x) {
			dim.x = 0;
		} else {
			dim.x = todim.x - x;
		}
	}
}

int Image::pad(Image const& from) {
	assert(this->dim.x == from.dim.x + 2);
	assert(this->dim.y == from.dim.y + 2);

	memset(this->pixels, 0, this->size());
	this->blit_unsafe(from, 1, 1);
	return 0;
}

Image::Image(u32 w, u32 h, u32 bpp)
	: ImageSlice((u8 *)memalloc(w * h * bpp), w, h, w * bpp, bpp) {
}

void Image::alloc_uninit(u32 w, u32 h, u32 bpp_init) {
	this->pixels = (u8 *)memrealloc(pixels, w * h * bpp_init, this->dim.x * this->dim.y * this->bpp);
	this->dim = VectorU2(w, h);
	this->pitch = w * bpp_init;
	this->bpp = bpp_init;
}

void Image::copy_from(Image const& other) {
	usize size = other.pitch * other.dim.y;
	this->pixels = (u8 *)realloc(this->pixels, size);
	this->dim = other.dim;
	this->pitch = other.pitch;
	this->bpp = other.bpp;
	memcpy(this->pixels, other.pixels, size);
}

Image ImageSlice::convert(u32 bpp_new, tl::Palette* pal) {
	Image ret(this->dim.x, this->dim.y, bpp_new);
	ret.blit(*this, pal);
	return std::move(ret);
}

void BaseBlitContext::init_blit_context(
	tl::Cursor* sources, ImageSlice* to, usize target_count,
	tl::Cursor* targets, ImageSlice* from, usize source_count, i32& x, i32& y) {

	this->dim = from[0].dim;
	VectorU2 todim = to[0].dim;

	// TODO: Verify that other sources have the same w/h
	// TODO: Verify that other targets have the same w/h

	VectorU2 src; //u32 src_x = 0, src_y = 0;
	clip(x, y, src, todim);

	for (u32 i = 0; i < source_count; ++i) {
		u32 fbpp = from[i].bpp;
		sources[i].pixels = from[i].ptr(src.x, src.y, fbpp);
		sources[i].pitch = from[i].pitch;
		sources[i].bpp = fbpp;
	}

	for (u32 i = 0; i < target_count; ++i) {
		u32 tbpp = to[i].bpp;
		targets[i].pixels = to[i].ptr((u32)x, (u32)y, tbpp);
		targets[i].pitch = to[i].pitch;
		targets[i].bpp = tbpp;
	}
}


} // namespace tl
