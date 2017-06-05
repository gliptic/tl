#ifndef TL_STRING_HPP
#define TL_STRING_HPP

#include "std.h"
#include "vec.hpp"

namespace tl {

typedef Vec<u8> String;
typedef VecSlice<u8 const> StringSlice;

/* TODO
struct StringSliceTerminated : StringSlice {
	using StringSlice::StringSlice;
};
*/

struct StringSliceLiteral : StringSlice {
	using StringSlice::StringSlice;
};

Vec<char> c_str(StringSlice str);
StringSlice from_c_str(char const* str);

}

inline tl::StringSliceLiteral operator"" _S(char const* p, usize len) {
	return tl::StringSliceLiteral((u8 const*)p, (u8 const*)p + len);
}

#endif // TL_STRING_HPP
