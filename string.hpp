#ifndef TL_STRING_HPP
#define TL_STRING_HPP

#include "std.h"
#include "vec.hpp"

namespace tl {

typedef Vec<u8> String;
typedef VecSlice<u8 const> StringSlice;

struct StringSliceTerminated : StringSlice {
	using StringSlice::StringSlice;

	char const* c_str() const { return (char const *)this->begin();  }
};

struct StringSliceLiteral : StringSliceTerminated {
	using StringSliceTerminated::StringSliceTerminated;
};

struct StringTerminated : Vec<char> {
	using Vec::Vec;

	operator char const*() const {
		return (char const *)this->begin();
	}
};

StringTerminated c_str(StringSlice str);
StringSliceTerminated from_c_str(char const* str);

}

inline tl::StringSliceLiteral operator"" _S(char const* p, usize len) {
	return tl::StringSliceLiteral((u8 const *)p, (u8 const *)p + len);
}

#endif // TL_STRING_HPP
