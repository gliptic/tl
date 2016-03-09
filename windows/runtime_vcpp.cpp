#include <vcruntime.h>
#include <stdlib.h>

_VCRT_ALLOCATOR void* __CRTDECL operator new(size_t _Size){
	return malloc(_Size);
}

_VCRT_ALLOCATOR void* __CRTDECL operator new[](size_t _Size) {
	return malloc(_Size);
}

void __CRTDECL operator delete(void* _Block) throw() {
	free(_Block);
}

void __CRTDECL operator delete(void* _Block, unsigned __int64) throw() {
	free(_Block);
}

void __CRTDECL operator delete[](void* _Block) throw() {
	free(_Block);
}