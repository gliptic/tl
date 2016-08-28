#include "string.hpp"

namespace tl {

Vec<char> c_str(StringSlice str) {
	Vec<char> result(str.size() + 1);
	memcpy(result.begin(), str.begin(), str.size());
	result[str.size()] = 0;
	result.unsafe_set_size(str.size() + 1);

	return move(result);
}

StringSlice from_c_str(char const* str) {
	return StringSlice((u8 const*)str, (u8 const*)str + strlen(str));
}

}