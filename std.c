#include "std.h"
#include "io.h"

void assert_fail(char const* expr, char const* file, int line) {
	sprint("Assert failure: !(");
	sprint(expr);
	sprint(")");
	panic();
}