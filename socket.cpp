#include "socket.hpp"
#include "socket_sys.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace tl {

using namespace win;

static sockaddr_in* get_sockaddr_in(InternetAddr& self) {
	return (sockaddr_in*)&self.addr_in;
}

static sockaddr_in const* get_sockaddr_in_c(InternetAddr const& self) {
	return (sockaddr_in const*)&self.addr_in;
}

static sockaddr* get_sockaddr(InternetAddr& self) {
	return (sockaddr*)&self.addr;
}

static sockaddr const* get_sockaddr_c(InternetAddr const& self) {
	return (sockaddr const*)&self.addr;
}

#if TL_WINDOWS

static int error() { return win::WSAGetLastError(); }
#define sckerr_in_progress win::WSAEINPROGRESS
#define sckerr_would_block win::WSAEWOULDBLOCK
#define sckerr_conn_reset  win::WSAECONNRESET

Socket Socket::invalid() {
	return Socket(win::INVALID_SOCKET);
}

bool Socket::valid() const {
	return this->s != win::INVALID_SOCKET;
}

void Socket::close() {
	if (this->valid()) {
		win::closesocket(this->s);
	}
}

void Socket::close_send() {
	win::shutdown(this->s, 1);
}

int const error_ret = win::SOCKET_ERROR;

#else

static int error() { return errno; }
#define sckerr_in_progress EINPROGRESS
#define sckerr_would_block EWOULDBLOCK
#define sckerr_conn_reset  ECONNRESET

Socket Socket::invalid() {
	return Socket(-1);
}

bool Socket::valid() const {
	return this->s != -1;
}

void Socket::close() {
	if (this->valid()) {
		close(this->s);
	}
}

int const error_ret = -1;

#endif

#if 0
static char* copy_string(char const* p)
{
	size_t len = std::strlen(p) + 1;
	char* s = new char[len];
	std::memcpy(s, p, len);
	return s;
}

static char** copy_list(char** p)
{
	int i;
	int l = 0;
	char** n;
	for(; p[l]; ++l)
		/* nothing */;
	
	n = new char*[l + 1];
	for(i = 0; i < l; ++i)
	{
		n[i] = copy_string(p[i]);
	}
	
	n[l] = 0;
	
	return n;
}

static char** copy_list_l(char** p, size_t len)
{
	int i;
	int l = 0;
	char** n;
	for(; p[l]; ++l)
		/* nothing */;
	
	n = new char*[l + 1];
	for(i = 0; i < l; ++i)
	{
		n[i] = new char[len];
		std::memcpy(n[i], p[i], len);
	}
	
	n[l] = 0;
	
	return n;
}

static void free_list(char** p)
{
	int i;
	for(i = 0; p[i] != NULL; ++i)
		delete [] p[i];
	delete [] p;
}

struct host_entry_storage : host_entry_impl
{
	hostent v;
	
	~host_entry_storage();
	
	void* storage()
	{ return &v; }
};

static host_entry_storage* create_host_entry(hostent const* p)
{
	host_entry_storage* self = new host_entry_storage;
	self->v.h_name = copy_string(p->h_name);
	self->v.h_aliases = copy_list(p->h_aliases);
	self->v.h_addrtype = p->h_addrtype;
	self->v.h_length = p->h_length;
	self->v.h_addr_list = copy_list_l(p->h_addr_list, p->h_length);
	return self;
}

host_entry_storage::~host_entry_storage()
{
	delete [] v.h_name;
	free_list(v.h_aliases);
	free_list(v.h_addr_list);
}

host_entry* resolve_host(char const* name)
{
	hostent* p = gethostbyname( name ); // TODO: This is deprecated IIRC
	
	if(!p)
		return 0; // ERROR
	
	return create_host_entry(p);
}

host_entry::host_entry(char const* name)
: ptr(resolve_host(name))
{
}
#endif

Socket Socket::tcp() {
	tl_init_sockets();
	Socket s = make_socket(socket(PF_INET, SOCK_STREAM, 0));

	//if (!s.valid())
	//	return s;

	//s.nonblocking(true);
	return s;
}

Socket Socket::udp() {
	tl_init_sockets();
	Socket s = make_socket(socket(PF_INET, SOCK_DGRAM, 0));

	//if (!s.valid())
	//	return s;

	//s.nonblocking(true);
	return s;
}

