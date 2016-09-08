#include "archive.hpp"

using std::move;

namespace tl {

inline u32 quad(char a, char b, char c, char d) {
	return (u32)a + ((u32)b << 8) + ((u32)c << 16) + ((u32)d << 24);
}

tl::Vec<u8> ArchiveBuilder::write() {

	tl::SinkVector out;
	out.put_raw(quad('h', 'a', 'r', 'c'));
	out.put_raw(u32(0)); // Offset to tree table

	// Write streams
	for (auto& str : this->streams) {
		str.pos = u32(out.position());
		out.put(str.data.slice_const());
	}

	// Write trees from the leaves
	for (usize i = this->trees.size(); i-- > 0; ) {
		auto& tree = this->trees[i];
		tree.pos = u32(out.position());

		out.put_raw((u32)tree.children.size());

		for (auto& child : tree.children) {
			assert(child.name.size() <= 255);
			out.put((u8)child.name.size());
			out.put(child.name.slice_const());

			if (child.is_tree) {
				auto& child_tree = this->trees[child.contents.tree.index];
				out.put_raw(u32((child_tree.pos << 1) | 1));
			} else {
				out.put_raw(u32((child.contents.file.offset << 1) | 0));
				out.put_raw(u32(child.contents.file.size));

				auto& stream = this->streams[child.contents.file.stream.index];
				out.put_raw(u32(stream.pos));
			}
		}
	}

	auto vec = out.unwrap_vec();
	memcpy(vec.begin() + 4, &trees[0].pos, sizeof(u32));

	return move(vec);
}

#if 0
void archive_test() {
	ArchiveBuilder ar;

	{
		auto str = ar.add_stream();
		auto subfile = ar.add_file(tl::String("subfile"_S), str, "1"_S);
		auto dir = ar.add_dir(tl::String("test"_S));
		ar.add_entry_to_dir(dir, move(subfile));

		ar.add_entry_to_dir(ar.root(), move(dir));
	}

	//tl::SinkVector out;

	auto vec = ar.write();

	//auto vec = out.unwrap_vec();
	//memcpy(vec.begin() + 4, &ar.trees[0].pos, sizeof(u32));
	printf("%d\n", (u32)vec.size());
}
#endif

}
