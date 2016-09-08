#include "stream.hpp"

namespace tl {

usize Pullable::move_data_left(Source& src, u8* new_buffer) {
	usize data_left = src.size();
	if (data_left != 0 && src.begin() != new_buffer) {
		// Data left. Must move it to beginning.
		memmove(new_buffer, src.begin(), data_left);
	}

	src.unsafe_set_buffer(new_buffer, new_buffer + data_left);

	return data_left;
}

}
