#ifndef TL_ARCHIVE_HPP
#define TL_ARCHIVE_HPP 1

#include "../string.hpp"
#include "stream.hpp"

namespace tl {

struct StreamRef {
	u32 index;

	StreamRef(StreamRef const&) = default;
	StreamRef& operator=(StreamRef const&) = default;

	StreamRef(u32 index_init) : index(index_init) {
	}
};

struct FileRef {
	FileRef(StreamRef stream, u32 offset, u32 size)
		: stream(stream), offset(offset), size(size) {
	}

	StreamRef stream;
	u32 offset;
	u32 size;
};

struct EntryRef;

struct TreeRef {
	u32 index;

	TreeRef(TreeRef const&) = default;
	TreeRef& operator=(TreeRef const&) = default;

	TreeRef(u32 index_init) : index(index_init) {
	}
};

union EntryContentsRef {

	EntryContentsRef(TreeRef tree_init)
		: tree(tree_init) {
	}

	EntryContentsRef(FileRef file)
		: file(file) {
	}

	~EntryContentsRef() {
	}

	TreeRef tree;
	FileRef file;
};

struct EntryRef {
	EntryRef(tl::String name, TreeRef tree)
		: name(move(name)), is_tree(true), contents(tree) {
	}

	EntryRef(tl::String name, FileRef file)
		: name(move(name)), is_tree(false), contents(file) {
	}

	tl::String name;
	EntryContentsRef contents;
	bool is_tree;
};

struct EntryTree {
	EntryTree() : pos(0) {
	}

	u32 pos;
	tl::Vec<EntryRef> children;
};

struct Stream {
	Stream(u32 encoding) : pos(0), uncompressed_size(0), encoding(encoding) {
	}

	tl::Vec<u8> data;
	u32 pos;
	u32 uncompressed_size;
	u32 encoding;
};

struct Encoding {

};

struct ArchiveBuilder {
	tl::Vec<EntryTree> trees;
	tl::Vec<Stream> streams;
	tl::Vec<Encoding> encoding;

	ArchiveBuilder() {
		trees.push_back(EntryTree());
	}

	tl::Vec<u8> write();

	TreeRef root() {
		return TreeRef(0);
	}

	StreamRef add_stream() {
		u32 index = tl::narrow<u32>(streams.size());
		streams.push_back(Stream(0));
		return StreamRef(index);
	}

	EntryRef add_file(tl::String name, StreamRef stream, tl::VecSlice<u8 const> contents) {
		auto& str = streams[stream.index];
		u32 uncompressed_size = tl::narrow<u32>(contents.size());
		u32 offset = str.uncompressed_size;
		u8* p = str.data.unsafe_alloc(contents.size());
		memcpy(p, contents.begin(), contents.size());

		str.uncompressed_size += uncompressed_size;
		return EntryRef(move(name), FileRef(stream, offset, uncompressed_size));
	}

	EntryRef add_dir(tl::String name) {
		u32 index = tl::narrow<u32>(trees.size());
		trees.push_back(EntryTree());
		return EntryRef(move(name), index);
	}

	void add_entry_to_dir(EntryRef const& parent, EntryRef child) {
		assert(parent.is_tree);
		add_entry_to_dir(parent.contents.tree, move(child));
	}

	void add_entry_to_dir(TreeRef parent, EntryRef child) {
		auto& tree = trees[parent.index];
		tree.children.push_back(move(child));
	}
};

/*
tree:
	u32 children_count
	repeat children_count:
		u8 name_size
		u8 name[name_size]
		u31 offset; u1 is_tree;
		if !is_tree:
			u32 size;
			u32 pos
*/

struct ArchiveReader {
	tl::VecSlice<u8 const> vec;
	u8 const* indexp;

	struct Node {
		tl::VecSlice<u8 const> vec;
		u8 const* base;
		bool is_tree;

		operator void const*() const {
			return this->base;
		}

		Node() : base(0) {
		}

		Node(tl::VecSlice<u8 const> vec, u8 const* base, bool is_tree)
			: vec(vec), base(base), is_tree(is_tree) {
		}

		Node find(tl::StringSlice needle) {
			if (!is_tree) return Node();

			usize needle_size = needle.size();

			u8 const* p = base;
			u32 count = tl::read_le<u32>(p);
			p += 4;
			
			for (u32 i = 0; i < count; ++i) {
				if (p == vec.end()) return Node();

				u8 name_size = *p++;
				if (vec.end() - p < name_size + 4) return Node();

				u8 const* namep = p;
				p += name_size;
				u32 offset = tl::read_le<u32>(p);
				p += 4;

				if (needle_size == name_size && memcmp(namep, needle.begin(), name_size) == 0) {
					
					if (offset & 1) {
						offset >>= 1;
						if (offset + 4 > vec.size()) return Node();

						return Node(vec, vec.begin() + offset, true);
					} else {
						offset >>= 1;
						if (vec.end() - p < 8) return Node();

						return Node(vec, p, false);
					}
				}

				if ((offset & 1) == 0) {
					if (vec.end() - p < 8) return Node();
					p += 4 + 4;
				}
			}

			return Node();
		}

		tl::VecSlice<u8 const> extract() {
			if (is_tree) return tl::VecSlice<u8 const>();

			u32 offset = tl::read_le<u32>(base - 4) >> 1;
			u32 size = tl::read_le<u32>(base);
			u32 pos = tl::read_le<u32>(base + 4);

			if (pos + offset + size > vec.size()) {
				return tl::VecSlice<u8 const>();
			}

			return tl::VecSlice<u8 const>(vec.begin() + pos + offset, vec.begin() + pos + offset + size);
		}
	};

	operator void const*() const {
		return this->indexp;
	}

	static ArchiveReader from(tl::VecSlice<u8 const> vec) {
		usize size = vec.size();
		if (size < 4 + 4)
			return ArchiveReader();

		u8 const* p = vec.begin();
		u32 index_offs = tl::read_le<u32>(p + 4);

		if (size < index_offs + 4)
			return ArchiveReader();

		return ArchiveReader(vec, p + index_offs);
	}

	Node root() {
		return Node(vec, indexp, true);
	}

private:
	ArchiveReader() : indexp(0) {
	}

	ArchiveReader(tl::VecSlice<u8 const> vec, u8 const* indexp)
		: vec(vec), indexp(indexp) {
		
	}


};


}

#endif // TL_ARCHIVE_HPP
