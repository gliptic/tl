#ifndef TL_WINDOWS_MINIWINDOWS_H
#define TL_WINDOWS_MINIWINDOWS_H 1

#include "../cstdint.h"
#include "../platform.h"

TL_BEGIN_C

#define _WIN32_WINNT 0x0410
#define WINVER _WIN32_WINNT

// TODO: DECLSPEC_IMPORT and DECLSPEC_NORETURN belong in platfrom.h

#if (defined(_M_IX86) || defined(_M_IA64) || defined(_M_AMD64) || defined(_M_ARM)) && !defined(MIDL_PASS)
#define DECLSPEC_IMPORT __declspec(dllimport)
#else
#define DECLSPEC_IMPORT
#endif

#ifndef DECLSPEC_NORETURN
#if (_MSC_VER >= 1200) && !defined(MIDL_PASS)
#define DECLSPEC_NORETURN   __declspec(noreturn)
#else
#define DECLSPEC_NORETURN
#endif
#endif

#define WINBASEAPI DECLSPEC_IMPORT
#define WINUSERAPI DECLSPEC_IMPORT
#define WINGDIAPI DECLSPEC_IMPORT

#define WINAPIV     TL_CCALL
#define CALLBACK    TL_STDCALL
#define PASCAL      TL_STDCALL
#define TL_WINAPI   TL_STDCALL
#define WINAPI      TL_WINAPI

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name

namespace tl {
namespace win {

typedef uintptr_t UINT_PTR;
typedef UINT_PTR SOCKET;
typedef unsigned char BYTE, *PBYTE, *LPBYTE;
typedef u8 BOOLEAN;
typedef BOOLEAN BOOL;
typedef char CHAR;
typedef u16 WORD;
typedef int *LPINT;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned int UINT, *PUINT;
typedef float FLOAT;
typedef u32 DWORD, *LPDWORD;
typedef void VOID;

typedef CHAR *NPSTR, *LPSTR, *PSTR;

typedef void *PVOID, *LPVOID;
typedef const void *LPCVOID;
typedef intptr_t LONG_PTR, *PLONG_PTR;
typedef uintptr_t ULONG_PTR, *PULONG_PTR;
typedef intptr_t INT_PTR, *PINT_PTR;
typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;

typedef void *HDC;
typedef void *HANDLE;
typedef void *HWND;
DECLARE_HANDLE(HRAWINPUT);
DECLARE_HANDLE(HCURSOR);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HICON);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HGLRC);          // OpenGL
typedef HINSTANCE HMODULE;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef WORD ATOM;

#ifdef _WIN64
typedef INT_PTR(WINAPI *FARPROC)();
typedef INT_PTR(WINAPI *NEARPROC)();
typedef INT_PTR(WINAPI *PROC)();
#else
typedef int (WINAPI *FARPROC)();
typedef int (WINAPI *NEARPROC)();
typedef int (WINAPI *PROC)();
#endif  // _WIN64

inline u16 LOWORD(u32 v) { return (u16)(v & 0xffff); }
inline u16 MAKEWORD(u8 a, u8 b) { return ((u16)a << 8) | b; }


// Windows.h
typedef union _LARGE_INTEGER {
	/*
	struct {
		DWORD LowPart;
		LONG HighPart;
	} DUMMYSTRUCTNAME;
	*/
	struct {
		DWORD LowPart;
		LONG HighPart;
	} u;

	LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
	/*
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} DUMMYSTRUCTNAME;
	*/
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} u;

	ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

WINBASEAPI DECLSPEC_NORETURN VOID WINAPI ExitProcess(UINT uExitCode);

WINBASEAPI BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER * lpPerformanceCount);
WINBASEAPI BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER * lpFrequency);
WINBASEAPI BOOL WINAPI CloseHandle(HANDLE hObject);

typedef const char *LPCSTR, *PCSTR;