void Socket::nonblocking(bool no_blocking) {
#if TL_WINDOWS
	unsigned long no_blocking_int = no_blocking;
	win::ioctlsocket(this->s, win::FIONBIO, &no_blocking_int);
#else
	fcntl(native_socket(*this), F_SETFL, no_blocking ? O_NONBLOCK : 0);
#endif
}

bool Socket::nodelay(bool no_delay) {

	char no_delay_int = (char)no_delay;
#if !defined(BEOS_NET_SERVER)
	return setsockopt(
		this->s,
		IPPROTO_TCP,
		TCP_NODELAY,
		(char*)&no_delay_int,
		sizeof(no_delay_int)) == 0;
#else
	return true;
#endif
}

bool Socket::bind(int port) {
	sockaddr_in addr;
	int ret;
	addr.sin_family = AF_INET;
	addr.sin_port = win::htons((u_short)port);
	addr.sin_addr.s_addr = INADDR_ANY;
	memset(&addr.sin_zero, 0, 8);

	ret = tl::bind(this->s, (sockaddr*)(&addr), sizeof(sockaddr_in)); // TODO: Some way to get size from addr
	return ret != error_ret;
}

bool Socket::listen() {
	int ret = win::listen(this->s, 5);
	return ret != error_ret;
}

Socket Socket::accept(InternetAddr& addr) {

	socklen_t sin_size = sizeof(sockaddr_in);

	// TODO: Check for errors
	return make_socket(tl::accept(this->s, get_sockaddr(addr), &sin_size));
}

bool Socket::connect(InternetAddr const& addr) {
	int r = tl::connect(this->s, get_sockaddr_c(addr), sizeof(sockaddr_in)); // TODO: Some way to get size from addr

	if (r == error_ret)	{
		int err = error();

#if TL_WIN32==1
		if (err != sckerr_would_block)
#else
		if (err != sckerr_in_progress)
#endif
			return false; // ERROR
	}

	return true;
}

static int translate_comm_result(int ret) {

	if (ret == 0) {
		return Socket::Disconnected;
	} else if (ret == error_ret || ret < 0) {
		int err = error();

#if 0
		if (err != sckerr_would_block)
			printf("Sockerr: %d\n", err);
#endif
		switch (err)
		{
		case sckerr_conn_reset: return Socket::ConnReset;
		case sckerr_would_block: return Socket::WouldBlock;
		case win::ERROR_IO_PENDING: return 0;
		default: return Socket::Failure;
		}
	}

	return ret;
}

static int translate_comm_result_async(int ret) {

	if (ret == error_ret || ret < 0) {
		int err = error();

#if 0
		if (err != sckerr_would_block)
			printf("Sockerr: %d\n", err);
#endif
		switch (err)
		{
		case sckerr_conn_reset: return Socket::ConnReset;
		case sckerr_would_block: return Socket::WouldBlock;
		case win::ERROR_IO_PENDING: return Socket::Pending;
		default: return Socket::Failure;
		}
	}

	return ret;
}

int Socket::send(tl::VecSlice<u8 const> data) {
	int ret = tl::send(this->s, (char const*)data.begin(), (int)data.size_bytes(), 0);

	return translate_comm_result(ret);
}

int Socket::recv(tl::VecSlice<u8> data) {
	int ret = tl::recv(this->s, (char*)data.begin(), (int)data.size_bytes(), 0);

	return translate_comm_result(ret);
}

int Socket::sendto(tl::VecSlice<u8 const> data, InternetAddr const& addr) {
	int ret = tl::sendto(
		this->s,
		(char const*)data.begin(),
		(int)data.size_bytes(), 0,
		get_sockaddr_c(addr),
		sizeof(sockaddr));

	return translate_comm_result(ret);
}

int Socket::recvfrom(tl::VecSlice<u8> data, InternetAddr& addr) {
	socklen_t fromlen = sizeof(sockaddr);
	int ret = tl::recvfrom(
		this->s,
		(char*)data.begin(),
		(int)data.size_bytes(), 0,
		get_sockaddr(addr),
		&fromlen);

	return translate_comm_result(ret);
}

#if TL_WINDOWS
int Socket::sendto_async(tl::VecSlice<u8 const> data, InternetAddr const& addr, win::OVERLAPPED* op) {
	win::WSABUF buffer = { data.size(), (win::CHAR *)data.begin() };

	memset(op, 0, sizeof(win::OVERLAPPED));

	int ret = win::WSASendTo(
		this->s,
		&buffer, 1,
		NULL,
		0,
		get_sockaddr_c(addr),
		sizeof(sockaddr),
		(win::LPWSAOVERLAPPED)op,
		NULL);

	return translate_comm_result_async(ret);
}

