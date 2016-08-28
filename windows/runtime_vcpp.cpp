#include <vcruntime.h>
#include <stdlib.h>
#include "../platform.h"

_VCRT_ALLOCATOR void* __CRTDECL operator new(size_t _Size) {
	return malloc(_Size);
}

_VCRT_ALLOCATOR void* __CRTDECL operator new[](size_t _Size) {
	return malloc(_Size);
}

void __CRTDECL operator delete(void* _Block) throw() {
	free(_Block);
}

#if TL_X86_64
void __CRTDECL operator delete(void* _Block, unsigned __int64) throw() {
	free(_Block);
}
#else
void __CRTDECL operator delete(void* _Block, unsigned int) throw() {
	free(_Block);
}
#endif

void __CRTDECL operator delete[](void* _Block) throw() {
	free(_Block);
}

extern "C" int __cdecl _purecall(void) { return 0; }
