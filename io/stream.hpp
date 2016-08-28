#ifndef UUID_49614D7C9472495896A2EF850C345AEA
#define UUID_49614D7C9472495896A2EF850C345AEA 1

#include "../cstdint.h"
#include "../vec.hpp"
#include "../string.hpp"
#include "../memory.h"
#include <memory>
#include <utility>

namespace tl {

struct Source;
struct Sink;
struct SinkBuf;

struct Pullable {
	enum struct Result {
		None = 0,
		HasRequestedAmount = 1 << 0,
		Eof = 1 << 1,
		Error = 1 << 2
	};

	usize move_data_left(Source& src, u8* new_buffer);

	virtual Result pull(Source&, usize min_size) = 0;
	virtual ~Pullable() = 0 {}
};

inline Pullable::Result operator|(Pullable::Result a, Pullable::Result b) {
	return Pullable::Result((u32)a | (u32)b);
}

inline Pullable::Result operator&(Pullable::Result a, Pullable::Result b) {
	return Pullable::Result((u32)a & (u32)b);
}

inline bool has_requested_amount(Pullable::Result r) {
	return (r & Pullable::Result::HasRequestedAmount) != Pullable::Result::None;
}

inline bool is_eof(Pullable::Result r) {
	return (r & Pullable::Result::Eof) != Pullable::Result::None;
}

struct Pushable {
	Pushable() : base_position(0) {
	}

	usize base_position;
	virtual int push(SinkBuf&, usize min_size) = 0;
	virtual ~Pushable() = 0 {}
};

struct SourceBuf : private VecSlice<u8 const> {

	using VecSlice<u8 const>::begin;
	using VecSlice<u8 const>::end;
	using VecSlice<u8 const>::empty;
	using VecSlice<u8 const>::size;
	using VecSlice<u8 const>::unsafe_get_le;
	
	SourceBuf() {}

	SourceBuf(u8 const* cur_init, u8 const* end_init)
		: VecSlice<u8 const>(cur_init, end_init) {}

	SourceBuf(SourceBuf&& other) = default;
	SourceBuf& operator=(SourceBuf&) = delete;

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

struct SinkBuf {
	SinkBuf() : beg(0), cur(0), end(0) {}

	SinkBuf(u8* beg_init, u8* cur_init, u8* end_init)
		: beg(beg_init), cur(cur_init), end(end_init) {}

	SinkBuf(SinkBuf&& other) = default;
	SinkBuf& operator=(SinkBuf&) = delete;

	void unsafe_put(u8 byte) { *cur++ = byte; }
	void unsafe_put(u8 const* data, usize size) { memcpy(this->cur, data, size); this->cur += size; }

	VecSlice<u8 const> get_written() const {
		return VecSlice<u8 const>(beg, cur);
	}

	void unsafe_cut_front(usize amount) {
		beg += amount;
		assert(beg <= cur);
	}

	void unsafe_set_buffer(u8* beg_new, u8* cur_new, u8* end_new) {
		this->beg = beg_new;
		this->cur = cur_new;
		this->end = end_new;
	}

	usize size() const {
		return end - cur;
	}

	usize written_size() const {
		return cur - beg;
	}

protected:
	u8* beg;
	u8* cur;
	u8* end;
};

struct Source : SourceBuf {
	Source(Source&& other) = default;

	Source(std::unique_ptr<Pullable> src_init)
	 : src(std::move(src_init)) {
	}

	u8 get_u8_def(u8 def = 0) {
		return ensure() ? def : unsafe_get();
	}

	template<typename T>
	T get_le(T def = T()) {
		if (ensure(sizeof(T)))
			return def;
		return this->unsafe_get_le<T>();
	}

	// NOTE: Any subsequent i/o operation on this Source will invalidate the returned slice.
	VecSlice<u8 const> window(usize amount) {
		if (ensure(amount)) {
			return VecSlice<u8 const>();
		}

		u8 const* begin = this->begin();
		this->unsafe_skip(amount);
		return VecSlice<u8 const>(begin, begin + amount);
	}

	VecSlice<u8 const> read_all() {
		for (;;) {
			if (is_eof(src->pull(*this, 0)))
				break;
		}

		u8 const* begin = this->begin();
		
		return VecSlice<u8 const>(begin, this->end());
	}

	bool ensure() {
		return !this->empty() ? false : !has_requested_amount(src->pull(*this, 1));
	}

	bool ensure(usize amount) {
		usize l = size();
		if (l < amount) {
			return !has_requested_amount(src->pull(*this, amount));
		}

		return false;
	}