typedef struct _SECURITY_ATTRIBUTES {
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _MEMORY_BASIC_INFORMATION {
	PVOID BaseAddress;
	PVOID AllocationBase;
	DWORD AllocationProtect;
	SIZE_T RegionSize;
	DWORD State;
	DWORD Protect;
	DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

#define DUMMYSTRUCTNAME  s
#define DUMMYSTRUCTNAME2 s2
#define DUMMYSTRUCTNAME3 s3
#define DUMMYSTRUCTNAME4 s4
#define DUMMYSTRUCTNAME5 s5
#define DUMMYSTRUCTNAME6 s6
#define DUMMYUNIONNAME   u
#define DUMMYUNIONNAME2  u2
#define DUMMYUNIONNAME3  u3
#define DUMMYUNIONNAME4  u4
#define DUMMYUNIONNAME5  u5
#define DUMMYUNIONNAME6  u6
#define DUMMYUNIONNAME7  u7
#define DUMMYUNIONNAME8  u8
#define DUMMYUNIONNAME9  u9

typedef struct _OVERLAPPED {
	ULONG_PTR Internal;
	ULONG_PTR InternalHigh;
	union {
		struct {
			DWORD Offset;
			DWORD OffsetHigh;
		} DUMMYSTRUCTNAME;
		PVOID Pointer;
	} DUMMYUNIONNAME;

	HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED, *LPWSAOVERLAPPED;

typedef
void
(CALLBACK * LPWSAOVERLAPPED_COMPLETION_ROUTINE)(
	DWORD dwError,
	DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped,
	DWORD dwFlags);

typedef struct tagRAWMOUSE {
	/*
	* Indicator flags.
	*/
	USHORT usFlags;

	/*
	* The transition state of the mouse buttons.
	*/
	union {
		ULONG ulButtons;
		struct {
			USHORT  usButtonFlags;
			USHORT  usButtonData;
		};
	};


	/*
	* The raw state of the mouse buttons.
	*/
	ULONG ulRawButtons;

	/*
	* The signed relative or absolute motion in the X direction.
	*/
	LONG lLastX;

	/*
	* The signed relative or absolute motion in the Y direction.
	*/
	LONG lLastY;

	/*
	* Device-specific additional information for the event.
	*/
	ULONG ulExtraInformation;

} RAWMOUSE, *PRAWMOUSE, *LPRAWMOUSE;

typedef struct tagRAWKEYBOARD {
	/*
	* The "make" scan code (key depression).
	*/
	USHORT MakeCode;

	/*
	* The flags field indicates a "break" (key release) and other
	* miscellaneous scan code information defined in ntddkbd.h.
	*/
	USHORT Flags;

	USHORT Reserved;

	/*
	* Windows message compatible information
	*/
	USHORT VKey;
	UINT   Message;

	/*
	* Device-specific additional information for the event.
	*/
	ULONG ExtraInformation;


} RAWKEYBOARD, *PRAWKEYBOARD, *LPRAWKEYBOARD;

typedef struct tagRAWINPUTHEADER {
	DWORD dwType;
	DWORD dwSize;
	HANDLE hDevice;
	WPARAM wParam;
} RAWINPUTHEADER, *PRAWINPUTHEADER, *LPRAWINPUTHEADER;

typedef struct tagRAWHID {
	DWORD dwSizeHid;    // byte size of each report
	DWORD dwCount;      // number of input packed
	BYTE bRawData[1];
} RAWHID, *PRAWHID, *LPRAWHID;

typedef struct tagRAWINPUTDEVICE {
	USHORT usUsagePage; // Toplevel collection UsagePage
	USHORT usUsage;     // Toplevel collection Usage
	DWORD dwFlags;
	HWND hwndTarget;    // Target hwnd. NULL = follows keyboard focus
} RAWINPUTDEVICE, *PRAWINPUTDEVICE, *LPRAWINPUTDEVICE;

typedef const RAWINPUTDEVICE* PCRAWINPUTDEVICE;

typedef struct tagRAWINPUT {
	RAWINPUTHEADER header;
	union {
		RAWMOUSE    mouse;
		RAWKEYBOARD keyboard;
		RAWHID      hid;
	} data;
} RAWINPUT, *PRAWINPUT, *LPRAWINPUT;

typedef struct _OVERLAPPED_ENTRY {
	ULONG_PTR lpCompletionKey;
	LPOVERLAPPED lpOverlapped;
	ULONG_PTR Internal;
	DWORD dwNumberOfBytesTransferred;
} OVERLAPPED_ENTRY, *LPOVERLAPPED_ENTRY;

typedef DWORD(WINAPI *PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

enum : DWORD {
	GENERIC_READ    = 0x80000000L,
	GENERIC_WRITE   = 0x40000000L,
	GENERIC_EXECUTE = 0x20000000L,
	GENERIC_ALL     = 0x10000000L,

	FILE_SHARE_READ   = 0x00000001,  
	FILE_SHARE_WRITE  = 0x00000002,  
	FILE_SHARE_DELETE = 0x00000004,  
	FILE_ATTRIBUTE_READONLY            = 0x00000001,
	FILE_ATTRIBUTE_HIDDEN              = 0x00000002,
	FILE_ATTRIBUTE_SYSTEM              = 0x00000004,
	FILE_ATTRIBUTE_DIRECTORY           = 0x00000010,
	FILE_ATTRIBUTE_ARCHIVE             = 0x00000020,
	FILE_ATTRIBUTE_DEVICE              = 0x00000040,
	FILE_ATTRIBUTE_NORMAL              = 0x00000080,
	FILE_ATTRIBUTE_TEMPORARY           = 0x00000100,
	FILE_ATTRIBUTE_SPARSE_FILE         = 0x00000200,
	FILE_ATTRIBUTE_REPARSE_POINT       = 0x00000400,
	FILE_ATTRIBUTE_COMPRESSED          = 0x00000800,
	FILE_ATTRIBUTE_OFFLINE             = 0x00001000,
	FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000,
	FILE_ATTRIBUTE_ENCRYPTED           = 0x00004000,
	FILE_ATTRIBUTE_INTEGRITY_STREAM    = 0x00008000,
	FILE_ATTRIBUTE_VIRTUAL             = 0x00010000,
	FILE_ATTRIBUTE_NO_SCRUB_DATA       = 0x00020000,
	FILE_ATTRIBUTE_EA                  = 0x00040000,
	FILE_NOTIFY_CHANGE_FILE_NAME   = 0x00000001,
	FILE_NOTIFY_CHANGE_DIR_NAME    = 0x00000002,
	FILE_NOTIFY_CHANGE_ATTRIBUTES  = 0x00000004,
	FILE_NOTIFY_CHANGE_SIZE        = 0x00000008,
	FILE_NOTIFY_CHANGE_LAST_WRITE  = 0x00000010,
	FILE_NOTIFY_CHANGE_LAST_ACCESS = 0x00000020,
	FILE_NOTIFY_CHANGE_CREATION    = 0x00000040,
	FILE_NOTIFY_CHANGE_SECURITY    = 0x00000100,
	FILE_ACTION_ADDED            = 0x00000001,
	FILE_ACTION_REMOVED          = 0x00000002,
	FILE_ACTION_MODIFIED         = 0x00000003,
	FILE_ACTION_RENAMED_OLD_NAME = 0x00000004,
	FILE_ACTION_RENAMED_NEW_NAME = 0x00000005,
	MAILSLOT_NO_MESSAGE   = ((DWORD)-1),
	MAILSLOT_WAIT_FOREVER = ((DWORD)-1),
	FILE_CASE_SENSITIVE_SEARCH         = 0x00000001,
	FILE_CASE_PRESERVED_NAMES          = 0x00000002,
	FILE_UNICODE_ON_DISK               = 0x00000004,
	FILE_PERSISTENT_ACLS               = 0x00000008,
	FILE_FILE_COMPRESSION              = 0x00000010,
	FILE_VOLUME_QUOTAS                 = 0x00000020,
	FILE_SUPPORTS_SPARSE_FILES         = 0x00000040,
	FILE_SUPPORTS_REPARSE_POINTS       = 0x00000080,
	FILE_SUPPORTS_REMOTE_STORAGE       = 0x00000100,
	FILE_VOLUME_IS_COMPRESSED          = 0x00008000,
	FILE_SUPPORTS_OBJECT_IDS           = 0x00010000,
	FILE_SUPPORTS_ENCRYPTION           = 0x00020000,
	FILE_NAMED_STREAMS                 = 0x00040000,
	FILE_READ_ONLY_VOLUME              = 0x00080000,
	FILE_SEQUENTIAL_WRITE_ONCE         = 0x00100000,
	FILE_SUPPORTS_TRANSACTIONS         = 0x00200000,
	FILE_SUPPORTS_HARD_LINKS           = 0x00400000,
	FILE_SUPPORTS_EXTENDED_ATTRIBUTES  = 0x00800000,
	FILE_SUPPORTS_OPEN_BY_FILE_ID      = 0x01000000,
	FILE_SUPPORTS_USN_JOURNAL          = 0x02000000,
	FILE_SUPPORTS_INTEGRITY_STREAMS    = 0x04000000,

	CREATE_NEW        = 1,
	CREATE_ALWAYS     = 2,
	OPEN_EXISTING     = 3,
	OPEN_ALWAYS       = 4,
	TRUNCATE_EXISTING = 5,

	STANDARD_RIGHTS_REQUIRED = 0x000F0000L,
	SECTION_QUERY                = 0x0001,
	SECTION_MAP_WRITE            = 0x0002,
	SECTION_MAP_READ             = 0x0004,
	SECTION_MAP_EXECUTE          = 0x0008,
	SECTION_EXTEND_SIZE          = 0x0010,
	SECTION_MAP_EXECUTE_EXPLICIT = 0x0020, // not included in SECTION_ALL_ACCESS

	
	SECTION_ALL_ACCESS = (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|
                            SECTION_MAP_WRITE |
                            SECTION_MAP_READ |
                            SECTION_MAP_EXECUTE |
                            SECTION_EXTEND_SIZE),

	FILE_MAP_WRITE = SECTION_MAP_WRITE,
	FILE_MAP_READ = SECTION_MAP_READ,
	FILE_MAP_ALL_ACCESS = SECTION_ALL_ACCESS,

	PAGE_NOACCESS          = 0x01,     
	PAGE_READONLY          = 0x02,     
	PAGE_READWRITE         = 0x04,     
	PAGE_WRITECOPY         = 0x08,     
	PAGE_EXECUTE           = 0x10,     
	PAGE_EXECUTE_READ      = 0x20,     
	PAGE_EXECUTE_READWRITE = 0x40,     
	PAGE_EXECUTE_WRITECOPY = 0x80,     
	PAGE_GUARD             = 0x100,     
	PAGE_NOCACHE           = 0x200,     
	PAGE_WRITECOMBINE      = 0x400,     
	PAGE_REVERT_TO_FILE_MAP = 0x80000000,

	STD_INPUT_HANDLE  = ((DWORD)-10),
	STD_OUTPUT_HANDLE = ((DWORD)-11),
	STD_ERROR_HANDLE  = ((DWORD)-12),
	INVALID_FILE_ATTRIBUTES = ((DWORD)-1),

	STATUS_WAIT_0 = (DWORD)0x00000000L,
	WAIT_OBJECT_0 = STATUS_WAIT_0 + 0,
	INFINITE = 0xFFFFFFFF,  // Infinite timeout

	CREATE_SUSPENDED = 0x00000004,
};

//FILE_INVALID_FILE_ID               ((LONGLONG)-1LL) 

WINBASEAPI
DWORD
WINAPI
GetModuleFileNameA(
	HMODULE hModule,
	LPSTR lpFilename,
	DWORD nSize
);

WINBASEAPI
HANDLE
WINAPI
GetStdHandle(
	DWORD nStdHandle
);

WINBASEAPI
BOOL
WINAPI
WriteConsoleA(
	HANDLE hConsoleOutput,
	const VOID * lpBuffer,
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved
);

WINUSERAPI
int
WINAPIV
wsprintfA(
	LPSTR,
	LPCSTR,
	...);

WINBASEAPI
HANDLE
WINAPI
CreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
);

WINBASEAPI
BOOL
WINAPI
ReadFile(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToRead,
	LPDWORD lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
);

WINBASEAPI
BOOL
WINAPI
WriteFile(
	HANDLE hFile,
	LPCVOID lpBuffer,
	DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
);

WINBASEAPI
DWORD
WINAPI
GetFileAttributesA(
	LPCSTR lpFileName
);

WINBASEAPI
HANDLE
WINAPI
CreateFileMappingA(
	HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	DWORD flProtect,
	DWORD dwMaximumSizeHigh,
	DWORD dwMaximumSizeLow,
	LPCSTR lpName
);

WINBASEAPI
LPVOID
WINAPI
MapViewOfFile(
	HANDLE hFileMappingObject,
	DWORD dwDesiredAccess,
	DWORD dwFileOffsetHigh,
	DWORD dwFileOffsetLow,
	SIZE_T dwNumberOfBytesToMap
);

WINBASEAPI
BOOL
WINAPI
GetQueuedCompletionStatusEx(
	HANDLE CompletionPort,
	LPOVERLAPPED_ENTRY lpCompletionPortEntries,
	ULONG ulCount,
	ULONG* ulNumEntriesRemoved,
	DWORD dwMilliseconds,
	BOOL fAlertable
);

WINBASEAPI
BOOL
WINAPI
PostQueuedCompletionStatus(
	HANDLE CompletionPort,
	DWORD dwNumberOfBytesTransferred,
	ULONG_PTR dwCompletionKey,
	LPOVERLAPPED lpOverlapped
);

WINBASEAPI
HANDLE
WINAPI
CreateIoCompletionPort(
	HANDLE FileHandle,
	HANDLE ExistingCompletionPort,
	ULONG_PTR CompletionKey,
	DWORD NumberOfConcurrentThreads
);

WINBASEAPI
HANDLE
WINAPI
CreateThread(
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	SIZE_T dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags,
	LPDWORD lpThreadId
);

WINBASEAPI
DWORD
WINAPI
SuspendThread(
	HANDLE hThread
);

WINBASEAPI
DWORD
WINAPI
ResumeThread(
	HANDLE hThread
);

WINBASEAPI
DWORD
WINAPI
WaitForSingleObject(
	HANDLE hHandle,
	DWORD dwMilliseconds
);

WINBASEAPI SIZE_T WINAPI VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength);
WINBASEAPI BOOL WINAPI UnmapViewOfFile(LPCVOID lpBaseAddress);
WINBASEAPI DWORD WINAPI GetLastError(VOID);
WINBASEAPI VOID WINAPI SetLastError(DWORD dwErrCode);
WINBASEAPI VOID WINAPI Sleep(DWORD dwMilliseconds);
WINBASEAPI HMODULE WINAPI GetModuleHandleA(LPCSTR lpModuleName);

// GDI
enum {
	CCHDEVICENAME = 32,
	CCHFORMNAME = 32,

	DM_COPIES           = 0x00000100L,
	DM_DEFAULTSOURCE    = 0x00000200L,
	DM_PRINTQUALITY     = 0x00000400L,
	DM_COLOR            = 0x00000800L,
	DM_DUPLEX           = 0x00001000L,
	DM_YRESOLUTION      = 0x00002000L,
	DM_TTOPTION         = 0x00004000L,
	DM_COLLATE          = 0x00008000L,
	DM_FORMNAME         = 0x00010000L,
	DM_LOGPIXELS        = 0x00020000L,
	DM_BITSPERPEL       = 0x00040000L,
	DM_PELSWIDTH        = 0x00080000L,
	DM_PELSHEIGHT       = 0x00100000L,
	DM_DISPLAYFLAGS     = 0x00200000L,
	DM_DISPLAYFREQUENCY = 0x00400000L,
#if(WINVER >= 0x0400) // TODO: WINVER
	DM_ICMMETHOD     = 0x00800000L,
	DM_ICMINTENT     = 0x01000000L,
	DM_MEDIATYPE     = 0x02000000L,
	DM_DITHERTYPE    = 0x04000000L,
	DM_PANNINGWIDTH  = 0x08000000L,
	DM_PANNINGHEIGHT = 0x10000000L,
#endif /* WINVER >= 0x0400 */

	CDS_UPDATEREGISTRY  = 0x00000001,
	CDS_TEST            = 0x00000002,
	CDS_FULLSCREEN      = 0x00000004,
	CDS_GLOBAL          = 0x00000008,
	CDS_SET_PRIMARY     = 0x00000010,
	CDS_VIDEOPARAMETERS = 0x00000020,
#if(WINVER >= 0x0600)
	CDS_ENABLE_UNSAFE_MODES  = 0x00000100,
	CDS_DISABLE_UNSAFE_MODES = 0x00000200,
#endif /* WINVER >= 0x0600 */
	CDS_RESET    = 0x40000000,
	CDS_RESET_EX = 0x20000000,
	CDS_NORESET  = 0x10000000,

	RID_INPUT  = 0x10000003,
	RID_HEADER = 0x10000005,

	HTERROR       = (-2),
	HTTRANSPARENT = (-1),
	HTNOWHERE     = 0,
	HTCLIENT      = 1,
	HTCAPTION     = 2,
	HTSYSMENU     = 3,
	HTGROWBOX     = 4,
	HTSIZE        = HTGROWBOX,
	HTMENU        = 5,
	HTHSCROLL     = 6,
	HTVSCROLL     = 7,
	HTMINBUTTON   = 8,
	HTMAXBUTTON   = 9,
	HTLEFT        = 10,
	HTRIGHT       = 11,
	HTTOP         = 12,
	HTTOPLEFT     = 13,
	HTTOPRIGHT    = 14,
	HTBOTTOM      = 15,
	HTBOTTOMLEFT  = 16,
	HTBOTTOMRIGHT = 17,
	HTBORDER      = 18,
	HTREDUCE      = HTMINBUTTON,
	HTZOOM        = HTMAXBUTTON,
	HTSIZEFIRST   = HTLEFT,
	HTSIZELAST    = HTBOTTOMRIGHT,
#if(WINVER >= 0x0400)
	HTOBJECT = 19,
	HTCLOSE  = 20,
	HTHELP   = 21,
#endif /* WINVER >= 0x0400 */

	WS_OVERLAPPED    = 0x00000000L,
	WS_POPUP         = 0x80000000L,
	WS_CHILD         = 0x40000000L,
	WS_MINIMIZE      = 0x20000000L,
	WS_VISIBLE       = 0x10000000L,
	WS_DISABLED      = 0x08000000L,
	WS_CLIPSIBLINGS  = 0x04000000L,
	WS_CLIPCHILDREN  = 0x02000000L,
	WS_MAXIMIZE      = 0x01000000L,
	WS_CAPTION       = 0x00C00000L,     /* WS_BORDER | WS_DLGFRAME  */
	WS_BORDER        = 0x00800000L,
	WS_DLGFRAME      = 0x00400000L,
	WS_VSCROLL       = 0x00200000L,
	WS_HSCROLL       = 0x00100000L,
	WS_SYSMENU       = 0x00080000L,
	WS_THICKFRAME    = 0x00040000L,
	WS_GROUP         = 0x00020000L,
	WS_TABSTOP       = 0x00010000L,

	WS_MINIMIZEBOX = 0x00020000L,
	WS_MAXIMIZEBOX = 0x00010000L,


	WS_TILED        = WS_OVERLAPPED,
	WS_ICONIC       = WS_MINIMIZE,
	WS_SIZEBOX      = WS_THICKFRAME,

	/*
	* Common Window Styles
	*/
	WS_OVERLAPPEDWINDOW = (WS_OVERLAPPED     |
                             WS_CAPTION        |
                             WS_SYSMENU        |
                             WS_THICKFRAME     |
                             WS_MINIMIZEBOX    |
                             WS_MAXIMIZEBOX),
	WS_TILEDWINDOW = WS_OVERLAPPEDWINDOW,

	WS_POPUPWINDOW = (WS_POPUP          |
                             WS_BORDER         |
                             WS_SYSMENU),

	WS_CHILDWINDOW = (WS_CHILD),

	/*
	* Extended Window Styles
	*/
	WS_EX_DLGMODALFRAME  = 0x00000001L,
	WS_EX_NOPARENTNOTIFY = 0x00000004L,
	WS_EX_TOPMOST        = 0x00000008L,
	WS_EX_ACCEPTFILES    = 0x00000010L,
	WS_EX_TRANSPARENT    = 0x00000020L,
#if(WINVER >= 0x0400)
	WS_EX_MDICHILD    = 0x00000040L,
	WS_EX_TOOLWINDOW  = 0x00000080L,
	WS_EX_WINDOWEDGE  = 0x00000100L,
	WS_EX_CLIENTEDGE  = 0x00000200L,
	WS_EX_CONTEXTHELP = 0x00000400L,

#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0400)

	WS_EX_RIGHT          = 0x00001000L,
	WS_EX_LEFT           = 0x00000000L,
	WS_EX_RTLREADING     = 0x00002000L,
	WS_EX_LTRREADING     = 0x00000000L,
	WS_EX_LEFTSCROLLBAR  = 0x00004000L,
	WS_EX_RIGHTSCROLLBAR = 0x00000000L,

	WS_EX_CONTROLPARENT = 0x00010000L,
	WS_EX_STATICEDGE    = 0x00020000L,
	WS_EX_APPWINDOW     = 0x00040000L,


	WS_EX_OVERLAPPEDWINDOW = (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE),
	WS_EX_PALETTEWINDOW    = (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST),

#endif /* WINVER >= 0x0400 */

#if(_WIN32_WINNT >= 0x0500)
	WS_EX_LAYERED = 0x00080000,

#endif /* _WIN32_WINNT >= 0x0500 */


#if(WINVER >= 0x0500)
	WS_EX_NOINHERITLAYOUT = 0x00100000L, // Disable inheritence of mirroring by children
#endif /* WINVER >= 0x0500 */

#if(WINVER >= 0x0602)
	WS_EX_NOREDIRECTIONBITMAP = 0x00200000L,
#endif /* WINVER >= 0x0602 */

#if(WINVER >= 0x0500)
	WS_EX_LAYOUTRTL = 0x00400000L, // Right to left mirroring
#endif /* WINVER >= 0x0500 */

#if(_WIN32_WINNT >= 0x0501)
	WS_EX_COMPOSITED = 0x02000000L,
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0500)
	WS_EX_NOACTIVATE = 0x08000000L,
#endif /* _WIN32_WINNT >= 0x0500 */

	PFD_DOUBLEBUFFER         = 0x00000001,
	PFD_STEREO               = 0x00000002,
	PFD_DRAW_TO_WINDOW       = 0x00000004,
	PFD_DRAW_TO_BITMAP       = 0x00000008,
	PFD_SUPPORT_GDI          = 0x00000010,
	PFD_SUPPORT_OPENGL       = 0x00000020,
	PFD_GENERIC_FORMAT       = 0x00000040,
	PFD_NEED_PALETTE         = 0x00000080,
	PFD_NEED_SYSTEM_PALETTE  = 0x00000100,
	PFD_SWAP_EXCHANGE        = 0x00000200,
	PFD_SWAP_COPY            = 0x00000400,
	PFD_SWAP_LAYER_BUFFERS   = 0x00000800,
	PFD_GENERIC_ACCELERATED  = 0x00001000,
	PFD_SUPPORT_DIRECTDRAW   = 0x00002000,
	PFD_DIRECT3D_ACCELERATED = 0x00004000,
	PFD_SUPPORT_COMPOSITION  = 0x00008000,

		/* PIXELFORMATDESCRIPTOR flags for use in ChoosePixelFormat only */
	PFD_DEPTH_DONTCARE        = 0x20000000,
	PFD_DOUBLEBUFFER_DONTCARE = 0x40000000,
	PFD_STEREO_DONTCARE       = 0x80000000,

	ENUM_CURRENT_SETTINGS  = ((DWORD)-1),
	ENUM_REGISTRY_SETTINGS = ((DWORD)-2),
};

enum {
	DISP_CHANGE_SUCCESSFUL =  0,
	DISP_CHANGE_RESTART    =  1,
	DISP_CHANGE_FAILED     = -1,
	DISP_CHANGE_BADMODE    = -2,
	DISP_CHANGE_NOTUPDATED = -3,
	DISP_CHANGE_BADFLAGS   = -4,
	DISP_CHANGE_BADPARAM   = -5,
#if(_WIN32_WINNT >= 0x0501)
	DISP_CHANGE_BADDUALVIEW = -6,
#endif /* _WIN32_WINNT >= 0x0501 */

	GWLP_WNDPROC    = -4,
	GWLP_HINSTANCE  = -6,
	GWLP_HWNDPARENT = -8,
	GWLP_USERDATA   = -21,
	GWLP_ID         = -12,

	PFD_TYPE_RGBA = 0,
	PFD_TYPE_COLORINDEX = 1,
	PFD_MAIN_PLANE     = 0,
	PFD_OVERLAY_PLANE  = 1,
	PFD_UNDERLAY_PLANE = (-1),
};

enum /*: UINT*/ {
	WM_SETFOCUS = 0x0007,
	WM_KILLFOCUS = 0x0008,
	WM_SYSKEYDOWN = 0x0104,
	WM_SYSKEYUP = 0x0105,
	WM_SETCURSOR = 0x0020,
	WM_CLOSE = 0x0010,
	WM_INPUT = 0x00FF,
	WM_PAINT = 0x000F,

	CS_OWNDC = 0x0020,

	PM_NOREMOVE = 0x0000,
	PM_REMOVE   = 0x0001,
	PM_NOYIELD  = 0x0002,
};

enum : int {
	VK_F4 = 0x73,

	SM_SWAPBUTTON = 23,

	CW_USEDEFAULT = ((int)0x80000000),

	SW_HIDE            = 0,
	SW_SHOWNORMAL      = 1,
	SW_NORMAL          = 1,
	SW_SHOWMINIMIZED   = 2,
	SW_SHOWMAXIMIZED   = 3,
	SW_MAXIMIZE        = 3,
	SW_SHOWNOACTIVATE  = 4,
	SW_SHOW            = 5,
	SW_MINIMIZE        = 6,
	SW_SHOWMINNOACTIVE = 7,
	SW_SHOWNA          = 8,
	SW_RESTORE         = 9,
	SW_SHOWDEFAULT     = 10,
	SW_FORCEMINIMIZE   = 11,
	SW_MAX             = 11,

	TRUE = 1,
	FALSE = 0,
};

typedef struct _POINTL      /* ptl  */
{
	LONG  x;
	LONG  y;
} POINTL, *PPOINTL, POINT, *PPOINT, *NPPOINT, *LPPOINT;

typedef struct tagRECT
{
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

typedef struct _devicemodeA {
	BYTE   dmDeviceName[CCHDEVICENAME];
	WORD dmSpecVersion;
	WORD dmDriverVersion;
	WORD dmSize;
	WORD dmDriverExtra;
	DWORD dmFields;
	union {
		/* printer only fields */
		struct {
			short dmOrientation;
			short dmPaperSize;
			short dmPaperLength;
			short dmPaperWidth;
			short dmScale;
			short dmCopies;
			short dmDefaultSource;
			short dmPrintQuality;
		};
		/* display only fields */
		struct {
			POINTL dmPosition;
			DWORD  dmDisplayOrientation;
			DWORD  dmDisplayFixedOutput;
		};
	};
	short dmColor;
	short dmDuplex;
	short dmYResolution;
	short dmTTOption;
	short dmCollate;
	BYTE   dmFormName[CCHFORMNAME];
	WORD   dmLogPixels;
	DWORD  dmBitsPerPel;
	DWORD  dmPelsWidth;
	DWORD  dmPelsHeight;
	union {
		DWORD  dmDisplayFlags;
		DWORD  dmNup;
	};
	DWORD  dmDisplayFrequency;
#if(WINVER >= 0x0400)
	DWORD  dmICMMethod;
	DWORD  dmICMIntent;
	DWORD  dmMediaType;
	DWORD  dmDitherType;
	DWORD  dmReserved1;
	DWORD  dmReserved2;
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= _WIN32_WINNT_NT4)
	DWORD  dmPanningWidth;
	DWORD  dmPanningHeight;
#endif
#endif /* WINVER >= 0x0400 */
} DEVMODEA, *PDEVMODEA, *NPDEVMODEA, *LPDEVMODEA;

typedef struct tagPIXELFORMATDESCRIPTOR
{
	WORD  nSize;
	WORD  nVersion;
	DWORD dwFlags;
	BYTE  iPixelType;
	BYTE  cColorBits;
	BYTE  cRedBits;
	BYTE  cRedShift;
	BYTE  cGreenBits;
	BYTE  cGreenShift;
	BYTE  cBlueBits;
	BYTE  cBlueShift;
	BYTE  cAlphaBits;
	BYTE  cAlphaShift;
	BYTE  cAccumBits;
	BYTE  cAccumRedBits;
	BYTE  cAccumGreenBits;
	BYTE  cAccumBlueBits;
	BYTE  cAccumAlphaBits;
	BYTE  cDepthBits;
	BYTE  cStencilBits;
	BYTE  cAuxBuffers;
	BYTE  iLayerType;
	BYTE  bReserved;
	DWORD dwLayerMask;
	DWORD dwVisibleMask;
	DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR, *PPIXELFORMATDESCRIPTOR, *LPPIXELFORMATDESCRIPTOR;

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagWNDCLASSA {
	UINT        style;
	WNDPROC     lpfnWndProc;
	int         cbClsExtra;
	int         cbWndExtra;
	HINSTANCE   hInstance;
	HICON       hIcon;
	HCURSOR     hCursor;
	HBRUSH      hbrBackground;
	LPCSTR      lpszMenuName;
	LPCSTR      lpszClassName;
} WNDCLASSA, *PWNDCLASSA, *NPWNDCLASSA, *LPWNDCLASSA;

typedef struct tagMSG {
	HWND        hwnd;
	UINT        message;
	WPARAM      wParam;
	LPARAM      lParam;
	DWORD       time;
	POINT       pt;
#ifdef _MAC
	DWORD       lPrivate;
#endif
} MSG, *PMSG, *NPMSG, *LPMSG;

WINGDIAPI PROC  WINAPI wglGetProcAddress(LPCSTR);
WINGDIAPI HDC WINAPI wglGetCurrentDC(VOID);
WINUSERAPI int WINAPI ReleaseDC(HWND hWnd, HDC hDC);
WINUSERAPI BOOL WINAPI DestroyWindow(HWND hWnd);
WINUSERAPI int WINAPI GetSystemMetrics(int nIndex);

WINUSERAPI
BOOL
WINAPI
EnumDisplaySettingsA(
	LPCSTR lpszDeviceName,
	DWORD iModeNum,
	DEVMODEA* lpDevMode);

WINUSERAPI
LONG
WINAPI
ChangeDisplaySettingsA(
	DEVMODEA* lpDevMode,
	DWORD dwFlags);

WINUSERAPI LONG_PTR WINAPI GetWindowLongPtrA(HWND hWnd, int nIndex);
WINUSERAPI LONG_PTR WINAPI SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
WINUSERAPI BOOL WINAPI IsWindowVisible(HWND hWnd);
WINUSERAPI BOOL WINAPI CloseWindow(HWND hWnd);
WINUSERAPI BOOL WINAPI OpenIcon(HWND hWnd);
WINUSERAPI HICON WINAPI LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName);
WINUSERAPI ATOM WINAPI RegisterClassA(const WNDCLASSA *lpWndClass);
WINUSERAPI BOOL WINAPI GetCursorPos(LPPOINT lpPoint);
WINUSERAPI BOOL WINAPI ScreenToClient(HWND hWnd, LPPOINT lpPoint);
WINUSERAPI BOOL WINAPI GetKeyboardState(PBYTE lpKeyState);
WINUSERAPI HDC WINAPI GetDC(HWND hWnd);
WINUSERAPI HWND WINAPI GetDesktopWindow(VOID);
WINUSERAPI BOOL WINAPI GetClientRect(HWND hWnd, LPRECT lpRect);
WINUSERAPI BOOL WINAPI MoveWindow(
	HWND hWnd,
	int X,
	int Y,
	int nWidth,
	int nHeight,
	BOOL bRepaint);

WINUSERAPI BOOL WINAPI ShowWindow(HWND hWnd, int nCmdShow);

WINUSERAPI
BOOL
WINAPI
AdjustWindowRectEx(
	LPRECT lpRect,
	DWORD dwStyle,
	BOOL bMenu,
	DWORD dwExStyle);

WINUSERAPI
HWND
WINAPI
CreateWindowExA(
	DWORD dwExStyle,
	LPCSTR lpClassName,
	LPCSTR lpWindowName,
	DWORD dwStyle,
	int X,
	int Y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam);

WINUSERAPI
BOOL
WINAPI
PeekMessageA(
	LPMSG lpMsg,
	HWND hWnd,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax,
	UINT wRemoveMsg);

WINUSERAPI
BOOL
WINAPI
TranslateMessage(const MSG *lpMsg);

WINUSERAPI
LRESULT
WINAPI
DispatchMessageA(const MSG *lpMsg);

WINGDIAPI BOOL  WINAPI SetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR * ppfd);
WINGDIAPI BOOL WINAPI SwapBuffers(HDC);
WINUSERAPI BOOL WINAPI ValidateRect(HWND hWnd, const RECT *lpRect);

