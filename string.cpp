#include "string.hpp"

namespace tl {

String c_str(StringSlice str) {
	String result(str.size() + 1);
	memcpy(result.begin(), str.begin(), str.size());
	result[str.size()] = 0;
	result.unsafe_set_size(str.size() + 1);

	return move(result);
}

}