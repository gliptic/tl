#include "../io.h"
#include "../std.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

void sprint(char const* s) {
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dummy;
	WriteConsole(
		h,
		s,
		(DWORD)strlen(s),
		&dummy,
		NULL);
}

static char* utos(uint32_t v, char* ends) {
	*--ends = 0;
	do {
		*--ends = '0' + (v % 10);
		v /= 10;
	} while (v);
	return ends;
}

void uprint(uint32_t v) {
	char buf[10];
	char *b = utos(v, buf + 10);
	sprint(b);
}

void iprint(int32_t v) {
	char buf[11], isneg = 0;

	if (v < 0) {
		v = -v;
		isneg = 1;
	}

	char *b = utos((uint32_t)v, buf + 11);
	if (isneg) *--b = '-';
	sprint(b);
}

void fprint(double v) {
	char buf[64];

	wsprintf(buf, "%f", v);
	sprint(buf);
}