	bool skip(usize amount) {
		if (ensure(amount))
			return true;

		this->unsafe_skip(amount);
		return false;
	}

	static Source from_file(char const* path);
	static Source from_file(StringSlice path);

private:

	std::unique_ptr<Pullable> src;
};

struct Sink : SinkBuf {

	static usize const direct_write_threshold = 1024;

	Sink(Sink&& other) = default;

	Sink(std::unique_ptr<Pushable> snk_init)
		: snk(std::move(snk_init)) {
	}

	~Sink() { this->flush(); }
	
	int put(u8 byte) {
		return check_push() ? 0 : (unsafe_put(byte), 1);
	}

	int put(VecSlice<u8 const> data) {
		usize s = data.size();
		if (check_push(s)) return 1;
		unsafe_put(data.begin(), s);

		return 0;
	}

	template<typename T>
	int put(T data) {
		if (check_push(sizeof(T))) return 1;
		unsafe_put((u8 const*)&data, sizeof(T));

		return 0;
	}

	VecSlice<u8> window(usize amount) {
		if (check_push(amount)) {
			return VecSlice<u8>();
		}

		u8* begin = this->cur;
		this->cur += amount;
		return VecSlice<u8>(begin, this->cur);
	}

	void flush() {
		if (snk) snk->push(*this, 1);
	}

	std::unique_ptr<Pushable> unwrap() {
		this->flush();
		return std::move(snk);
	}

	usize position() const {
		return this->snk->base_position + this->written_size();
	}

	static Sink from_file(char const* path);
	static Sink from_file(StringSlice path);

protected:

	int check_push(usize required = 1) {
		return usize(end - cur) >= required ? 0 : snk->push(*this, required);
	}

	std::unique_ptr<Pushable> snk;
};

struct PushableVector : Pushable {

	int push(SinkBuf& owner, usize min_size) {
		auto to_write = owner.get_written();

		this->base_position += to_write.size();
		vec.unsafe_set_size(vec.size() + to_write.size());
		vec.enlarge(vec.size() < min_size ? min_size - vec.size() : 128);

		owner.unsafe_set_buffer(vec.end(), vec.end(), vec.cap_end());
		return 0;
	}

	usize position() const {
		return vec.size();
	}

	tl::Vec<u8> unwrap() {
		return move(vec);
	}

	virtual ~PushableVector() {}

private:
	tl::Vec<u8> vec;
};

struct SinkVector : Sink {
	SinkVector() : Sink(std::make_unique<PushableVector>()) {
	}

	/*
	usize current_size() {
		return static_cast<PushableVector *>(this->snk.get())->size() + this->written_size();
	}
	*/

	tl::Vec<u8> unwrap_vec() {
		auto v = this->unwrap();
		return static_cast<PushableVector *>(v.get())->unwrap();
	}
};

struct FixedBufferPullable : Pullable {
	static usize const BufferSize = 4096;

	FixedBufferPullable()
		: buffer(0)
		, buffer_size(0) {
	}

	~FixedBufferPullable() {
		memfree(this->buffer);
	}

	// Returns unused space left at the end of the buffer
	usize pull_begin(Source& src, usize min_size) {
		
		usize data_left = this->move_data_left(src, this->buffer);
		usize old_buffer_size = buffer_size;

		if (buffer_size < min_size) {
			// TODO: If top bit of min_size is set, buffer_size will break
			buffer_size = usize(1) << (tl_fls64(min_size) + 1);
		} else {
			buffer_size = tl::max(BufferSize, buffer_size * 2); // TODO: Check buffer_size overflow
		}

		this->buffer = (u8 *)memrealloc(this->buffer, buffer_size, old_buffer_size);
		src.unsafe_set_buffer(this->buffer, this->buffer + data_left);

		return buffer_size - data_left;
	}

	void pull_done(Source& src, usize amount) {
		src.unsafe_expand_buffer(amount);
	}

	u8* buffer;
	usize buffer_size;
};

struct FilePullable : FixedBufferPullable {
	static FilePullable* open(char const* path);
};

struct FixedBufferPushable : Pushable {
	static usize const BufferSize = 4096;

	FixedBufferPushable()
		: buffer((u8 *)memalloc(BufferSize))
		, buffer_size(BufferSize) {
	}

	~FixedBufferPushable() {
		memfree(this->buffer);
	}

	u8* buffer;
	usize buffer_size;
};

struct FilePushable : FixedBufferPushable {
	static FilePushable* open(char const* path);
};

} // namespace tl

#endif // UUID_49614D7C9472495896A2EF850C345AEA
