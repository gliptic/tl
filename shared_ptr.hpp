#ifndef TL_SHARED_PTR_HPP
#define TL_SHARED_PTR_HPP

#include "std.h"
#include <algorithm>
#include <utility>

namespace tl {

struct RcNode {

	RcNode()
	: _ref_count(1)	{
	}
	
	// const to allow Rc<T const>
	void add_ref() const {
		++_ref_count;
	}
	
	// const to allow Rc<T const>
	void release() const {
		if (--_ref_count == 0) {
			_delete();
		}
	}
	
	void swap(RcNode& b) {
		std::swap(_ref_count, b._ref_count);
	}

	i32 ref_count() const { return _ref_count; }

	virtual ~RcNode() {	}
	
private:
	void _delete() const {
		delete this;
	}
	
	mutable i32 _ref_count; // You should be able to have Rc<T const>
};

struct SharedOwnership {};

template<typename T>
struct Rc {
	Rc()
	: v(0)
	{ }
	
	// Takes ownership, v_init assumed fresh (no add_ref!)
	explicit Rc(T* v_init)
	: v(v_init)	{
	}
	
	// Shares ownership
	explicit Rc(T* v_init, SharedOwnership) {
		_set(v_init);
	}
	
	~Rc() {
		_release();
	}
	
	/*
	explicit Rc(Rc const& b) {
		_set(b.get());
	}

	template<typename SrcT>
	explicit Rc(Rc<SrcT> const& b) {
		T* p = b.get();
		_set(p);
	}
	*/

	Rc(Rc&& b) {
		v = b.get();
		b.v = 0;
	}

	template<typename SrcT>
	friend struct Rc;

	template<typename SrcT>
	Rc(Rc<SrcT>&& b) {
		T* p = b.get();
		v = p;
		b.v = 0;
	}

	Rc clone() const {
		return Rc(v, SharedOwnership());
	}

	/*
	Rc& operator=(Rc const& b) {
		_reset_shared(b.get());
		return *this;
	}

	template<typename SrcT>
	Rc& operator=(Rc<SrcT> const& b) {
		T* p = b.get();
		_reset_shared(p);
		return *this;
	}*/

	Rc& operator=(Rc&& b) {
		_reset_fresh(b.get());
		b.v = 0;
		return *this;
	}

	template<typename SrcT>
	Rc& operator=(Rc<SrcT>&& b) {
		T* p = b.get();
		_reset_fresh(p);
		b.v = 0;
		return *this;
	}
		
	operator void const*() const { return v; }
	
	T* operator->() const { assert(v); return v; }
	T& operator*() const { assert(v); return *v; }
	
	// Takes ownership, v_new assumed fresh (no add_ref!)
	void reset(T* v_new) {
		_reset(v_new);
	}

	void reset(T* v_new, SharedOwnership) {
		_reset_shared(v_new);
	}
	
	void reset() {
		_release();
		this->v = 0;
	}
	
	Rc release() {
		Rc ret(v);
		this->v = 0;
		return std::move(ret);
	}
	
	void swap(Rc& b) { std::swap(v, b.v); }
	
	template<typename DestT>
	Rc<DestT> static_cast_()
	{ return Rc<DestT>(static_cast<DestT*>(get()), SharedOwnership());	}
	
	T& cow() {
		assert(v);
		if(v->ref_count() > 1)
			reset(get()->clone());
		return *get();
	}
	
	T* get() const {
		return v;
	}
	
private:
	// Takes ownership (no add_ref!)
	void _reset(T* v_new) {
		assert(v_new != v); // self-reset is invalid
		_release();
		v = v_new;
	}

	void _reset_fresh(T* v_new) {
		T* old = v;
		v = v_new;
		if(old)
			old->release();
	}
	
	// Shares ownership
	void _reset_shared(T* v_new) {
		T* old = v;
		_set(v_new);
		if(old)
			old->release();
	}
	
	void _release() {
		if (v)
			v->release();
	}
	
	void _set(T* v_new) {
		v = v_new;
		if (v)
			v->add_ref();
	}
	
	void _set_non_zero(T* v_new) {
		v = v_new;
		assert(v);
		v->add_ref();
	}
	
	T* v;
};

}

#endif // TL_SHARED_PTR_HPP