WINGDIAPI int   WINAPI ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
WINGDIAPI HGLRC WINAPI wglCreateContext(HDC);
WINGDIAPI BOOL  WINAPI wglDeleteContext(HGLRC);
WINGDIAPI BOOL  WINAPI wglMakeCurrent(HDC, HGLRC);

// TODO: Get rid of macro
#define MAKEINTRESOURCEA(i) ((tl::win::LPSTR)((tl::win::ULONG_PTR)((tl::win::WORD)(i))))

WINUSERAPI
LRESULT
WINAPI
DefWindowProcA(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam);

WINUSERAPI
UINT
WINAPI
GetRawInputData(
	HRAWINPUT hRawInput,
	UINT uiCommand,
	LPVOID pData,
	PUINT pcbSize,
	UINT cbSizeHeader);

WINUSERAPI
BOOL
WINAPI
RegisterRawInputDevices(
	PCRAWINPUTDEVICE pRawInputDevices,
	UINT uiNumDevices,
	UINT cbSize);

WINUSERAPI HCURSOR WINAPI SetCursor(HCURSOR hCursor);

// WinSock2.h
#ifndef WINSOCK_API_LINKAGE
#ifdef DECLSPEC_IMPORT
#define WINSOCK_API_LINKAGE DECLSPEC_IMPORT
#else
#define WINSOCK_API_LINKAGE
#endif
#endif

