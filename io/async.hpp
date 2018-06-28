#ifndef TL_IO_ASYNC_HPP
#define TL_IO_ASYNC_HPP 1

#include "../platform.h"
#include "../io.hpp"
#include "../region.hpp"
#include "../windows/miniwindows.h"
#include "../socket.hpp"
#include <string.h>
#include <memory>
using std::unique_ptr;

namespace tl {

#if TL_WINDOWS
TL_ALIGN(8) struct PendingOperation {
	static usize const Size = 2 * sizeof(usize) + 8 + sizeof(void*);
	u8 s[Size];

	PendingOperation() {
		memset(s, 0, Size);
	}

	/*
	uintptr_t Internal;
	uintptr_t InternalHigh;
	union {
	struct {
	u32 Offset;
	u32 OffsetHigh;
	} DUMMYSTRUCTNAME;
	void* Pointer;
	} DUMMYUNIONNAME;

	void*  hEvent;
	*/
};

inline win::HANDLE to_win_handle(Handle h) {
	return (tl::win::HANDLE)h.h;
}

struct IocpRef {
	win::HANDLE h;

	IocpRef()
		: h(win::INVALID_HANDLE_VALUE) {
	}

	explicit IocpRef(tl::win::HANDLE h)
		: h(h) {
	}

	void reg_raw(Handle handle) {
		win::CreateIoCompletionPort(to_win_handle(handle), this->h, (win::ULONG_PTR)handle.h, 0);
	}

	void close() {
		tl::win::CloseHandle((tl::win::HANDLE)this->h);
	}

	static IocpRef create() {
		IocpRef ret(win::CreateIoCompletionPort(win::INVALID_HANDLE_VALUE, NULL, 0, 0));
		win::DWORD h = win::GetLastError();
		return ret;
	}
};

struct IocpOp : win::OVERLAPPED {
	enum Type : u8 {
		Read = (1 << 0),
		Write = (0 << 0),

		Socket = (1 << 1),
		Connectionless = (1 << 2) | Socket,
		Accept = (1 << 3) | Socket,

		RecvFrom = Read | Connectionless,
		SendTo = Write | Connectionless,
	};

	IocpOp(Type type)
		: type(type) {
	}

	Type type;
};

struct IocpAccept : IocpOp {
	IocpAccept(Type type = Accept)
		: IocpOp(type) {
	}

	InternetAddr from;
};

struct IocpRecvFromOp : IocpOp {
	IocpRecvFromOp(Type type = RecvFrom)
		: IocpOp(type), size(64 * 1024) {
	}

	VecSlice<u8> slice() { return VecSlice<u8>(buffer, buffer + size); }
	VecSlice<u8 const> slice_const() { return VecSlice<u8 const>(buffer, buffer + size); }

	InternetAddr from;
	usize size;
	u8 buffer[64 * 1024];
};

struct IocpAcceptOp : IocpOp {
	IocpAcceptOp(tl::InternetAddr from)
		: IocpOp(Accept), from(from) {
	}

	InternetAddr from;
};

/*
struct IocpRecvOp : IocpOp {
	IocpRecvOp()
		: IocpOp(RecvFrom), size(64 * 1024) {
	}

	VecSlice<u8> slice() { return VecSlice<u8>(buffer, buffer + size); }
	VecSlice<u8 const> slice_const() { return VecSlice<u8 const>(buffer, buffer + size); }

	usize size;
	u8 buffer[64 * 1024];
};*/

struct IocpSendToOp : IocpOp {
	IocpSendToOp()
		: IocpOp(SendTo), size(0) {
	}

	VecSlice<u8> slice() { return VecSlice<u8>(buffer, buffer + size); }
	VecSlice<u8 const> slice_const() { return VecSlice<u8 const>(buffer, buffer + size); }

	usize size;
	u8 buffer[64 * 1024];
};

struct IocpSendOp : IocpOp {
	IocpSendOp(tl::Vec<u8> buf)
		: IocpOp(Write), buf(move(buf)) {
	}

	VecSlice<u8> slice() { return buf.slice(); }
	VecSlice<u8 const> slice_const() { return buf.slice_const(); }

