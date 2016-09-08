#include "filesystem.hpp"
#include "archive.hpp"
#include "../shared_ptr.hpp"

#if TL_WINDOWS
#include "../windows/win.hpp"
#endif

namespace tl {

struct ArchiveHolder : RcNode {
	Source src;

	ArchiveHolder(Source src) : src(move(src)) {
	}
};


static Rc<FsNodeImp> join_nodes(Rc<FsNodeImp> a, Rc<FsNodeImp> b);

struct FsNodeJoin : FsNodeImp {

	Rc<FsNodeImp> a, b;

	FsNodeJoin(Rc<FsNodeImp> a_init, Rc<FsNodeImp> b_init)
		: a(std::move(a_init)), b(std::move(b_init)) {
	}

	Rc<FsNodeImp> go(StringSlice name) {
		return join_nodes(a->go(name), b->go(name));
	}

	/*
	bool exists() const {
		return a->exists() || b->exists();
	}
	*/

	Source try_get_source() {
		auto s = a->try_get_source();
		if (s) return move(s);
		return b->try_get_source();
	}

	Sink try_get_sink() {
		auto s = a->try_get_sink();
		if (s) return move(s);
		return b->try_get_sink();
	}
};

static Rc<FsNodeImp> join_nodes(Rc<FsNodeImp> a, Rc<FsNodeImp> b) {

	if (!b) return move(a);
	if (!a) return move(b);
	return Rc<FsNodeImp>(new FsNodeJoin(move(a), move(b)));
}

struct FsNodeHrc : FsNodeImp {
	Rc<ArchiveHolder> holder;
	ArchiveReader::Node node;

	FsNodeHrc(Source src)
		: holder(new ArchiveHolder(move(src))) {
		
		this->node = ArchiveReader::from(holder->src.read_all()).root();
	}

	explicit FsNodeHrc(FsNodeHrc const& other)
		: holder(other.holder.clone()), node(other.node) {
	}

	explicit FsNodeHrc(FsNodeHrc const& other, ArchiveReader::Node node_init)
		: holder(other.holder.clone()), node(node_init) {
	}

	Rc<FsNodeImp> go(StringSlice name) {
		auto subnode = node.find(name);

		if (subnode) {
			return Rc<FsNodeImp>(new FsNodeHrc(*this, subnode));
		}

		return Rc<FsNodeImp>();
	}

	Source try_get_source() {
		auto vec = node.extract();
		if (vec.empty()) { // TODO: Empty files will count as non-existing like this
			return Source();
		}

		return Source(std::make_unique<EofPullable>(), vec);
	}

	Sink try_get_sink() {
		return Sink();
	}
};

static bool file_exists(LPCTSTR path) {
	DWORD attrib = GetFileAttributes(path);

	return attrib != INVALID_FILE_ATTRIBUTES
		&& !(attrib & FILE_ATTRIBUTE_DIRECTORY);
}

Rc<FsNodeImp> FsNode::from_path(String&& full_path) {
	auto hrc_path = String::concat(full_path.slice(), ".hrc"_S, u8(0));
	auto first = Rc<FsNodeImp>(new FsNodeFilesystem(move(full_path)));

	if (file_exists((char const *)hrc_path.begin())) {
		auto src = Source::from_file((char const *)hrc_path.begin());
		return join_nodes(Rc<FsNodeImp>(new FsNodeHrc(move(src))), move(first));
	}

	return move(first);
}

Rc<FsNodeImp> FsNodeFilesystem::go(StringSlice name) {
	return FsNode::from_path(join(this->path.slice_const(), name));
}

}
