#ifndef TL_RESULT_HPP
#define TL_RESULT_HPP 1

#include <utility>
using std::move;

namespace tl {

struct Unit {};
constexpr Unit unit = Unit();

struct from_ok {};
struct from_error {};

template<typename O, typename E>
struct Storage {
	union U {
		U() {}
		U(O&& o, from_ok) : _ok(move(o)) {}
		U(E&& e, from_error) : _error(move(e)) {}
		~U() {}

		O _ok;
		E _error;
	} v;

	Storage() {}
	Storage(Storage const& other) {
		memcpy(&v, &other.v, sizeof(U));
	}

	Storage(O&& o, from_ok) : v(move(o), from_ok()) { }
	Storage(E&& e, from_error) : v(move(e), from_error()) { }

	~Storage() {}

	O& get_ok() { return v._ok; }
	E& get_error() { return v._error; }
};

template<typename O>
struct Storage<O, Unit> {
	union { O _ok; };

	Storage() {}
	Storage(Unit) {}
	Storage(O&& e) : _ok(move(e)) { }

	O& get_ok() { return _ok; }
	Unit get_error() { return Unit(); }
};

template<typename E>
struct Storage<Unit, E> {
	union { E _error; };

	Storage() {}
	Storage(Unit) {}
	Storage(E&& e) : _error(move(e)) { }

	Unit get_ok() { return Unit(); }
	E& get_error() { return _error; }
};

template<>
struct Storage<Unit, Unit> {
	Storage() {}
	Storage(Unit) {}

	Unit get_ok() { return Unit(); }
	Unit get_error() { return Unit(); }
};

template<typename O, typename E>
struct Result : protected Storage<O, E> {
	enum State : u8 {
#if RESULT_UNINIT
		Uninit,
#endif
		Ok,
		Error
	};

	State state;

#if RESULT_UNINIT
	Result()
		: state(Uninit) {
	}
#endif

	Result(Result&& other)
		: Storage(move((Storage<O, E> &&)other))
		, state(other.state) {
	}

	Result(Storage<O, E>&& storage, State state_init)
		: Storage(move(storage))
		, state(state_init) {
	}

	~Result() {
		if (state == Ok) {
			this->get_ok().~O();
#if RESULT_UNINIT
		} else if (state == Error) {
#else
		} else {
#endif
			this->get_error().~E();
		}
	}

	template<typename F>
	auto map(F f) {
		using Ret = Result<decltype(f(move(this->get_ok()))), E>;

		if (state == Ok) {
			auto r(Ret::ok(f(move(this->get_ok()))));
#if RESULT_UNINIT
			this->state = Uninit;
#endif
			return move(r);
#if RESULT_UNINIT
		} else if (state == Error) {
#else
		} else {
#endif
			auto r(Ret::error(move(this->get_error())));
#if RESULT_UNINIT
			this->state = Uninit;
#endif
			return move(r);
#if RESULT_UNINIT
		} else {
			return Ret();
#endif
		}
	}

	template<typename F>
	auto flat_map(F f) {
		using Ret = decltype(f(move(this->get_ok())));

		if (state == Ok) {
			auto r(f(move(this->get_ok())));
#if RESULT_UNINIT
			this->state = Uninit;
#endif
			return move(r);
#if RESULT_UNINIT
		} else if (state == Error) {
#else
		} else {
#endif
			auto r(Ret::error(move(this->get_error())));
#if RESULT_UNINIT
			this->state = Uninit;
#endif
			return move(r);
#if RESULT_UNINIT
		} else {
			return Ret();
#endif
		}
	}

	O&& unwrap() {
		if (state == Ok) {
#if RESULT_UNINIT
			this->state = Uninit;
#endif
			return move(this->get_ok());
		} else {
			abort();
		}
	}

	static Result ok(O&& o) {
		return Result(Storage<O, E>(move(o), from_ok()), Ok);
	}

	static Result error(E&& e) {
		return Result(Storage<O, E>(move(e), from_error()), Error);
	}

private:
	Result(State state_init)
		: state(state_init) {
	}
};

}

#endif // TL_RESULT_HPP