	tl::Vec<u8> buf;
};

struct Iocp;

typedef unique_ptr<IocpOp, FreelistDelete> IocpOpPtr;
typedef unique_ptr<IocpRecvFromOp, FreelistDelete> IocpRecvFromOpPtr;
typedef unique_ptr<IocpSendToOp, FreelistDelete> IocpSendToOpPtr;

struct IocpEvent {
	IocpEvent(IocpOp::Type type, Handle h, IocpOpPtr read_data, InternetAddr from)
		: h(h), op(move(read_data)),
		from(from), type(type) {
	}

	IocpEvent(IocpOp::Type type, Handle h, IocpOpPtr data)
		: h(h), op(move(data)),
		from(), type(type) {
	}

	TL_NON_COPYABLE(IocpEvent);
	TL_MOVABLE(IocpEvent);

	~IocpEvent() {
	}

	Handle h;
	IocpOpPtr op;

	InternetAddr from;
	IocpOp::Type type;

	bool is_read_() const {
		return (this->type & IocpOp::Read) != 0;
	}
};

struct Iocp : private IocpRef {
	TL_NON_COPYABLE(Iocp);
	TL_MOVABLE(Iocp);

	//using IocpRef::wait;

	tl::Region region;
	tl::Freelist<sizeof(IocpRecvFromOp)> recvFromFree;
	tl::Freelist<sizeof(IocpAcceptOp)> acceptFree;
	tl::Freelist<sizeof(IocpSendToOp)> sendToFree;
	tl::Freelist<sizeof(IocpSendOp)> sendFree;

	Iocp() {
	}

	~Iocp() {
		this->close();
	}

	explicit Iocp(IocpRef ref)
		: IocpRef(ref) {
	}

	IocpRef ref() const {
		return *this;
	}

	static Iocp create() {
		return Iocp(IocpRef::create());
	}

	IocpRecvFromOp* new_recv_from_op() {
		void* p = recvFromFree.try_alloc();
		if (!p) p = region.alloc(sizeof(IocpRecvFromOp));
		return new (p) IocpRecvFromOp();
	}

	IocpRecvFromOp* new_recv_op() {
		void* p = recvFromFree.try_alloc();
		if (!p) p = region.alloc(sizeof(IocpRecvFromOp));
		return new (p) IocpRecvFromOp(IocpOp::Read);
	}

	IocpAcceptOp* new_accept_op(tl::InternetAddr addr) {
		return new IocpAcceptOp(addr);
	}

	void delete_recv_from_op(IocpRecvFromOp* p) {
		p->~IocpRecvFromOp();
		recvFromFree.free(p);
	}

	IocpSendToOp* new_send_to_op() {
		void* p = sendToFree.try_alloc();
		if (!p) p = region.alloc(sizeof(IocpSendToOp));
		return new (p) IocpSendToOp();
	}

	IocpSendToOpPtr new_send_to_op_ptr() {
		return IocpSendToOpPtr(new_send_to_op(), FreelistDelete(sendToFree));
	}

	IocpSendOp* new_send_op(tl::Vec<u8> buf) {
		void* p = sendFree.try_alloc();
		if (!p) p = region.alloc(sizeof(IocpSendOp));
		return new (p) IocpSendOp(move(buf));
	}

	void delete_send_to_op(IocpSendToOp* p) {
		p->~IocpSendToOp();
		sendToFree.free(p);
	}

	void delete_send_op(IocpSendOp* p) {
		p->~IocpSendOp();
		sendFree.free(p);
	}

	void delete_accept_op(IocpAcceptOp* p) {
		delete p;
	}

	void schedule_recvfrom(Handle handle) {
		IocpRecvFromOp* new_op = new_recv_from_op();

		int r = Socket((uintptr_t)handle.h).recvfrom_async(new_op->slice(), &new_op->from, new_op);
		printf("r = %d\n", r);
		if (r < 0) {
			delete_recv_from_op(new_op);
		}
	}

	void schedule_recv(Handle handle) {
		IocpRecvFromOp* new_op = new_recv_op();

		int r = Socket((uintptr_t)handle.h).recv_async(new_op->slice(), new_op);
		printf("r = %d\n", r);
		if (r < 0) {
			delete_recv_from_op(new_op);
		}
	}

