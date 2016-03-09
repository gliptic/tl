#ifndef UUID_49614D7C9472495896A2EF850C345AEA
#define UUID_49614D7C9472495896A2EF850C345AEA 1

#include "cstdint.h"
#include "vector.hpp"
#include <memory>
#include <utility>

namespace tl {

struct source;
struct sink;

struct pullable {
	virtual int pull(source&) = 0;
	virtual ~pullable() = 0 {}
};

struct pushable {
	// TODO: Parameter for whether to request more space. 0 for flush.
	virtual int push(sink&) = 0;
	virtual ~pushable() = 0 {}
};

struct source_buf {
	source_buf() : cur(0), end(0) {}

	source_buf(u8 const* cur_init, u8 const* end_init)
		: cur(cur_init), end(end_init) {}

	source_buf(source_buf&& other) = default;
	source_buf& operator=(source_buf&) = delete;

	usize left() const { return static_cast<usize>(end - cur); }

	u8 unsafe_get() { return *cur++; }

protected:
	u8 const* cur;
	u8 const* end;
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

struct source : source_buf {
	source(source&& other)
	 : src(std::move(other.src)) {
	}

	u8 get_u8_def(u8 def = 0) {
		return check_pull() ? def : unsafe_get();
	}

private:
	int check_pull() {
		return cur != end ? 0 : src->pull(*this);
	}

	std::unique_ptr<pullable> src;
};

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
		owner.beg = owner.cur = vec.end();
		owner.end = vec.cap_end();
		return 0;
	}

	virtual ~pushable_vector() {}

private:
	tl::vector<u8> vec;
};

}

#endif // UUID_49614D7C9472495896A2EF850C345AEA
