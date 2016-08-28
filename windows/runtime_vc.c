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
typedef char _TSCHAR;

typedef struct {
	int newmode;
} _startupinfo;

#define _CRTALLOC(x) __declspec(allocate(x))
_CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { NULL };
_CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { NULL };

char _fltused;

//extern void main();
void __cdecl __getmainargs(int *, char ***, char ***, int, _startupinfo *);
__declspec(noreturn) void __cdecl exit(int _Code);
extern int main(int, char **, char **);

#define PC_53	0x200
#define PC_64	0x300

#define _IMP___FMODE    (__p__fmode())
#define _IMP___COMMODE  (__p__commode())
extern int * _IMP___FMODE;      /* exported from the CRT DLL */
extern int * _IMP___COMMODE;    /* these names are implementation-specific */
#define _fmode (0)
#define _commode (0)
#define _dowildcard (0)

void tl_std_init();

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
