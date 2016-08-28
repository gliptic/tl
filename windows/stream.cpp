#include "../io/stream.hpp"
#include "win.hpp"

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

struct FilePullableMappingWin : Pullable {
	FilePullableMappingWin(HANDLE f)
		: f(f) {
	}

	~FilePullableMappingWin() {
		if (f) CloseHandle(f);
	}

	Result pull(Source& src, usize min_size);

	HANDLE f;
};

static FilePullableMappingWin* open_mapping(char const* path) {
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) {
		printf("%d\n", GetLastError());
	}

	HANDLE fm = CreateFileMapping(h, NULL, PAGE_READONLY, 0, 0, NULL);

	if (fm == INVALID_HANDLE_VALUE) {
		printf("%d\n", GetLastError());
	}

	//MapViewOfFile(fm, FILE_MAP_READ, )

	return new FilePullableMappingWin(h);
}

FilePullable* FilePullable::open(char const* path) {
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) {
		printf("%d\n", GetLastError());
	}
	return new FilePullableWin(h);
}

FilePushable* FilePushable::open(char const* path) {
	HANDLE h = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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
	return Source(std::unique_ptr<Pullable>(FilePullable::open(path)));
}

Source Source::from_file(StringSlice path) {
	Vec<char> path_cstr(c_str(path));
	return Source(std::unique_ptr<Pullable>(FilePullable::open(path_cstr.begin())));
}


// Sink

int FilePushableWin::push(SinkBuf& src, usize min_size) {
	auto to_write = src.get_written();
	usize amount_to_write = to_write.size();
	DWORD written;
	BOOL r = WriteFile(this->f, to_write.begin(), (DWORD)amount_to_write, &written, NULL);

	this->base_position += written;
	src.unsafe_cut_front(written);

	usize s = src.size();

	if (s == 0) {
		src.unsafe_set_buffer(buffer, buffer, buffer + buffer_size);
		s = buffer_size;
	}

	if (s < min_size) {
		usize cur_offs = to_write.end() - buffer;
		buffer = (u8 *)memrealloc(buffer, cur_offs + min_size, buffer_size);
		buffer_size = cur_offs + min_size;
		src.unsafe_set_buffer(buffer + cur_offs, buffer + cur_offs, buffer + buffer_size);
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