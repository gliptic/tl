#ifndef TL_FILESYSTEM_HPP
#define TL_FILESYSTEM_HPP

#include "../string.hpp"
#include "../shared_ptr.hpp"
#include "stream.hpp"

namespace tl {

struct FsNodeImp : RcNode {
	virtual Rc<FsNodeImp> go(StringSlice name) = 0;
	virtual Source try_get_source() = 0;
	virtual Sink try_get_sink() = 0;
};

struct FsNode {
	Rc<FsNodeImp> imp;

	inline FsNode(String&& path_init);

	FsNode(FsNodeImp* imp_init)
		: imp(imp_init) {
	}

	FsNode(Rc<FsNodeImp>&& imp_init)
		: imp(move(imp_init)) {
	}

	FsNode operator/(StringSlice name) {
		return FsNode(imp->go(name));
	}

	Source try_get_source() {
		return imp->try_get_source();
	}

	Sink try_get_sink() {
		return imp->try_get_sink();
	}
};

struct FsNodeFilesystem : FsNodeImp {
	String path;

	FsNodeFilesystem(String&& path_init)
		: path(move(path_init)) {

		if (!path.empty()) {
			u8 l = path.back();
			if (l == '/' || l == '\\')
				path.unsafe_pop();
		}
	}

	// NOTE: Assumes 'a' does not have a directory separator at the end
	static String join(StringSlice a, StringSlice b) {

		String result(a.size() + 1 + b.size());

		u8* dest = result.begin();
		memcpy(dest, a.begin(), a.size());
		dest += a.size();
		*dest++ = '/';
		memcpy(dest, b.begin(), b.size());
		dest += b.size();
		result.unsafe_set_size(dest - result.begin());

		return move(result);
	}

	Rc<FsNodeImp> go(StringSlice name) {
		String full_path(join(this->path.slice_const(), name));

		return Rc<FsNodeImp>(new FsNodeFilesystem(move(full_path)));
	}

	Source try_get_source() {
		return Source::from_file(path.slice_const());
	}

	Sink try_get_sink() {
		return Sink::from_file(path.slice_const());
	}
};

FsNode::FsNode(String&& path_init)
	: imp(new FsNodeFilesystem(std::move(path_init))) {
}

}

#endif // TL_FILESYSTEM_HPP
