#include "std.h"
#include "io.h"

void assert_fail(char const* expr, char const* file, int line) {
	TL_UNUSED(file); TL_UNUSED(line);
	sprint("Assert failure: !(");
	sprint(expr);
	sprint(")");
	panic();
}