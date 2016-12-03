#ifndef TL_VEC_HPP
#define TL_VEC_HPP

#include "std.h"
#include <stddef.h>
#include <algorithm>
#include <utility>

#include "cstdint.h"
#include "platform.h"

using std::move;

namespace tl {


struct VecSliceAbstract {
private:
	u8 *b;
	u8 *e;
public:

	VecSliceAbstract(VecSliceAbstract const&) = default;
	VecSliceAbstract& operator=(VecSliceAbstract const&) = default;
	
	VecSliceAbstract() : b(0), e(0) {
	}

	VecSliceAbstract(u8* b_init, u8* e_init) : b(b_init), e(e_init) {
	}

	VecSliceAbstract(VecSliceAbstract&& other)
		: b(other.b), e(other.e) {
		other.b = other.e = 0;
	}

	VecSliceAbstract& operator=(VecSliceAbstract&& other) {
		this->b = other.b;
		this->e = other.e;
		other.b = other.e = 0;
		return *this;
	}

	u8* unsafe_inc_size_bytes(usize amount) {
		u8* old_e = this->e;
		this->e += amount;
		return old_e;
	}

	void unsafe_cut_front_bytes(usize amount) {
		this->b += amount;
	}

	void unsafe_cut_back_bytes(usize amount) {
		this->e -= amount;
	}

	void unsafe_set_size_bytes(usize new_size) {
		this->e = this->b + new_size;
	}

	u8** end_bytes_ptr() { return &this->e; }
	u8** begin_bytes_ptr() { return &this->b; }

	u8* end_bytes() const { return this->e; }
	u8* begin_bytes() const { return this->b; }
	
	usize size_bytes() const { return this->e - this->b; }
	bool empty() const { return this->b == this->e; }
};

template<typename T>
struct VecSlice : protected VecSliceAbstract {
	using VecSliceAbstract::size_bytes;
	using VecSliceAbstract::empty;

	VecSlice() = default;
	VecSlice(VecSlice const&) = default;
	VecSlice& operator=(VecSlice const&) = default;

	template<usize S>
	VecSlice(T (&arr)[S])
		: VecSliceAbstract((u8 *)arr, (u8 *)(arr + S)) {
	}

	VecSlice(T* b_init, T* e_init)
		: VecSliceAbstract((u8 *)b_init, (u8 *)e_init) {
	}
	
	T& operator[](usize index) const {
		return this->begin()[index];
	}

	T* begin() const { return (T *)this->begin_bytes(); }
	T* end() const { return (T *)this->end_bytes(); }

	usize size() const { return this->end() - this->begin(); }

	void unsafe_cut_front(usize amount) {
		this->unsafe_cut_front_bytes(amount * sizeof(T));
	}

	void unsafe_cut_back(usize amount) {
		this->unsafe_cut_back_bytes(amount * sizeof(T));
	}

	T& unsafe_pop_front() {
		T& r = *this->begin();
		this->unsafe_cut_front_bytes(sizeof(T));
		return r;
	}

	T& unsafe_pop_back() {
		this->unsafe_cut_back_bytes(sizeof(T));
		return *this->end();
	}

	template<typename U>
	U unsafe_get_le() {
		u8 const* p = this->begin();
		U r = tl::read_le<U>(p);
		this->unsafe_cut_front(sizeof(U));
		return r;
	}
};

#if 1

#pragma region VecAbstract
struct VecAbstract : protected VecSliceAbstract {

	u8 *c;

	VecAbstract() : c(0) {}

	VecAbstract(VecAbstract&& other)
		: VecSliceAbstract(move(other)), c(other.c) {
		other.c = 0;
	}

	VecAbstract& operator=(VecAbstract&& other) {
		VecSliceAbstract::operator=(move(other));
		this->c = other.c;
		other.c = 0;
		return *this;
	}

	template<typename T>
	VecSlice<T> slice() const {
		VecSliceAbstract const& base = *this;
		return static_cast<VecSlice<T> const&>(base);
	}

	u8* cap_end_bytes() { return this->c; }
	
	void reserve_bytes(usize amount);
	TL_NEVER_INLINE void enlarge(usize extra);

	~VecAbstract() {
		// TODO: VC++ doesn't know that it doesn't have to
		// free pointers known to be NULL. Having this check
		// makes it omit the free call for e.g. moved vectors,
		// but it will unnecessarily do the check in other cases.
		if (this->begin_bytes())
			free(this->begin_bytes());
	}
};

#pragma endregion
#else

#pragma region VecAbstract
struct VecAbstract {

	u8 *b;
	u8 *c;
	usize unused;

	VecAbstract() : b(0), c(0), unused(0) {}

