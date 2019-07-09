#include "../platform.h"
#include "../std.h"
#include <stdlib.h>

#if TL_CPP
extern "C" {
#endif

#include "win.hpp"

// Minimal runtime support
#pragma comment(linker, "/merge:.CRT=.rdata")
#pragma section(".CRT$XIA",long,read)
#pragma section(".CRT$XIZ",long,read)
//
#pragma section(".CRT$XLA",long,read)
#pragma section(".CRT$XLZ",long,read)
#pragma section(".rdata$T",long,read)
#pragma section(".tls",long,read,write)
#pragma section(".tls$AAA",long,read,write)
#pragma section(".tls$ZZZ",long,read,write)

typedef void(__cdecl *_PVFV)(void);
typedef int(__cdecl *_PIFV)(void);
typedef void(__cdecl *_PVFI)(int);
typedef char _TSCHAR;

typedef struct {
	int newmode;
} _startupinfo;

#define TL_CRTALLOC(x) __declspec(allocate(x))
TL_CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { NULL };
TL_CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { NULL };

ULONG _tls_index = 0;
/* TLS raw template data start and end. */
TL_CRTALLOC(".tls") char _tls_start = 0;
TL_CRTALLOC(".tls$ZZZ") char _tls_end = 0;

TL_CRTALLOC(".CRT$XLA") PIMAGE_TLS_CALLBACK __xl_a = 0;
TL_CRTALLOC(".CRT$XLZ") PIMAGE_TLS_CALLBACK __xl_z = 0;

TL_CRTALLOC(".rdata$T")
const IMAGE_TLS_DIRECTORY _tls_used =
{
	(ULONG)(ULONG_PTR)&_tls_start, // start of tls data
	(ULONG)(ULONG_PTR)&_tls_end,   // end of tls data
	(ULONG)(ULONG_PTR)&_tls_index, // address of tls_index
	(ULONG)(ULONG_PTR)(&__xl_a + 1), // pointer to callbacks
	(ULONG)0,                      // size of tls zero fill
	(ULONG)0                       // characteristics
};

char _fltused;

//extern void main();
void __cdecl __getmainargs(int *, char ***, char ***, int, _startupinfo *);
//__declspec(noreturn) void __cdecl exit(int _Code);
extern int main(int, char **, char **);

#define PC_53	0x200
#define PC_64	0x300

#if 0
#define _IMP___FMODE    (__p__fmode())
#define _IMP___COMMODE  (__p__commode())
extern int * _IMP___FMODE;      /* exported from the CRT DLL */
extern int * _IMP___COMMODE;    /* these names are implementation-specific */
#endif

#ifndef _fmode
# define _fmode (0)
#endif
#ifndef _commode
# define _commode (0)
#endif
#ifndef _dowildcard
# define _dowildcard (0)
#endif

void __declspec(noinline) mainCRTStartup(void) {

	//*_IMP___FMODE = _fmode;
	//*_IMP___COMMODE = _commode;

	// TODO: Adapt to different versions of VC. This works for VC++ 10-15 at least.
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

	_startupinfo startinfo;
	int argc;
	_TSCHAR **argv;
	_TSCHAR **envp;
	__getmainargs(&argc, &argv, &envp, _dowildcard, &startinfo);

	tl_std_init();

	int r = main(argc, argv, envp);

	ExitProcess((UINT)r);
}

#if TL_CPP
} // extern "C"
#endif
