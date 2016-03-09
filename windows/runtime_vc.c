#include "../platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Minimal runtime support
#pragma comment(linker, "/merge:.CRT=.rdata")
#pragma section(".CRT$XIA",long,read)
#pragma section(".CRT$XIZ",long,read)

typedef void(__cdecl *_PVFV)(void);
typedef int(__cdecl *_PIFV)(void);
typedef void(__cdecl *_PVFI)(int);

#define _CRTALLOC(x) __declspec(allocate(x))
_CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { NULL };
_CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { NULL };

char _fltused;

extern void main();

#define PC_53	0x200
#define PC_64	0x300

void __declspec(noinline) mainCRTStartup(void) {

	// TODO: Adapt to different versions of VC. This works for VC++ 10-14 at least.
	_PVFV *pfbegin = (_PVFV *)__xi_a, *pfend = (_PVFV *)__xi_z;

	for (; pfbegin < pfend; ++pfbegin) {
		if (*pfbegin != NULL)
			(**pfbegin)();
	}

#if TL_X86 && TL_MSVCPP
	unsigned short x;
	__asm { fnstcw x }
	x &= ~PC_64;
	x |= PC_53;
	__asm { fldcw x }
#endif

	main();
	ExitProcess(0);
}
