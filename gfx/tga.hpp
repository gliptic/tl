#ifndef TGA_HPP
#define TGA_HPP 1

#include "../io/stream.hpp"
#include "image.hpp"

namespace tl {

int read_tga(Source& src, Image& img, Palette& pal);
int write_tga(Sink& sink, ImageSlice img, Palette const& pal);

}

#endif // TGA_HPP