#define WSAAPI PASCAL

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

static HANDLE const INVALID_HANDLE_VALUE = ((HANDLE)(UINT_PTR)-1);

enum : int {
	WSADESCRIPTION_LEN = 256,
	WSASYS_STATUS_LEN = 128,
	TCP_NODELAY = 0x0001,
	SOCKET_ERROR = -1,

	SOL_SOCKET = 0xffff,          /* options for socket level */

	SO_SNDBUF   = 0x1001,          /* send buffer size */
	SO_RCVBUF   = 0x1002,          /* receive buffer size */
	SO_SNDLOWAT = 0x1003,          /* send low-water mark */
	SO_RCVLOWAT = 0x1004,          /* receive low-water mark */
	SO_SNDTIMEO = 0x1005,          /* send timeout */
	SO_RCVTIMEO = 0x1006,          /* receive timeout */
	SO_ERROR    = 0x1007,          /* get error status and clear */
	SO_TYPE     = 0x1008,          /* get socket type */

	SOCK_STREAM    = 1,               /* stream socket */
	SOCK_DGRAM     = 2,               /* datagram socket */
	SOCK_RAW       = 3,               /* raw-protocol interface */
	SOCK_RDM       = 4,               /* reliably-delivered message */
	SOCK_SEQPACKET = 5,               /* sequenced packet stream */

