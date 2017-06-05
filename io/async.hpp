#ifndef TL_IO_ASYNC_HPP
#define TL_IO_ASYNC_HPP 1

#include "../platform.h"
#include "../io.hpp"
#include "../region.hpp"
#include "../windows/miniwindows.h"
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

win::HANDLE to_win_handle(Handle h) {
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

#if 0
	PendingOperation* wait(int timeout, uintptr_t& key, u32& transferred) {
		LPOVERLAPPED overlapped;
		return ::GetQueuedCompletionStatus(this->h, (DWORD *)&transferred, (ULONG_PTR *)&key, &overlapped, timeout)
			? (PendingOperation *)overlapped
			: NULL;
	}
#endif

	void close() {
		tl::win::CloseHandle((tl::win::HANDLE)this->h);
		//CloseHandle(this->h);
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

		RecvFrom = Read | Connectionless,
		SendTo = Write | Connectionless,
	};

	IocpOp(Type type)
		: type(type) {
	}

	Type type;
};


struct IocpRecvFromOp : IocpOp {
	IocpRecvFromOp()
		: IocpOp(RecvFrom), size(64 * 1024) {
	}

	VecSlice<u8> slice() { return VecSlice<u8>(buffer, buffer + size); }
	VecSlice<u8 const> slice_const() { return VecSlice<u8 const>(buffer, buffer + size); }

	InternetAddr from;
	usize size;
	u8 buffer[64 * 1024];
};

struct IocpSendToOp : IocpOp {
	IocpSendToOp()
		: IocpOp(SendTo), size(0) {
	}

	VecSlice<u8> slice() { return VecSlice<u8>(buffer, buffer + size); }
	VecSlice<u8 const> slice_const() { return VecSlice<u8 const>(buffer, buffer + size); }

	usize size;
	u8 buffer[64 * 1024];
};

struct Iocp;

typedef unique_ptr<IocpOp, FreelistDelete> IocpOpPtr;
typedef unique_ptr<IocpRecvFromOp, FreelistDelete> IocpRecvFromOpPtr;
typedef unique_ptr<IocpSendToOp, FreelistDelete> IocpSendToOpPtr;

struct IocpEvent {
	IocpEvent(bool is_read, Handle h, IocpOpPtr read_data, InternetAddr from)
		: h(h), op(move(read_data)),
		from(from), is_read(is_read) {
	}

	TL_NON_COPYABLE(IocpEvent);
	TL_MOVABLE(IocpEvent);

	~IocpEvent() {
	}

	Handle h;
	IocpOpPtr op;

	InternetAddr from;
	bool is_read;
};

struct Iocp : private IocpRef {
	TL_NON_COPYABLE(Iocp);
	TL_MOVABLE(Iocp);

	//using IocpRef::wait;

	tl::Region region;
	tl::Freelist<sizeof(IocpRecvFromOp)> recvFromFree;
	tl::Freelist<sizeof(IocpSendToOp)> sendToFree;

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

	void delete_send_to_op(IocpSendToOp* p) {
		p->~IocpSendToOp();
		sendToFree.free(p);
	}

	void schedule_recvfrom(Handle handle) {
		IocpRecvFromOp* new_op = new_recv_from_op();

		int r = Socket((uintptr_t)handle.h).recvfrom_async(new_op->slice(), &new_op->from, new_op);
		printf("r = %d\n", r);
		if (r < 0) {
			delete_recv_from_op(new_op);
		}
	}

	void sendto(Socket socket, IocpSendToOp* new_op, InternetAddr const& addr) {
		auto slice = new_op->slice_const();

		if (socket.sendto_async(slice, addr, new_op) < 0) {
			delete_send_to_op(new_op);
		}
	}

	void reg(Handle handle) {
		this->reg_raw(handle);
		schedule_recvfrom(handle);
	}

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

				if (op->type == IocpOp::RecvFrom) {
					IocpRecvFromOp* recvFromOp = (IocpRecvFromOp *)op;
					recvFromOp->size = entry.dwNumberOfBytesTransferred;

					events.push_back(IocpEvent(true, handle, IocpOpPtr(recvFromOp, FreelistDelete(recvFromFree)), recvFromOp->from));
					schedule_recvfrom(handle);
				} else if (op->type == IocpOp::SendTo) {
					// TODO

					this->delete_send_to_op((IocpSendToOp *)op);
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
