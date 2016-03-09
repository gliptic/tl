#include "../std.h"
//#include "../io.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

void panic() {
	ExitProcess(1);
}

static int has_freq = 0;
double inv_freq;
LARGE_INTEGER org_counter;

TL_API u64 tl_get_ticks() {
	if (!has_freq) {
		has_freq = 1;
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		inv_freq = 10000000.0 / freq.QuadPart;

		QueryPerformanceCounter(&org_counter);
	}

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	return (u64)((counter.QuadPart - org_counter.QuadPart) * inv_freq);
}