	VecAbstract(VecAbstract&& other)
		: b(other.b), c(other.c), unused(other.unused) {
		other.b = 0;
		other.c = 0;
		other.unused = 0;
	}

	VecAbstract& operator=(VecAbstract&& other) {
		this->b = other.b;
		this->c = other.c;
		this->unused = other.unused;
		other.b = 0;
		other.c = 0;
		other.unused = 0;
		return *this;
	}

	template<typename T>
	VecSlice<T> slice() const {
		return VecSlice<T>((T *)this->b, (T *)(this->c - this->unused));
	}

	usize size_bytes() const { return this->c - this->b - this->unused; }

	u8* begin_bytes() const { return this->b; }
	u8* end_bytes() const { return this->c - this->unused; }
	u8* cap_end_bytes() const { return this->c; }
	bool empty() const { return size_bytes() == 0; }

	void unsafe_set_size_bytes(usize new_size) {
		this->unused = this->c - this->b - new_size;
	}

	u8* unsafe_inc_size_bytes(usize amount) {
		u8* old_e = this->c - this->unused;
		this->unused += amount;
		return old_e;
	}

	void unsafe_cut_front_bytes(usize amount) {
		this->b += amount;
	}

	void unsafe_cut_back_bytes(usize amount) {
		this->unused += amount;
	}

	void reserve_bytes(usize amount);
	TL_NEVER_INLINE void enlarge(usize extra);

	~VecAbstract() { free(this->begin_bytes()); }
};

#pragma endregion
#endif

#pragma region VecInlineAbstract
template<usize InlineSize = 128>
struct VecInlineAbstract : protected VecSliceAbstract {
	u8 *c;
	u8 inline_buffer[InlineSize];

	VecInlineAbstract()
		: VecSliceAbstract(inline_buffer, inline_buffer)
		, c(inline_buffer + InlineSize) {
	}

	// Not so simple to move inline vecs
	VecInlineAbstract(VecAbstract&& other) = delete;
	VecInlineAbstract& operator=(VecAbstract&& other) = delete;

	template<typename T>
	VecSlice<T> slice() const {
		VecSliceAbstract const& base = *this;
		return static_cast<VecSlice<T> const&>(base);
	}

	u8* cap_end_bytes() { return this->c; }

	TL_NEVER_INLINE void enlarge(usize extra);
	void reserve_bytes(usize new_cap);

	~VecInlineAbstract() {
		u8* beg = this->begin_bytes();
		if (beg != inline_buffer)
			free(beg);
	}
};

template<usize InlineSize>
void VecInlineAbstract<InlineSize>::enlarge(usize extra) {
	usize size = this->size_bytes();
	usize new_cap = 2 * size + extra;

	this->reserve_bytes(new_cap);
}

template<usize InlineSize>
void VecInlineAbstract<InlineSize>::reserve_bytes(usize new_cap) {

	u8* old_b = this->begin_bytes();
	u8* cur_b = old_b;

	if (new_cap <= usize(c - old_b))
		return;

	u8* new_b;

	if (old_b == inline_buffer) {
		old_b = 0; // Do not deallocate
	}

	usize size = this->size_bytes();
	
	if ((new_b = (u8 *)realloc(old_b, new_cap)) == 0) {
		free(cur_b);
		new_b = 0;
	} else if (!old_b) {
		// Moving to malloc storage
		memcpy(new_b, cur_b, size);
	}

	this->VecSliceAbstract::operator=(VecSliceAbstract(new_b, new_b + size));
	c = new_b + new_cap;
}
#pragma endregion

#pragma region Vec

template<typename T>
inline usize concat_size_1(T) {
	return 1;
}

template<typename T>
inline usize concat_size_1(tl::VecSlice<T> first) {
	return first.size();
}

template<typename T>
inline usize concat_size_1(tl::VecSlice<T const> first) {
	return first.size();
}

template<typename T>
inline usize concat_size() {
	return 0;
}

template<typename T, typename First, typename... Rest>
usize concat_size(First first, Rest... rest) {
	return concat_size_1<T>(first) + concat_size<T>(rest...);
}

template<typename T>
inline T* concat_write_1(T* dest, T first) {
	*dest = first;
	return dest + 1;
}

template<typename T>
inline T* concat_write_1(T* dest, tl::VecSlice<T> first) {
	memcpy(dest, first.begin(), first.size());
	return dest + first.size();
}

template<typename T>
inline T* concat_write_1(T* dest, tl::VecSlice<T const> first) {
	memcpy(dest, first.begin(), first.size());
	return dest + first.size();
}

template<typename T>
inline T* concat_write(T* dest) {
	return dest;
}

template<typename T, typename First, typename... Rest>
inline T* concat_write(T* dest, First first, Rest... rest) {
	dest = concat_write_1(dest, first);
	return concat_write(dest, rest...);
}

template<typename T, typename Base = VecAbstract>
struct Vec : Base {

