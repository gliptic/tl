#ifndef WAV_HPP
#define WAV_HPP 1

#include "io/stream.hpp"
#include "vec.hpp"

int read_wav(
	tl::Source src,
	tl::Vec<i16>& sound);

#endif // WAV_HPP
