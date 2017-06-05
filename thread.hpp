#ifndef TL_THREAD_HPP
#define TL_THREAD_HPP 1

#include "platform.h"
#include "std.h"
#include <utility>
using std::move;

#if TL_WINDOWS
# include "windows/miniwindows.h"
#endif

namespace tl {

#if TL_WINDOWS

struct ThreadRef {
	win::HANDLE h;
	win::DWORD thread_id;

	ThreadRef()
		: h(win::INVALID_HANDLE_VALUE) {
	}

	ThreadRef(win::HANDLE h, win::DWORD thread_id)
		: h(h), thread_id(thread_id) {
	}

	void suspend() {
		win::SuspendThread(this->h);
	}

	void resume() {
		win::ResumeThread(this->h);
	}

	bool join() {
		return win::WAIT_OBJECT_0 == win::WaitForSingleObject(this->h, win::INFINITE);
	}

	void close() { win::CloseHandle(this->h); }

	static ThreadRef create(bool suspended, void(*f)(void* param), void* param) {
		win::DWORD thread_id;
		auto handle = win::CreateThread(NULL, 0, (win::LPTHREAD_START_ROUTINE)f, param, suspended ? win::CREATE_SUSPENDED : 0, &thread_id);

		return ThreadRef(handle, thread_id);
	}
};

struct Thread : ThreadRef {
	TL_NON_COPYABLE(Thread);
	//TL_MOVABLE(Thread);

	Thread(Thread&& other)
		: ThreadRef(other.h, other.thread_id) {
		other.h = win::INVALID_HANDLE_VALUE;
	}

	Thread& operator=(Thread&& other) = delete;

	explicit Thread(ThreadRef ref)
		: ThreadRef(ref) {
	}

	static Thread create(bool suspended, void(*f)(void* param), void* param) {
		return Thread(ThreadRef::create(suspended, f, param));
	}

	~Thread() {
		this->close();
	}
};

struct ThreadScoped : Thread {
	using Thread::Thread;

	ThreadScoped(Thread&& th)
		: Thread(move(th)) {
	}

	TL_NON_COPYABLE(ThreadScoped);
	TL_MOVABLE(ThreadScoped);

	template<typename F>
	static ThreadScoped create(bool suspended, F&& f) {
		F* p = new F(move(f));
		return Thread::create(suspended, [](void* p) {
			(*(F *)p)();
			delete p;
		}, p);
	}

	~ThreadScoped() {
		if (this->h != win::INVALID_HANDLE_VALUE) {
			this->join();
		}
	}
};

#endif

}

#endif // TL_THREAD_HPP

