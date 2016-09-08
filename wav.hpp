#ifndef WAV_HPP
#define WAV_HPP 1

#include "io/stream.hpp"
#include "vec.hpp"

namespace tl {

int write_wav(Sink& dest, VecSlice<u8 const>& sound);
int read_wav(Source src, Vec<i16>& sound);

}

#endif // WAV_HPP
