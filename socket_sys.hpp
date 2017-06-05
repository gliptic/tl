#ifndef UUID_338826A0A88B40CBD1DE1AA4A2F6B6CD
#define UUID_338826A0A88B40CBD1DE1AA4A2F6B6CD

#include "socket.hpp"

#if TL_WINDOWS

/*
#undef  NOGDI
#define NOGDI
#undef  NOMINMAX
#define NOMINMAX
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#undef  NONAMELESSUNION
#define NONAMELESSUNION
#undef  NOKERNEL
#define NOKERNEL
#undef  NONLS
#define NONLS

#ifndef POINTER_64
#define POINTER_64 // Needed for bugged headers
#endif

#if TL_WIN32
#define _WIN32_WINDOWS 0x0410
#endif

#define WINVER 0x0410

#include <winsock2.h>
//#include "windows/miniwindows.h"
*/
#include <stdlib.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

//#define tl_native_socket(s) ((SOCKET)(s)._voidp)
//#define native_socket(sock) ((sock).s)

namespace tl {

static int tl_sockets_initialized = 0;

static void tl_init_sockets() {
	if (!tl_sockets_initialized) {
		win::WSADATA wsaData;
		int res;

		tl_sockets_initialized = 1;

		res = WSAStartup(win::MAKEWORD(2,2), &wsaData);
		if (res != 0)
			return;
	}
}

TL_INLINE Socket make_socket(win::SOCKET native) {
	return Socket(native);
}

typedef int socklen_t;

/*
typedef win::sockaddr_in sockaddr_in;
typedef win::sockaddr sockaddr;
typedef win::u_short u_short;
typedef win::hostent hostent;
constexpr auto& socket = win::socket;
constexpr auto& setsockopt = win::setsockopt;
constexpr auto& bind = win::bind;
constexpr auto& listen = win::listen;
constexpr auto& accept = win::accept;
constexpr auto& connect = win::connect;
constexpr auto& sendto = win::sendto;
constexpr auto& recvfrom = win::recvfrom;
constexpr auto& recv = win::recv;
constexpr auto& send = win::send;
constexpr auto& getsockname = win::getsockname;
constexpr auto& ntohl = win::ntohl;
constexpr auto& ntohs = win::ntohs;
constexpr auto& htonl = win::htonl;
constexpr auto& htons = win::htons;
constexpr auto IPPROTO_TCP = win::IPPROTO_TCP;
constexpr auto TCP_NODELAY = win::TCP_NODELAY;
constexpr auto PF_INET = win::PF_INET;
constexpr auto AF_INET = win::AF_INET;
constexpr auto INADDR_ANY = win::INADDR_ANY;
constexpr auto SOCK_STREAM = win::SOCK_STREAM;
constexpr auto SOCK_DGRAM = win::SOCK_DGRAM;
*/

#else //if !defined(OSK)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static void tl_init_sockets() {
	// Do nothing
}

//#define tl_native_socket(s) ((s)._int)
//#define native_socket(sock) ((sock).s)

TL_INLINE Socket make_socket(int native) {
	return Socket(native);
}

#endif

}

#endif // UUID_338826A0A88B40CBD1DE1AA4A2F6B6CD
