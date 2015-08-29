#include "std.h"
#include "io.h"

void mcpy(uint8_t* dst, uint8_t const* src, size_t len) {
	while (len--) *dst++ = *src++;
}

void mset(uint8_t* dst, uint8_t v, size_t len) {
	while (len--) *dst++ = v;
}

size_t slen(char const* s) {
	char const* p = s;
	while (*p) ++p;
	return p - s;
}

void assert_fail(char const* expr, char const* file, int line) {
	sprint("Assert failure: !(");
	sprint(expr);
	sprint(")");
	panic();
}