	AF_UNSPEC    = 0,               // unspecified
	AF_UNIX      = 1,               // local to host (pipes, portals)
	AF_INET      = 2,               // internetwork: UDP, TCP, etc.
	AF_IMPLINK   = 3,               // arpanet imp addresses
	AF_PUP       = 4,               // pup protocols: e.g. BSP
	AF_CHAOS     = 5,               // mit CHAOS protocols
	AF_NS        = 6,               // XEROX NS protocols
	AF_IPX       = AF_NS,           // IPX protocols: IPX, SPX, etc.
	AF_ISO       = 7,               // ISO protocols
	AF_OSI       = AF_ISO,          // OSI is ISO
	AF_ECMA      = 8,               // european computer manufacturers
	AF_DATAKIT   = 9,               // datakit protocols
	AF_CCITT     = 10,              // CCITT protocols, X.25 etc
	AF_SNA       = 11,              // IBM SNA
	AF_DECnet    = 12,              // DECnet
	AF_DLI       = 13,              // Direct data link interface
	AF_LAT       = 14,              // LAT
	AF_HYLINK    = 15,              // NSC Hyperchannel
	AF_APPLETALK = 16,              // AppleTalk
	AF_NETBIOS   = 17,              // NetBios-style addresses
	AF_VOICEVIEW = 18,              // VoiceView
	AF_FIREFOX   = 19,              // Protocols from Firefox
	AF_UNKNOWN1  = 20,              // Somebody is using this!
	AF_BAN       = 21,              // Banyan
	AF_ATM       = 22,              // Native ATM Services
	AF_INET6     = 23,              // Internetwork Version 6
	AF_CLUSTER   = 24,              // Microsoft Wolfpack
	AF_12844     = 25,              // IEEE 1284.4 WG AF
	AF_IRDA      = 26,              // IrDA
	AF_NETDES    = 28,              // Network Designers OSI & gateway

