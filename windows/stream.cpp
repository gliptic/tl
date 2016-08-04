#include "..\stream.hpp"
#include "win.hpp"

namespace tl {

struct file_pullable_win : file_pullable {
	file_pullable_win(HANDLE f)
		: f(f) {
	}

	~file_pullable_win() {
		if (f) CloseHandle(f);
	}

	int pull(source& src, usize min_size);

	HANDLE f;
};

file_pullable* file_pullable::open(char const* path) {
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h == INVALID_HANDLE_VALUE) {
		printf("%d\n", GetLastError());
	}
	return new file_pullable_win(h);
}

int file_pullable_win::pull(source& src, usize min_size) {
	usize space_left = pull_begin(src, min_size);

	DWORD c;
	BOOL r = ReadFile(this->f, (void *)src.end(), DWORD(space_left), &c, NULL);

	pull_done(src, c);

	if (!r) {
		printf("err: %d\n", GetLastError());
	}

	return c < min_size;
}

source source::from_file(char const* path) {
	return source(std::unique_ptr<pullable>(file_pullable::open(path)));
}

source source::from_file(StringSlice path) {
	String path_cstr(c_str(path));
	return source(std::unique_ptr<pullable>(file_pullable::open((char const *)path_cstr.begin())));
}

}