	/*
	void schedule_accept(Handle handle) {
		IocpRecvFromOp* new_op = new_recv_op();

		int r = Socket((uintptr_t)handle.h).recv_async(new_op->slice(), new_op);
		printf("r = %d\n", r);
		if (r < 0) {
			delete_recv_from_op(new_op);
		}
	}
	*/

	void sendto(Socket socket, IocpSendToOp* new_op, InternetAddr const& addr) {
		auto slice = new_op->slice_const();

		if (socket.sendto_async(slice, addr, new_op) < 0) {
			delete_send_to_op(new_op);
		}
	}

	bool send(Socket socket, IocpSendOp* new_op) {
		auto slice = new_op->slice_const();

		int err = socket.send_async(slice, new_op);
		if (err < 0) {
			delete_send_op(new_op);
			return false;
		}

		return true;
	}

	bool post(Handle handle, IocpOp* op) {
		return win::PostQueuedCompletionStatus(this->h, 0, handle.h, op) != 0;
	}

	void begin_recvfrom(Handle handle) {
		this->reg_raw(handle);
		schedule_recvfrom(handle);
	}

	void begin_recv(Handle handle) {
		this->reg_raw(handle);
		schedule_recv(handle);
	}

	/*
	void begin_listen(Handle handle) {
		this->reg_raw(handle);
		schedule_accept(handle);
	}
	*/

	bool wait(Vec<IocpEvent>& events, int timeout) {
		tl::win::OVERLAPPED_ENTRY buf[64];
		win::ULONG events_returned = 0;
		win::BOOL res = win::GetQueuedCompletionStatusEx(this->h, buf, sizeof(buf) / sizeof(*buf), &events_returned, timeout, win::FALSE);

		if (!res) {
			return false;
		}

		for (win::ULONG i = 0; i < events_returned; ++i) {
			auto& entry = buf[i];

			{
				IocpOp* op = (IocpOp *)entry.lpOverlapped;
				Handle handle = Handle((usize)entry.lpCompletionKey);

				// TODO: Better error detection/handling
				if (entry.dwNumberOfBytesTransferred == 0 && op->type != IocpOp::Accept) {
					// TODO: IocpOp still needs to be freed
					continue;
				}

				if (op->type == IocpOp::RecvFrom) {
					IocpRecvFromOp* recvFromOp = (IocpRecvFromOp *)op;
					recvFromOp->size = entry.dwNumberOfBytesTransferred;

					events.push_back(IocpEvent(op->type, handle, IocpOpPtr(recvFromOp, FreelistDelete(recvFromFree)), recvFromOp->from));
					schedule_recvfrom(handle);
				} else if (op->type == IocpOp::SendTo) {
					// TODO

					this->delete_send_to_op((IocpSendToOp *)op);
				} else if (op->type == IocpOp::Read) {
					IocpRecvFromOp* recvFromOp = (IocpRecvFromOp *)op;
					recvFromOp->size = entry.dwNumberOfBytesTransferred;

					events.push_back(IocpEvent(op->type, handle, IocpOpPtr(recvFromOp, FreelistDelete(recvFromFree)), recvFromOp->from));
					schedule_recv(handle);
				} else if (op->type == IocpOp::Write) {
					IocpSendOp* sendOp = (IocpSendOp *)op;

					events.push_back(IocpEvent(op->type, handle, IocpOpPtr(sendOp, FreelistDelete(sendFree))));

					//this->delete_send_op((IocpSendOp *)op);
				} else if (op->type == IocpOp::Accept) {
					IocpAcceptOp* acceptOp = (IocpAcceptOp *)op;

					events.push_back(IocpEvent(op->type, handle, IocpOpPtr(), acceptOp->from));
					this->delete_accept_op(acceptOp);
				} else {
					// Not implemented
					TL_UNREACHABLE();
				}
			}
		}

		return true;
	}

	IocpRef unwrap() {
		IocpRef ret(*this);
		*static_cast<IocpRef *>(this) = IocpRef();
		return ret;
	}
};

#else
# error "Not implemented"
#endif

}

#endif // TL_IO_ASYNC_HPP