	PF_UNSPEC    = AF_UNSPEC,
	PF_UNIX      = AF_UNIX,
	PF_INET      = AF_INET,
	PF_IMPLINK   = AF_IMPLINK,
	PF_PUP       = AF_PUP,
	PF_CHAOS     = AF_CHAOS,
	PF_NS        = AF_NS,
	PF_IPX       = AF_IPX,
	PF_ISO       = AF_ISO,
	PF_OSI       = AF_OSI,
	PF_ECMA      = AF_ECMA,
	PF_DATAKIT   = AF_DATAKIT,
	PF_CCITT     = AF_CCITT,
	PF_SNA       = AF_SNA,
	PF_DECnet    = AF_DECnet,
	PF_DLI       = AF_DLI,
	PF_LAT       = AF_LAT,
	PF_HYLINK    = AF_HYLINK,
	PF_APPLETALK = AF_APPLETALK,
	PF_VOICEVIEW = AF_VOICEVIEW,
	PF_FIREFOX   = AF_FIREFOX,
	PF_UNKNOWN1  = AF_UNKNOWN1,
	PF_BAN       = AF_BAN,
	PF_ATM       = AF_ATM,
	PF_INET6     = AF_INET6,
	//PF_BTH       = AF_BTH
};

#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))
#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

enum : long {
	IOCPARM_MASK = (long)0x7f,            /* parameters must be < 128 bytes */
	IOC_VOID     = (long)0x20000000,      /* no parameters */
	IOC_OUT      = (long)0x40000000,      /* copy out parameters */
	IOC_IN       = (long)0x80000000,      /* copy in parameters */
	IOC_INOUT    = (long)(IOC_IN|IOC_OUT),
	/* 0x20000000 distinguishes new &
	old ioctl's */

