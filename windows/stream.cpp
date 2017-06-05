#include "../io/stream.hpp"
#include "win.hpp"
//#include "miniwindows.h"

namespace tl {

struct FilePullableWin : FilePullable {
	FilePullableWin(HANDLE f)
		: f(f) {
	}

	~FilePullableWin() {
		if (f) CloseHandle(f);
	}

	Result pull(Source& src, usize min_size);

	HANDLE f;
};

struct FilePushableWin : FilePushable {
	FilePushableWin(HANDLE f)
		: f(f) {
	}

	~FilePushableWin() {
		if (f) CloseHandle(f);
	}

	int push(SinkBuf& src, usize min_size);

	HANDLE f;
};

struct FilePullableMappingWin : EofPullable {
	FilePullableMappingWin(void* base, HANDLE f, HANDLE fm)
		: base(base), f(f), fm(fm) {
	}

	~FilePullableMappingWin() {
		if (base) UnmapViewOfFile(base);
		if (fm) CloseHandle(fm);
		if (f) CloseHandle(f);
	}

	void* base;
	HANDLE f, fm;
};

static Source open_mapping(char const* path) {
	HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE fm = INVALID_HANDLE_VALUE;
	void* p = NULL;
	tl::VecSlice<u8 const> sl;

	if (h == INVALID_HANDLE_VALUE) {
		printf("%s: %d\n", path, GetLastError());
		return Source();
	}

	fm = CreateFileMappingA(h, NULL, PAGE_READONLY, 0, 0, NULL);

	if (fm == INVALID_HANDLE_VALUE) {
		printf("%s: %d\n", path, GetLastError());
		CloseHandle(h);
		return Source();
	}

	p = MapViewOfFile(fm, FILE_MAP_READ, 0, 0, 0);

	if (p == NULL) {
		printf("%s: %d\n", path, GetLastError());
		CloseHandle(h);
		CloseHandle(fm);
		return Source();
	}

	MEMORY_BASIC_INFORMATION meminfo;
	if (VirtualQuery(p, &meminfo, sizeof(meminfo)) != 0) {
		sl = tl::VecSlice<u8 const>((u8 const*)p, (u8 const*)p + meminfo.RegionSize);
	}

	return Source(std::make_unique<FilePullableMappingWin>(p, h, fm), sl);
}

static Source open(char const* path) {
	HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) {
		printf("%d\n", GetLastError());
	}
	return Source(std::make_unique<FilePullableWin>(h));
}

FilePushable* FilePushable::open(char const* path) {
	HANDLE h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) {
		printf("%d\n", GetLastError());
	}
	return new FilePushableWin(h);
}

Pullable::Result FilePullableWin::pull(Source& src, usize min_size) {
	usize space_left = pull_begin(src, min_size);

	DWORD c;
	BOOL r = ReadFile(this->f, (void *)src.end(), DWORD(space_left), &c, NULL);

	pull_done(src, c);

	if (!r) {
		printf("err: %d\n", GetLastError());
		return Pullable::Result::Error;
	}

	return (c < space_left ? Result::Eof : Result::None)
		 | (src.size() >= min_size ? Result::HasRequestedAmount : Result::None);
}

Source Source::from_file(char const* path) {
	return open_mapping(path);
}

Source Source::from_file(StringSlice path) {
	Vec<char> path_cstr(c_str(path));
	return open_mapping(path_cstr.begin());
}


// Sink

int FilePushableWin::push(SinkBuf& src, usize min_size) {
	auto to_write = src.get_to_write();
	usize amount_to_write = to_write.size();
	DWORD written;
	BOOL r = WriteFile(this->f, to_write.begin(), (DWORD)amount_to_write, &written, NULL);

	this->base_position += written;
	src.unsafe_cut_front(written);

	assert(src.to_write_size() == 0);

	src.unsafe_set_buffer(buffer, buffer, buffer + buffer_size);
	usize s = buffer_size;

	if (s < min_size) {
		//usize cur_offs = to_write.end() - buffer;
		buffer = (u8 *)memrealloc(buffer, min_size, buffer_size);
		buffer_size = min_size;
		src.unsafe_set_buffer(buffer, buffer, buffer + buffer_size);
	}

	return written < amount_to_write;
}

Sink Sink::from_file(char const* path) {
	return Sink(std::unique_ptr<Pushable>(FilePushable::open(path)));
}

Sink Sink::from_file(StringSlice path) {
	Vec<char> path_cstr(c_str(path));
	return Sink(std::unique_ptr<Pushable>(FilePushable::open(path_cstr.begin())));
}

}