	using Base::size_bytes;
	using Base::empty;

	Vec(Vec const&) = delete;
	Vec& operator=(Vec const& other) = delete;

	Vec() = default;
	Vec(Vec&& other) = default;
	Vec& operator=(Vec&& other) = default;

	Vec(VecSlice<T const> other)
		: Vec(other.begin(), other.size()) {
	}

	Vec(T const* src, usize len) {
		usize len_in_bytes = len * sizeof(T);
		this->reserve_bytes(len_in_bytes);
		// TODO: Only do this if the copying is trivial
		memcpy(this->begin(), src, len_in_bytes);
		this->unsafe_inc_size_bytes(len_in_bytes);
	}

	Vec(usize len, T const& value) {
		usize len_in_bytes = len * sizeof(T);
		this->reserve_bytes(len_in_bytes);

		auto* p = this->begin();
		for (usize i = 0; i < len; ++i) {
			new (p + i, tl::non_null()) T(value);
		}
		this->unsafe_inc_size_bytes(len_in_bytes);
	}

	Vec(usize capacity) {
		usize capacity_in_bytes = capacity * sizeof(T);
		this->reserve_bytes(capacity_in_bytes);
	}

	template<typename... Args>
	static Vec concat(Args... args) {
		usize size = concat_size<T>(args...);
		Vec vec(size);

		concat_write<T>(vec.begin(), args...);
		vec.unsafe_inc_size(size);
		return move(vec);
	}

	void push_back(T const& v) {
		if (this->cap_end_bytes() - this->end_bytes() < sizeof(v)) {
			this->enlarge(sizeof(v));
		}
		new (this->unsafe_inc_size_bytes(sizeof(v)), non_null()) T(v);
	}

	void push_back(T&& v) {
		if (this->cap_end_bytes() - this->end_bytes() < sizeof(v)) {
			this->enlarge(sizeof(v));
		}
		new (this->unsafe_inc_size_bytes(sizeof(v)), non_null()) T(move(v));
	}

	// TODO: This name implies there's no bounds check, but there is.
	// The unsafety is in that it's uninitialized. Rename it.
	T* unsafe_alloc(usize count) {
		if (usize(this->cap_end_bytes() - this->end_bytes()) < count * sizeof(T))
			this->enlarge(count * sizeof(T));

		T* p = this->end();
		this->unsafe_inc_size_bytes(count * sizeof(T));
		return p;
	}

	VecSlice<T> slice() {
		return this->Base::template slice<T>();
	}

	VecSlice<T const> slice_const() const {
		return this->Base::template slice<T const>();
	}

	T& operator[](usize index) { return this->slice()[index]; }
	T const& operator[](usize index) const { return this->slice_const()[index]; }

	T* begin() { return this->slice().begin(); }
	T* end() { return this->slice().end(); }
	T const* begin() const { return this->slice_const().begin(); }
	T const* end() const { return this->slice_const().end(); }
	T* cap_end() { return (T *)this->cap_end_bytes(); }
	usize size() const { return this->slice_const().size(); }

	T& back() { return end()[-1]; }

	void reserve(usize new_cap) {
		this->reserve_bytes(new_cap * sizeof(T));
	}

	void clear() {
		destroy_all();
		this->unsafe_set_size_bytes(0);
	}

	T* unsafe_inc_size(usize count) {
		assert(usize(this->cap_end_bytes() - this->end_bytes()) >= count * sizeof(T));
		T* p = this->end();
		this->unsafe_inc_size_bytes(count * sizeof(T));
		return p;
	}

	void unsafe_set_size(usize new_size) {
		assert(usize(this->cap_end() - this->begin()) >= new_size);
		this->unsafe_set_size_bytes(new_size * sizeof(T));
	}

	// TODO: This should destruct the popped value
	void unsafe_pop() {
		this->unsafe_cut_back_bytes(1);
	}

	~Vec() {
		destroy_all();
	}

private:

	void destroy_all() {
		for (T& p : slice()) {
			TL_UNUSED(p); // TODO: VC++ somehow reports p as unused in some cases
			p.~T();
		}
	}
};
#pragma endregion

template<typename T, usize InlineSize = 128>
using VecSmall = Vec<T, VecInlineAbstract<InlineSize>>;

struct BufferMixed : Vec<u8> {

	BufferMixed() = default;
	BufferMixed(BufferMixed&&) = default;

	BufferMixed& operator=(BufferMixed&&) = default;

	template<typename U>
	void unsafe_push(U const& v) {
		u8* p = unsafe_alloc(sizeof(U));
		new (p, non_null()) U(v);
	}


};

}

#endif // TL_VEC_HPP
