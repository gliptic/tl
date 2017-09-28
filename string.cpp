#include "string.hpp"

namespace tl {

StringTerminated c_str(StringSlice str) {
	StringTerminated result(str.size() + 1);
	memcpy(result.begin(), str.begin(), str.size());
	result[str.size()] = 0;
	result.unsafe_set_size(str.size() + 1);

	return move(result);
}

StringSliceTerminated from_c_str(char const* str) {
	return StringSliceTerminated((u8 const*)str, (u8 const*)str + strlen(str));
}

}