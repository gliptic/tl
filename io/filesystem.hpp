#ifndef TL_FILESYSTEM_HPP
#define TL_FILESYSTEM_HPP

#include "../string.hpp"
#include "../shared_ptr.hpp"
#include "stream.hpp"
#include <sys/stat.h>

#if TL_WINDOWS
#include <io.h>
#else
#include <unistd.h>
#endif

namespace tl {

struct FsNodeImp : RcNode {
	virtual Rc<FsNodeImp> go(StringSlice name) = 0;
	virtual Source try_get_source() = 0;
	virtual Sink try_get_sink() = 0;
};

struct FsNode {

	static Rc<FsNodeImp> from_path(String&& full_path);

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

private:
	Rc<FsNodeImp> imp;
	//inline FsNode(String&& path_init);
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

		return String::concat(a, u8('/'), b);
	}

	Rc<FsNodeImp> go(StringSlice name);

	Source try_get_source() {
		return Source::from_file(path.slice_const());
	}

	Sink try_get_sink() {
		return Sink::from_file(path.slice_const());
	}
};

#if TL_WINDOWS
typedef struct _stati64 FileStat;
typedef __time64_t FileTimestamp;
inline int stat(char const* path, FileStat& stat) { return _stati64(path, &stat); }
#else
# error "Not implemented"
#endif

/*
FsNode::FsNode(String&& path_init)
	: imp(new FsNodeFilesystem(std::move(path_init))) {
}*/

}

#endif // TL_FILESYSTEM_HPP
