#include "std.h"
#include "io.hpp"

extern "C" {

void assert_fail(char const* expr, char const* file, int line) {
	TL_UNUSED(file); TL_UNUSED(line);
	tl::sprint("Assert failure: !(");
	tl::sprint(expr);
	tl::sprint(")");
	panic();
}

}
