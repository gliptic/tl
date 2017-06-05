#ifndef TL_APPROXMATH_HPP
#define TL_APPROXMATH_HPP 1

#include "../vector.hpp"

namespace tl {

static f64 const pi = 3.1415926535897932384626433832795;
static f64 const pi2 = 6.283185307179586476925286766559;

VectorD2 sincos(double x);
VectorI2 sincos_fixed(i32 x);
VectorI2 sincos_fixed2(i32 x);
VectorD2 sincos_f64(i32 x);

VectorD2 sincos_che(double x);

}

#endif // TL_APPROXMATH_HPP

