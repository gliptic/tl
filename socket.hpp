#ifndef UUID_BEDBCF086FF249445BE6E39168A62865
#define UUID_BEDBCF086FF249445BE6E39168A62865

#include <stddef.h>
#include "config.h"
#include "cstdint.h"
#include "platform.h"
#include "vec.hpp"
#include "io.hpp"
#include "windows/miniwindows.h"

namespace tl {

#define tl_failure      (-1)
#define tl_would_block  (-2)
#define tl_conn_reset   (-3)
#define tl_disconnected (-4)

struct InternetAddr;
struct PendingOperation;

struct Socket {
	enum Result : int {
		Failure = -1,
		WouldBlock = -2,
		ConnReset = -3,
		Disconnected = -4
	};

#if TL_WINDOWS
	win::SOCKET s;

	Socket(win::SOCKET s) : s(s) {}

	win::SOCKET native() {
		return this->s;
	}
#else
	int s;

	Socket(int s) : s(s) {}

	int native() {
		return this->s;
	}
#endif

	static Socket invalid();
	static Socket tcp();
	static Socket udp();

	bool valid() const;
	void close();

	void nonblocking(bool no_blocking);
	bool nodelay(bool no_delay);
	bool bind(int port);
	bool listen();
	bool connect(InternetAddr const& addr);
	Socket accept(InternetAddr& addr);
	int send(tl::VecSlice<u8 const> data);
	int recv(tl::VecSlice<u8> data);
	int sendto(tl::VecSlice<u8 const> data, InternetAddr const& addr);
	int recvfrom(tl::VecSlice<u8> data, InternetAddr& addr);

#if TL_WINDOWS
	int sendto_async(tl::VecSlice<u8 const> data, InternetAddr const& addr, win::OVERLAPPED* op);
	int recvfrom_async(tl::VecSlice<u8> data, InternetAddr* addr, win::OVERLAPPED* op);
#endif

	Handle to_handle() {
		return Handle((Win_HANDLE)this->s);
	}
};

/*TL_ALIGN(8) */ struct InternetAddr {

	//u8 s[128];
	union {
		win::sockaddr addr;
		win::sockaddr_in addr_in;
	};

	InternetAddr();
	static InternetAddr from_socket(Socket socket);
	static InternetAddr from_ip4_port(u32 ip4, int port);
	static InternetAddr from_name(char const* name, int port);

	int port() const;
	void port(int new_port);
	u32 ip4() const;
	void ip4(u32 new_ip4);
	void reset();

	bool operator==(InternetAddr const&) const;
};

}

#endif // UUID_BEDBCF086FF249445BE6E39168A62865