int Socket::recvfrom_async(tl::VecSlice<u8> data, InternetAddr* addr, win::OVERLAPPED* op) {
	win::WSABUF buffer = { data.size(), (win::CHAR *)data.begin() };

	memset(op, 0, sizeof(win::OVERLAPPED));

	win::DWORD flags = 0;
	socklen_t fromlen = sizeof(sockaddr);
	int ret = win::WSARecvFrom(
		this->s,
		&buffer, 1,
		NULL,
		&flags,
		get_sockaddr(*addr),
		&fromlen,
		(win::LPWSAOVERLAPPED)op,
		NULL);

	return translate_comm_result_async(ret);
}

int Socket::send_async(tl::VecSlice<u8 const> data, win::OVERLAPPED* op) {
	win::WSABUF buffer = { data.size(), (win::CHAR *)data.begin() };

	memset(op, 0, sizeof(win::OVERLAPPED));

	int ret = win::WSASend(
		this->s,
		&buffer, 1,
		NULL,
		0,
		(win::LPWSAOVERLAPPED)op,
		NULL);

	printf("WSASend() == %d\n", ret);

	return translate_comm_result_async(ret);
}

int Socket::recv_async(tl::VecSlice<u8> data, win::OVERLAPPED* op) {
	win::WSABUF buffer = { data.size(), (win::CHAR *)data.begin() };

	memset(op, 0, sizeof(win::OVERLAPPED));

	win::DWORD flags = 0;
	socklen_t fromlen = sizeof(sockaddr);
	int ret = win::WSARecv(
		this->s,
		&buffer, 1,
		NULL,
		&flags,
		(win::LPWSAOVERLAPPED)op,
		NULL);

	return translate_comm_result_async(ret);
}

int Socket::accept_async(win::OVERLAPPED* op) {
	//win::WSABUF buffer = { data.size(), (win::CHAR *)data.begin() };

	memset(op, 0, sizeof(win::OVERLAPPED));
	abort();

	/*
	auto ret = AcceptEx(
			this->s,
			sAcceptSocket,
			NULL,
			0,
			dwLocalAddressLength,
			dwRemoteAddressLength,
			NULL,
			(win::LPOVERLAPPED)op);
	*/

	return -1;
}
#endif

//
InternetAddr::InternetAddr() {
}

InternetAddr InternetAddr::from_socket(Socket sock) {
	InternetAddr self;
	self.reset();

	socklen_t t = sizeof(sockaddr_in);
	sockaddr_in addr;

	if (tl::getsockname(sock.native(), (sockaddr*)&addr, &t) != error_ret) {
		*get_sockaddr_in(self) = addr;
	}

	return self;
}

InternetAddr InternetAddr::from_ip4_port(u32 ip4, int port) {
	InternetAddr addr;
	addr.reset();
	sockaddr_in* saddr = get_sockaddr_in(addr);

	saddr->sin_family = AF_INET;
	saddr->sin_port = htons((u_short)port);
	saddr->sin_addr.s_addr = htonl(ip4);
	return addr;
}

InternetAddr InternetAddr::from_name(char const* name, int port) {
	InternetAddr addr;
	addr.reset();
	hostent* p = win::gethostbyname(name);
	
	if (p) {
		sockaddr_in* saddr = get_sockaddr_in(addr);

		memmove(&saddr->sin_addr, p->h_addr_list[0], p->h_length);
		saddr->sin_family = p->h_addrtype;
		saddr->sin_port = htons((u_short)port);
	}

	return addr;
}

int InternetAddr::port() const {
	sockaddr_in const* p = get_sockaddr_in_c(*this);
	return ntohs(p->sin_port);
}

void InternetAddr::port(int new_port) {
	sockaddr_in* p = get_sockaddr_in(*this);
	p->sin_port = htons((u_short)new_port);
}

u32 InternetAddr::ip4() const {
	sockaddr_in const* p = get_sockaddr_in_c(*this);
	return ntohl(p->sin_addr.s_addr);
}

void InternetAddr::ip4(u32 new_ip4) {
	sockaddr_in* p = get_sockaddr_in(*this);
	p->sin_addr.s_addr = htonl(new_ip4);
}

void InternetAddr::reset() {
	//memset(&this->s, 0, sizeof(this->s));
	memset(this, 0, sizeof(*this));
}

bool InternetAddr::operator==(InternetAddr const& other) const {
	return 0 == memcmp(this, &other, sizeof(*this));
}

}