	FIONREAD = (long)_IOR('f', 127, u_long), /* get # bytes to read */
	FIONBIO  = (long)_IOW('f', 126, u_long), /* set/clear non-blocking i/o */
	FIOASYNC = (long)_IOW('f', 125, u_long), /* set/clear async i/o */

	WSAEINPROGRESS = 10036L,
	WSAEWOULDBLOCK = 10035L,
	WSAECONNRESET = 10054L,
	ERROR_IO_PENDING = 997,
};

#undef _IO
#undef _IOR
#undef _IOW

enum : u_long {

	INADDR_ANY = (u_long)0x00000000,
	INADDR_LOOPBACK = 0x7f000001,
	INADDR_BROADCAST = (u_long)0xffffffff,
	INADDR_NONE = 0xffffffff
};

enum : SOCKET {
	INVALID_SOCKET = (SOCKET)(~0)
};

typedef struct in_addr {
	/*
	union {
		struct { u_char s_b1, s_b2, s_b3, s_b4; } S_un_b;
		struct { u_short s_w1, s_w2; } S_un_w;
		u_long S_addr;
	} S_un;
	*/
	union {
		struct { u_char s_net, s_host, s_lh, s_impno; };
		struct { u_short s_w1, s_imp; };
		u_long s_addr;
	};
//#define s_addr  S_un.S_addr /* can be used for most tcp & ip code */
//#define s_host  S_un.S_un_b.s_b2    // host on imp
//#define s_net   S_un.S_un_b.s_b1    // network
//#define s_imp   S_un.S_un_w.s_w2    // imp
//#define s_impno S_un.S_un_b.s_b4    // imp #
//#define s_lh    S_un.S_un_b.s_b3    // logical host
} IN_ADDR, *PIN_ADDR, *LPIN_ADDR;

typedef u_short ADDRESS_FAMILY;

typedef struct sockaddr {

	ADDRESS_FAMILY sa_family;           // Address family.

	CHAR sa_data[14];                   // Up to 14 bytes of direct address.
} SOCKADDR, *PSOCKADDR, *LPSOCKADDR;

typedef struct sockaddr_in {

	ADDRESS_FAMILY sin_family;

	USHORT sin_port;
	IN_ADDR sin_addr;
	CHAR sin_zero[8];
} SOCKADDR_IN, *PSOCKADDR_IN;

struct hostent {
	char    * h_name;           /* official name of host */
	char    * * h_aliases;  /* alias list */
	short   h_addrtype;             /* host address type */
	short   h_length;               /* length of address */
	char    * * h_addr_list; /* list of addresses */
// TODO: Replace macro with something else
//#define h_addr  h_addr_list[0]          /* address, for backward compat */
};

typedef enum {
#if(_WIN32_WINNT >= 0x0501)
	IPPROTO_HOPOPTS = 0,  // IPv6 Hop-by-Hop options
#endif//(_WIN32_WINNT >= 0x0501)
	IPPROTO_ICMP = 1,
	IPPROTO_IGMP = 2,
	IPPROTO_GGP = 3,
#if(_WIN32_WINNT >= 0x0501)
	IPPROTO_IPV4 = 4,
#endif//(_WIN32_WINNT >= 0x0501)
#if(_WIN32_WINNT >= 0x0600)
	IPPROTO_ST = 5,
#endif//(_WIN32_WINNT >= 0x0600)
	IPPROTO_TCP = 6,
#if(_WIN32_WINNT >= 0x0600)
	IPPROTO_CBT = 7,
	IPPROTO_EGP = 8,
	IPPROTO_IGP = 9,
#endif//(_WIN32_WINNT >= 0x0600)
	IPPROTO_PUP = 12,
	IPPROTO_UDP = 17,
	IPPROTO_IDP = 22,
#if(_WIN32_WINNT >= 0x0600)
	IPPROTO_RDP = 27,
#endif//(_WIN32_WINNT >= 0x0600)

#if(_WIN32_WINNT >= 0x0501)
	IPPROTO_IPV6 = 41, // IPv6 header
	IPPROTO_ROUTING = 43, // IPv6 Routing header
	IPPROTO_FRAGMENT = 44, // IPv6 fragmentation header
	IPPROTO_ESP = 50, // encapsulating security payload
	IPPROTO_AH = 51, // authentication header
	IPPROTO_ICMPV6 = 58, // ICMPv6
	IPPROTO_NONE = 59, // IPv6 no next header
	IPPROTO_DSTOPTS = 60, // IPv6 Destination options
#endif//(_WIN32_WINNT >= 0x0501)

	IPPROTO_ND = 77,
#if(_WIN32_WINNT >= 0x0501)
	IPPROTO_ICLFXBM = 78,
#endif//(_WIN32_WINNT >= 0x0501)
#if(_WIN32_WINNT >= 0x0600)
	IPPROTO_PIM = 103,
	IPPROTO_PGM = 113,
	IPPROTO_L2TP = 115,
	IPPROTO_SCTP = 132,
#endif//(_WIN32_WINNT >= 0x0600)
	IPPROTO_RAW = 255,

	IPPROTO_MAX = 256,
	//
	//  These are reserved for internal use by Windows.
	//
	IPPROTO_RESERVED_RAW = 257,
	IPPROTO_RESERVED_IPSEC = 258,
	IPPROTO_RESERVED_IPSECOFFLOAD = 259,
	IPPROTO_RESERVED_WNV = 260,
	IPPROTO_RESERVED_MAX = 261
} IPPROTO, *PIPROTO;

