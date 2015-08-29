#include "../std.h"
#include "../io.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

void* memalloc(size_t s) {
	return GlobalAlloc(0, s);
}

void* memrealloc(void* p, size_t news, size_t olds) {
	void* newp = GlobalReAlloc(p, news, 0);
	if (newp) {
		return newp;
	}
	newp = memalloc(news);
	mcpy(newp, p, min(olds, news));
	GlobalFree(p);
	return newp;
}

void memfree(void* p) {
	if (p) GlobalFree(p);
}

void panic() {
	ExitProcess(1);
}