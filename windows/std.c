#include "../std.h"
#include "../bits.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define RtlGenRandom SystemFunction036
//#define DLLEXPORT __declspec(dllexport)
__declspec(dllexport) BOOLEAN WINAPI RtlGenRandom(PVOID in, ULONG len); // {}

void panic() {
#if !defined(NDEBUG) && TL_MSVCPP
	__debugbreak();
#endif
	ExitProcess(1);
}

static double inv_freq;
static LARGE_INTEGER org_counter;

TL_API void tl_std_init() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	inv_freq = 10000000.0 / freq.QuadPart;
	QueryPerformanceCounter(&org_counter);
}

TL_API u64 tl_get_ticks() {

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	// Converting to i64 is cheaper, and i64 is good for nearly 30000 years
	return (u64)(i64)((counter.QuadPart - org_counter.QuadPart) * inv_freq);
}

TL_API void tl_crypto_rand(void* data, u32 amount) {
	RtlGenRandom(data, amount);
}