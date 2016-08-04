#ifndef UUID_49614D7C9472495896A2EF850C345AEA
#define UUID_49614D7C9472495896A2EF850C345AEA 1

#include "cstdint.h"
#include "vec.hpp"
#include "string.hpp"
#include "memory.h"
#include <memory>
#include <utility>

namespace tl {

struct source;
struct sink;

struct pullable {
	usize move_data_left(source& src, u8* new_buffer);

	virtual int pull(source&, usize min_size) = 0;
	virtual ~pullable() = 0 {}
};

struct pushable {
	// TODO: Parameter for whether to request more space. 0 for flush.
	virtual int push(sink&) = 0;
	virtual ~pushable() = 0 {}
};

struct source_buf : private VecSlice<u8 const> {
	friend struct pullable;

	using VecSlice<u8 const>::begin;
	using VecSlice<u8 const>::end;
	using VecSlice<u8 const>::empty;
	using VecSlice<u8 const>::size;
	using VecSlice<u8 const>::unsafe_get_le;
	
	source_buf() {}

	source_buf(u8 const* cur_init, u8 const* end_init)
		: VecSlice<u8 const>(cur_init, end_init) {}

	source_buf(source_buf&& other) = default;
	source_buf& operator=(source_buf&) = delete;

	u8 unsafe_get() { return this->unsafe_pop_front(); }
	
	void unsafe_expand_buffer(usize amount) {
		this->unsafe_inc_size_bytes(amount);
	}

	void unsafe_skip(usize amount) {
		this->unsafe_cut_front(amount);
	}

	void unsafe_set_buffer(u8 const* cur, u8 const* end) {
		VecSlice<u8 const> *base = this;
		*base = VecSlice<u8 const>(cur, end);
	}

protected:
	using VecSlice<u8 const>::begin_bytes_ptr;
};

struct sink_buf {
	sink_buf() : beg(0), cur(0), end(0) {}

	sink_buf(u8* beg_init, u8* cur_init, u8* end_init)
		: beg(beg_init), cur(cur_init), end(end_init) {}

	sink_buf(sink_buf&& other) = default;
	sink_buf& operator=(sink_buf&) = delete;

	void unsafe_put(u8 byte) { *cur++ = byte; }

protected:
	u8* beg;
	u8* cur;
	u8* end;
};

struct StreamWindow : VecSlice<u8 const> {
	StreamWindow(VecSlice<u8 const> slice, u8** begin_ptr_init)
		: VecSlice<u8 const>(slice)
		, begin_ptr(begin_ptr_init) {
	}

	operator bool() const {
		return !this->empty();
	}

	void done() {
		*begin_ptr = (u8 *)this->end();
	}

private:
	u8** begin_ptr;
};

struct source : source_buf {
	source(source&& other)
	 : src(std::move(other.src)) {
	}

	source(std::unique_ptr<pullable> src_init)
	 : src(std::move(src_init)) {
	}

	u8 get_u8_def(u8 def = 0) {
		return check_pull() ? def : unsafe_get();
	}

	template<typename T>
	T get_le(T def = T()) {
		if (ensure(sizeof(T)))
			return def;
		return this->unsafe_get_le<T>();
	}

	StreamWindow window(usize amount) {
		if (ensure(amount)) {
			amount = 0;
		}

		return StreamWindow(VecSlice<u8 const>(this->begin(), this->begin() + amount), this->begin_bytes_ptr());
	}

	int ensure(usize amount) {
		usize l = size();
		if (l < amount) {
			return src->pull(*this, amount - l);
		}

		return 0;
	}

	int skip(usize amount) {
		int c = ensure(amount);
		this->unsafe_skip(amount);
		
		return c;
	}

	static source from_file(char const* path);
	static source from_file(StringSlice path);

private:
	int check_pull(usize min_size = 0) {
		return !this->empty() ? 0 : src->pull(*this, min_size);
	}

	std::unique_ptr<pullable> src;
};

/*
struct source {
	Err skip(usize amount);
	Err read_vec(usize amount, Vec<u8>& out);
	Err ensure(usize amount);
}
*/

struct sink : sink_buf {

	friend struct pushable_vector;

	sink(std::unique_ptr<pushable> snk_init)
		: snk(std::move(snk_init)) {
	}
	
	int put(u8 byte) {
		return check_push() ? 0 : (unsafe_put(byte), 1);
	}

	std::unique_ptr<pushable> unwrap() {
		snk->push(*this);
		return std::move(snk);
	}

private:

	int check_push() {
		return cur != end ? 0 : snk->push(*this);
	}

	std::unique_ptr<pushable> snk;
};

struct pushable_vector : pushable {

	int push(sink& owner) {
		vec.unsafe_set_size(vec.size() + (owner.cur - owner.beg));
		vec.enlarge(1);

		// TODO: Function for this in pushable. No need for sink to befriend everyone.
		owner.beg = owner.cur = vec.end();
		owner.end = vec.cap_end();
		return 0;
	}

	virtual ~pushable_vector() {}

private:
	tl::Vec<u8> vec;
};

struct fixed_buffer_pullable : pullable {
	static usize const BufferSize = 4096;

	fixed_buffer_pullable()
		: buffer((u8 *)memalloc(BufferSize))
		, buffer_size(BufferSize) {
	}

	~fixed_buffer_pullable() {
		memfree(this->buffer);
	}

	// Returns unused space left at the end of the buffer
	usize pull_begin(source& src, usize min_size) {
		
		usize data_left = this->move_data_left(src, this->buffer);
		if (buffer_size < min_size) {
			usize old_buffer_size = buffer_size;
			do
				buffer_size *= 2;
			while (buffer_size < min_size);

			this->buffer = (u8 *)memrealloc(this->buffer, buffer_size, old_buffer_size);
			src.unsafe_set_buffer(this->buffer, this->buffer + data_left);
		}

		return buffer_size - data_left;
	}

	void pull_done(source& src, usize amount) {
		src.unsafe_expand_buffer(amount);
	}

	u8* buffer;
	usize buffer_size;
};

struct file_pullable : fixed_buffer_pullable {
	static file_pullable* open(char const* path);
};

} // namespace tl

#endif // UUID_49614D7C9472495896A2EF850C345AEA
