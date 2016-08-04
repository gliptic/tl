#ifndef TL_STRING_HPP
#define TL_STRING_HPP

#include "std.h"
#include "vec.hpp"

namespace tl {

typedef Vec<u8> String;
typedef VecSlice<u8 const> StringSlice;

String c_str(StringSlice str);

}

inline tl::StringSlice operator"" _S(char const* p, usize len) {
	return tl::StringSlice((u8 const*)p, (u8 const*)p + len);
}



#endif // TL_STRING_HPP