typedef struct WSAData {
	u16                    wVersion;
	u16                    wHighVersion;
#ifdef TL_WIN64
	u16                     iMaxSockets;
	u16                     iMaxUdpDg;
	char*                   lpVendorInfo;
	char                    szDescription[WSADESCRIPTION_LEN + 1];
	char                    szSystemStatus[WSASYS_STATUS_LEN + 1];
#else
	char                    szDescription[WSADESCRIPTION_LEN + 1];
	char                    szSystemStatus[WSASYS_STATUS_LEN + 1];
	u16                     iMaxSockets;
	u16                     iMaxUdpDg;
	char*                   lpVendorInfo;
#endif
} WSADATA, *LPWSADATA;

typedef struct _WSABUF {
	ULONG len;     /* the length of the buffer */
	CHAR *buf; /* the pointer to the buffer */
} WSABUF, *LPWSABUF;

WINSOCK_API_LINKAGE
int
WSAAPI
WSAStartup(
	WORD wVersionRequested,
	LPWSADATA lpWSAData
);

WINSOCK_API_LINKAGE
int
WSAAPI
WSAGetLastError(void);

WINSOCK_API_LINKAGE
SOCKET
WSAAPI
socket(
	int af,
	int type,
	int protocol);

WINSOCK_API_LINKAGE
int
WSAAPI
closesocket(SOCKET s);

int PASCAL shutdown(
	SOCKET s,
	int how);

WINSOCK_API_LINKAGE
int
WSAAPI
ioctlsocket(
	SOCKET s,
	long cmd,
	u_long * argp
);

WINSOCK_API_LINKAGE
int
WSAAPI
setsockopt(
	SOCKET s,
	int level,
	int optname,
	const char * optval,
	int optlen
);

WINSOCK_API_LINKAGE
int
WSAAPI
bind(
	SOCKET s,
	const struct sockaddr * name,
	int namelen
);

WINSOCK_API_LINKAGE
int
WSAAPI
listen(
	SOCKET s,
	int backlog
);

WINSOCK_API_LINKAGE
SOCKET
WSAAPI
accept(
	SOCKET s,
	struct sockaddr * addr,
	int * addrlen
);

WINSOCK_API_LINKAGE
int
WSAAPI
connect(
	SOCKET s,
	const struct sockaddr * name,
	int namelen
);

WINSOCK_API_LINKAGE
int
WSAAPI
recv(
	SOCKET s,
	char * buf,
	int len,
	int flags
);

WINSOCK_API_LINKAGE
int
WSAAPI
WSARecvFrom(
	SOCKET s,
	LPWSABUF lpBuffers,
	DWORD dwBufferCount,
	LPDWORD lpNumberOfBytesRecvd,
	LPDWORD lpFlags,
	struct sockaddr * lpFrom,
	LPINT lpFromlen,
	LPWSAOVERLAPPED lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

WINSOCK_API_LINKAGE
int
WSAAPI
WSARecv(
	SOCKET                             s,
	LPWSABUF                           lpBuffers,
	DWORD                              dwBufferCount,
	LPDWORD                            lpNumberOfBytesRecvd,
	LPDWORD                            lpFlags,
	LPWSAOVERLAPPED                    lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

WINSOCK_API_LINKAGE
int
WSAAPI
WSASend(
	SOCKET                             s,
	LPWSABUF                           lpBuffers,
	DWORD                              dwBufferCount,
	LPDWORD                            lpNumberOfBytesSent,
	DWORD                              dwFlags,
	LPWSAOVERLAPPED                    lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

WINSOCK_API_LINKAGE
int
WSAAPI
recvfrom(
	SOCKET s,
	char * buf,
	int len,
	int flags,
	struct sockaddr * from,
	int * fromlen
);

WINSOCK_API_LINKAGE
int
WSAAPI
send(
	SOCKET s,
	const char * buf,
	int len,
	int flags
);

WINSOCK_API_LINKAGE
int
WSAAPI
WSASendTo(
	SOCKET s,
	LPWSABUF lpBuffers,
	DWORD dwBufferCount,
	LPDWORD lpNumberOfBytesSent,
	DWORD dwFlags,
	const struct sockaddr * lpTo,
	int iTolen,
	LPWSAOVERLAPPED lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

WINSOCK_API_LINKAGE
int
WSAAPI
sendto(
	SOCKET s,
	const char * buf,
	int len,
	int flags,
	const struct sockaddr * to,
	int tolen
);

// Mswsock, windows 8!
BOOL
AcceptEx(
	SOCKET sListenSocket,
	SOCKET sAcceptSocket,
	PVOID lpOutputBuffer,
	DWORD dwReceiveDataLength,
	DWORD dwLocalAddressLength,
	DWORD dwRemoteAddressLength,
	LPDWORD lpdwBytesReceived,
	LPOVERLAPPED lpOverlapped
);

WINSOCK_API_LINKAGE
int
WSAAPI
getsockopt(
	SOCKET s,
	int level,
	int optname,
	char * optval,
	int * optlen
);

WINSOCK_API_LINKAGE
int
WSAAPI
getsockname(
	SOCKET s,
	struct sockaddr * name,
	int * namelen
);

WINSOCK_API_LINKAGE
struct hostent *
WSAAPI
gethostbyname(
	const char * name
);

// TODO: Replace with inline implemention?
WINSOCK_API_LINKAGE
u_short
WSAAPI
htons(
	u_short hostshort
);

WINSOCK_API_LINKAGE
u_short
WSAAPI
ntohs(
	u_short netshort
);

WINSOCK_API_LINKAGE
u_long
WSAAPI
htonl(
	u_long hostlong
);

WINSOCK_API_LINKAGE
u_long
WSAAPI
ntohl(
	u_long netlong
);

} // namespace win
} // namespace tl

TL_END_C

// Undefines
#undef WINSOCK_API_LINKAGE
#undef WSAAPI
#undef DECLSPEC_IMPORT
#undef DECLSPEC_NORETURN
#undef WINBASEAPI
#undef WINUSERAPI
#undef WINGDIAPI
#undef CALLBACK
#undef PASCAL
#undef WINAPI
#undef WINAPIV
#undef DECLARE_HANDLE

#endif // TL_WINDOWS_MINIWINDOWS_H

