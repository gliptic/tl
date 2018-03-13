#include "memo.hpp"
#include "filesystem.hpp"

namespace tl {

bool should_run(tl::StringSlice name, tl::VecSlice<tl::String> inputs, tl::VecSlice<tl::String> outputs) {
	auto oldest_output = std::numeric_limits<tl::FileTimestamp>::max();
	for (auto& o : outputs) {
		tl::FileStat st;
		if (tl::stat(tl::c_str(o.slice_const()), st)) {
			oldest_output = std::numeric_limits<tl::FileTimestamp>::min();
			break;
		} else {
			oldest_output = tl::min(oldest_output, st.st_mtime);
		}
	}

	auto newest_input = std::numeric_limits<tl::FileTimestamp>::min();
	for (auto& i : inputs) {
		tl::FileStat st;
		if (tl::stat(tl::c_str(i.slice_const()), st)) {
			// Error
			break;
		}

		newest_input = tl::max(newest_input, st.st_mtime);
	}

	return newest_input >= oldest_output;
}

}
