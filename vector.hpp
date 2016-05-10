#ifndef UUID_C8B889F6B0254CE3F7AE58B920794A20
#define UUID_C8B889F6B0254CE3F7AE58B920794A20

#include "std.h"
#include <stddef.h>
#include <algorithm>
#include <utility>

#include "cstdint.h"
#include "platform.h"

using std::move;

namespace tl {

template<typename T>
struct vector_slice {
	T *b, *e;

	vector_slice()
		: b(0), e(0) {
	}

	vector_slice(T* b_init, T* e_init)
		: b(b_init), e(e_init) {
	}

	T* begin() {
		return this->b;
	}

	T* end() {
		return this->e;
	}

	usize size() const {
		return this->e - this->b;
	}

	usize size_in_bytes() const {
		return (u8 const *)this->e - (u8 const *)this->b;
	}

	bool empty() const {
		return this->b == this->e;
	}

	vector_slice<T> unsafe_cut_front_in_bytes(usize bytes) {
		return vector_slice((T *)((u8 *)this->b + bytes), this->e);
	}

	vector_slice<T> unsafe_limit_size_in_bytes(usize bytes) {
		return vector_slice(this->b, (T *)((u8 *)this->b + bytes));
	}

	template<typename U>
	U* unsafe_read() {
		TL_STATIC_ASSERT((sizeof(U) / sizeof(T)) * sizeof(T) == sizeof(U));

		if (size_in_bytes() >= sizeof(U)) {
			U *cur = (U *)this->b;
			this->b = (T *)((u8 *)this->b + sizeof(U));
			return cur;
		} else {
			return 0;
		}
	}
};

struct memalloc_allocator {
	template<typename T>
	void alloc_empty(T& slice) {
	}

	template<typename T>
	void* alloc_slice(T& slice, usize cap_in_bytes) {
		return memalloc(cap_in_bytes);
	}

	template<typename T>
	void* realloc_slice(T& slice, usize new_cap_in_bytes, usize old_cap_in_bytes) {
		void* new_b;

		usize size_bytes = slice.size_in_bytes();

		if (new_cap_in_bytes < size_bytes
		 || !(new_b = memrealloc(slice.b, new_cap_in_bytes, old_cap_in_bytes))) {
			memfree(slice.b);
			new_b = 0;
		}

		return new_b;
	}

	template<typename T>
	void free_slice(T& slice) {
		memfree(slice.b);
	}
};

/*
template<typename T, usize InlineSize>
struct small_vector;
*/

template<typename T, typename Allocator = memalloc_allocator> // , usize InlineSize = (64 + sizeof(T) - 1) / sizeof(T)
struct vector : protected vector_slice<T>, private Allocator {

	T *c;

	using vector_slice::begin;
	using vector_slice::end;
	using vector_slice::size;
	using vector_slice::empty;
	using vector_slice::size_in_bytes;

	vector()
		: c(0) {

		this->Allocator::alloc_empty(static_cast<vector_slice<T>&>(*this));
	}

	// TODO: How to limit this to T with trivial copy?
	vector(T const* src, usize len) {
		usize len_in_bytes = len * sizeof(T);
		this->b = (T *)this->Allocator::alloc_slice(static_cast<vector_slice<T>&>(*this), len_in_bytes);
		memcpy(this->b, src, len_in_bytes);
		this->c = this->e = (T *)((u8 *)this->b + len_in_bytes);
	}

	vector(vector&& other)
		: vector_slice(other.b, other.e), c(other.c) {

		other.b = other.e = other.c = 0;
	}

	vector_slice slice() {
		return *this;
	}

	vector& operator=(vector&& other) {

		this->b = other.b;
		this->e = other.e;
		this->c = other.c;

		other.b = other.e = other.c = 0;
		return *this;
	}

	void push_back(T const& value) {
		if(this->cap_left_in_bytes() < sizeof(T))
			enlarge(sizeof(T));
		new (this->e) T(value);
		++this->e;
	}

	void push_back(T&& value) {
		if(this->cap_left_in_bytes() < sizeof(T))
			enlarge(sizeof(T));
		new (this->e) T(move(value));
		++this->e;
	}

	T* cap_end() {
		return this->c;
	}

	usize cap_in_bytes() const {
		return (u8 const *)this->c - (u8 const *)this->b;
	}

	usize cap_left_in_bytes() const {
		return (u8 const *)this->c - (u8 const *)this->e;
	}
	
	void reserve(usize new_cap) {
		if(new_cap * sizeof(T) > cap_in_bytes()) {
			reserve_bytes(new_cap * sizeof(T));
		}
	}

