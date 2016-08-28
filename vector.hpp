#ifndef UUID_604BD56BB2F2436A4CC319ACA78603F3
#define UUID_604BD56BB2F2436A4CC319ACA78603F3

#include "platform.h"
#include "cstdint.h"
#include <cmath>

namespace tl {

// Only N == 2 supported at the moment

template<typename T, int N>
struct BasicVector
{
};

template<typename T>
struct BasicVector<T, 2>
{
	BasicVector()
		: x(T()), y(T())
	{ }

	template<class T2>
	explicit BasicVector(BasicVector<T2, 2> const& b)
		: x(static_cast<T>(b.x)), y(static_cast<T>(b.y))
	{ }

	/// Vector from a to b.
	///
	BasicVector(BasicVector const& a, BasicVector const& b)
		: x(b.x - a.x), y(b.y - a.y)
	{ }

	BasicVector(T const& x_, T const& y_)
		: x(x_), y(y_)
	{ }

	template<class T2>
	BasicVector(T2 const& x, T2 const& y)
		: x(static_cast<T>(x)), y(static_cast<T>(y))
	{ }

	void zero()
	{
		x = T(); y = T();
	}

	BasicVector& operator += (BasicVector const& rhs)
	{
		x += rhs.x; y += rhs.y;
		return *this;
	}

	BasicVector& operator -= (BasicVector const& rhs)
	{
		x -= rhs.x; y -= rhs.y;
		return *this;
	}

	template<typename U>
	BasicVector& operator *= (U rhs)
	{
		x *= rhs; y *= rhs;
		return *this;
	}

	template<typename U>
	BasicVector& operator /= (U rhs)
	{
		x /= rhs; y /= rhs;
		return *this;
	}

	friend BasicVector operator - (BasicVector const& self)
	{
		return BasicVector(-self.x, -self.y);
	}

	BasicVector half() const
	{
		return BasicVector(x / T(2), y / T(2));
	}

	template<typename U>
	BasicVector<U, 2> cast() const {
		return BasicVector<U, 2>(U(x), U(y));
	}

	T x;
	T y;

};

// Operations

template<typename T>
inline BasicVector<T, 2> operator - (BasicVector<T, 2> const& lhs, BasicVector<T, 2> const& rhs)
{
	BasicVector<T, 2> ret(lhs); ret -= rhs; return ret;
}

template<typename T>
inline BasicVector<T, 2> operator + (BasicVector<T, 2> const& lhs, BasicVector<T, 2> const& rhs)
{
	BasicVector<T, 2> ret(lhs); ret += rhs; return ret;
}

template<typename T>
inline BasicVector<T, 2> operator * (BasicVector<T, 2> const& lhs, BasicVector<T, 2> const& rhs)
{
	BasicVector<T, 2> ret(lhs); ret *= rhs; return ret;
}

template<typename T, typename U>
inline BasicVector<T, 2> operator * (BasicVector<T, 2> const& lhs, U rhs)
{
	BasicVector<T, 2> ret(lhs); ret *= rhs; return ret;
}

template<typename T, typename U>
inline BasicVector<T, 2> operator * (U lhs, BasicVector<T, 2> const& rhs)
{
	BasicVector<T, 2> ret(rhs); ret *= lhs; return ret;
}

template<typename T, typename U>
inline BasicVector<T, 2> operator / (BasicVector<T, 2> const& lhs, U rhs)
{
	BasicVector<T, 2> ret(lhs); ret /= rhs; return ret;
}

template<typename T>
inline bool operator==(BasicVector<T, 2> a, BasicVector<T, 2> b)
{
	return a.x == b.x && a.y == b.y;
}

template<typename T>
inline bool operator!=(BasicVector<T, 2> a, BasicVector<T, 2> b)
{
	return !(a == b);
}

template<typename T>
inline BasicVector<T, 2> cross(T a, BasicVector<T, 2> b)
{
	BasicVector<T, 2> ret(-a * b.y, a * b.x);  return ret;
}

template<typename T>
inline BasicVector<T, 2> cross(BasicVector<T, 2> a, T b)
{
	BasicVector<T, 2> ret(b * a.y, -b * a.x);  return ret;
}

template<typename T>
inline T cross(BasicVector<T, 2> a, BasicVector<T, 2> b)
{
	return a.x * b.y - a.y * b.x;
}

template<typename T>
inline T dot(BasicVector<T, 2> a, BasicVector<T, 2> b)
{
	return a.x * b.x + a.y * b.y;
}

template<typename T>
inline BasicVector<T, 2> perp(BasicVector<T, 2> self)
{
	BasicVector<T, 2> ret(-self.y, self.x);  return ret;
}

// Rotate a using b
template<typename T>
inline BasicVector<T, 2> rotate(BasicVector<T, 2> a, BasicVector<T, 2> b)
{
	BasicVector<T, 2> ret(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
	return ret;
}

// Unrotate a using b
template<typename T>
inline BasicVector<T, 2> unrotate(BasicVector<T, 2> a, BasicVector<T, 2> b)
{
	BasicVector<T, 2> ret(a.x * b.x + a.y * b.y, a.y * b.x - a.x * b.y);
	return ret;
}

template<typename T>
inline T length_sqr(BasicVector<T, 2> self) {
	return self.x*self.x + self.y*self.y;
}

template<typename T>
inline T length(BasicVector<T, 2> self) {
	using std::sqrt; // To allow overloading
	return sqrt(length_sqr(self));
}

template<typename T2, typename T>
inline T2 length_convert(BasicVector<T, 2> self) {
	using std::sqrt; // To allow overloading
	return sqrt(T2(length_sqr(self)));
}

template<typename T>
inline BasicVector<T, 2> normal(BasicVector<T, 2> self) {
	T invLength = T(1) / length(self);
	BasicVector<T, 2> ret(self.x*invLength, self.y*invLength);
	return ret;
}

// Uses reciprocal to avoid one division, thus not as accurate as it
// could be. Use normal_accurate to get a more accurate result.
template<typename T2, typename T>
inline BasicVector<T2, 2> normal_convert(BasicVector<T, 2> self) {
	T2 invLength = T2(1) / length_convert<T2>(self);
	BasicVector<T2, 2> ret(T2(self.x)*invLength, T2(self.y)*invLength);
	return ret;
}

template<typename T>
inline BasicVector<T, 2> normal_accurate(BasicVector<T, 2> self) {
	T len = length(self);
	BasicVector<T, 2> ret(self.x / len, self.y / len);
	return ret;
}

// Common aliases
typedef BasicVector<float, 2> VectorF2;
typedef BasicVector<f64, 2> VectorD2;
typedef BasicVector<i32, 2> VectorI2;
typedef BasicVector<u32, 2> VectorU2;

} // namespace tl

#endif // UUID_604BD56BB2F2436A4CC319ACA78603F3
