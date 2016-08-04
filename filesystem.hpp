#ifndef TL_FILESYSTEM_HPP
#define TL_FILESYSTEM_HPP

#include "string.hpp"
#include "shared_ptr.hpp"
#include "stream.hpp"

namespace tl {

struct FsNodeImp : RcNode {
	virtual Rc<FsNodeImp> go(StringSlice name) = 0;
	virtual source try_get_source() = 0;
};

struct FsNode {
	Rc<FsNodeImp> imp;

	FsNode(FsNodeImp* imp_init)
		: imp(imp_init) {
	}

	FsNode(Rc<FsNodeImp>&& imp_init)
		: imp(move(imp_init)) {
	}

	FsNode operator/(StringSlice name) {
		return FsNode(imp->go(name));
	}

	source try_get_source() {
		return imp->try_get_source();
	}
};

struct FsNodeFilesystem : FsNodeImp {
	String path;

	FsNodeFilesystem(String&& path_init) 
		: path(move(path_init)) {
	}

	static String join(StringSlice a, StringSlice b) {
		bool add_slash = false;
		if (a.empty() || a.end()[-1] != '/') {
			add_slash = true;
		}

		String result(a.size() + 1 + b.size());

		u8* dest = result.begin();
		memcpy(dest, a.begin(), a.size());
		dest += a.size();
		if (add_slash) *dest++ = '/';
		memcpy(dest, b.begin(), b.size());
		dest += b.size();
		result.unsafe_set_size(dest - result.begin());

		return move(result);
	}

	Rc<FsNodeImp> go(StringSlice name) {
		String full_path(join(this->path.slice_const(), name));

		return Rc<FsNodeImp>(new FsNodeFilesystem(move(full_path)));
	}

	source try_get_source() {
		return source::from_file(path.slice_const());
	}
};

}

#endif // TL_FILESYSTEM_HPP