	void reserve_in_bytes(usize new_cap_in_bytes) {
		if(new_cap_in_bytes > cap_in_bytes()) {
			reserve_bytes(new_cap_in_bytes);
		}
	}

	void reserve_bytes(usize new_cap_in_bytes) {
		usize size_bytes = size_in_bytes();

		//printf("new cap: %d\n", new_cap_in_bytes);

		T *new_b = (T *)this->Allocator::realloc_slice(static_cast<vector_slice<T>&>(*this), new_cap_in_bytes, cap_in_bytes());
		
		this->b = new_b;
		this->e = (T *)((u8 *)new_b + size_bytes);
		this->c = (T *)((u8 *)new_b + new_cap_in_bytes);
	}

	void enlarge(usize extra_in_bytes) {
		reserve_in_bytes(size_in_bytes() * 2 + extra_in_bytes);
	}

	void unsafe_set_size(usize new_size) {
		this->e = this->b + new_size;
	}

	void unsafe_set(T* b, T* e, T* c) {
		this->b = b;
		this->e = e;
		this->c = c;
	}

	void clear() {
		destroy_all();
		this->e = this->b;
	}

	~vector() {
		destroy_all();
		this->Allocator::free_slice(static_cast<vector_slice<T>&>(*this));
	}

private:

	void destroy_all() {
		for (T* p = this->b; p != this->e; ++p) {
			p->~T();
		}
	}
};

struct mixed_buffer : tl::vector<u8> {
	mixed_buffer() {
	}

	mixed_buffer(mixed_buffer&& other)
		: vector(move(other)) {
	}

	template<typename U>
	void unsafe_push(U const& v) {
		if(this->cap_left_in_bytes() < sizeof(U))
			enlarge(sizeof(U));
		new (this->e) U(v);
		this->e += sizeof(U);
	}

	u8* unsafe_alloc(usize count) {
		if(this->cap_left_in_bytes() < count)
			enlarge(count);

		u8* p = end();
		this->e += count;
		return p;
	}

	vector& operator=(mixed_buffer&& other) {
		return vector::operator=(move(other));
	}
};

template<typename T, usize InlineSize>
struct small_vector;

template<typename ElemT, usize InlineSize>
struct inline_allocator : protected memalloc_allocator {
	static usize const inline_size_in_bytes = (InlineSize + sizeof(ElemT) - 1) / sizeof(ElemT) * sizeof(ElemT);

	template<typename T>
	void alloc_empty(T& slice) {
		auto& inline_slice = static_cast<small_vector<ElemT, InlineSize>&>(slice);

		inline_slice.unsafe_set(
			(ElemT *)inline_slice.inline_data,
			(ElemT *)inline_slice.inline_data,
			(ElemT *)(inline_slice.inline_data + inline_size_in_bytes));
	}

	template<typename T>
	void* alloc_slice(T& slice, usize cap_in_bytes) {
		if (cap_in_bytes <= inline_size_in_bytes) {
			auto& inline_slice = static_cast<small_vector<ElemT, InlineSize>&>(slice);
			return inline_slice.inline_data;
		}

		return memalloc(cap_in_bytes);
	}

	template<typename T>
	void* realloc_slice(T& slice, usize new_cap_in_bytes, usize old_cap_in_bytes) {
		void* new_b;

		usize size_bytes = slice.size_in_bytes();
		void* old_b = slice.b;
		auto& inline_slice = static_cast<small_vector<ElemT, InlineSize>&>(slice);

		if (old_b == inline_slice.inline_data) {
			old_b = NULL;
		}

		if (new_cap_in_bytes < size_bytes
			|| !(new_b = memrealloc(old_b, new_cap_in_bytes, old_cap_in_bytes))) {
			memfree(old_b);
			new_b = 0;
		} else if (!old_b) {
			memcpy(new_b, slice.b, size_bytes);
		}

		return new_b;
	}

	template<typename T>
	void free_slice(T& slice) {
		auto& inline_slice = static_cast<small_vector<ElemT, InlineSize>&>(slice);

		if ((u8 *)slice.b != inline_slice.inline_data) {
			memfree(slice.b);
		}
	}
};

template<typename T, usize InlineSize = 128>
struct small_vector : tl::vector<T, inline_allocator<T, InlineSize> > {

	small_vector() {

	}

	union {
		T inline_data_[(InlineSize + sizeof(T) - 1) / sizeof(T)];
		u8 inline_data[];
	};
};

}

#endif // UUID_C8B889F6B0254CE3F7AE58B920794A20
