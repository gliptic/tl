#include "../std.h"
#include "../io.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

void panic() {
	ExitProcess(1);
}