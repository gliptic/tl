#ifndef TL_RECT_HPP
#define TL_RECT_HPP

#include "cstdint.h"
#include "vector.hpp"

namespace tl {

template<typename T>
class BasicRect
{
public:
	BasicRect()
	: x1(0), y1(0), x2(0), y2(0)
	{ }

	BasicRect(T x1_, T y1_, T x2_, T y2_)
	: x1(x1_), y1(y1_), x2(x2_), y2(y2_)
	{ }
	
	BasicRect(BasicVector<T, 2> const& pixel)
	: x1(pixel.x), y1(pixel.y)
	, x2(pixel.x + T(1)), y2(pixel.y + T(1))
	{ }
	
	BasicRect(BasicVector<T, 2> const& corner, T size)
	: x1(corner.x), y1(corner.y)
	, x2(corner.x + size), y2(corner.y + size)
	{ }
	
	BasicRect(BasicVector<T, 2> const& corner, T w, T h)
	: x1(corner.x), y1(corner.y)
	, x2(corner.x + w), y2(corner.y + h)
	{ }
	
	BasicRect(BasicVector<T, 2> const& corner1, BasicVector<T, 2> const& corner2) {
		if (corner1.x < corner2.x) {
			x1 = corner1.x;
			x2 = corner2.x + 1;
		} else {
			x1 = corner2.x;
			x2 = corner1.x + 1;
		}
		
		if (corner1.y < corner2.y) {
			y1 = corner1.y;
			y2 = corner2.y + 1;
		} else {
			y1 = corner2.y;
			y2 = corner1.y + 1;
		}
	}
	
	T x1;
	T y1;
	T x2;
	T y2;
	
	T center_x() const { return (x1 + x2) / T(2); }
	T center_y() const { return (y1 + y2) / T(2); }
	
	BasicVector<T, 2> center() const {
		return BasicVector<T, 2>(center_x(), center_y());
	}

	T width() const { return x2 - x1; }
	T height() const { return y2 - y1; }
	
	BasicRect flip() const {
		return BasicRect(y1, x1, y2, x2);
	}

	struct RowMajorRange : private BasicRect {
		T curx;

		RowMajorRange(BasicRect const& r)
			: BasicRect(r), curx(r.x1) {

		}

		bool next(BasicVector<T, 2>& result) {
			if (this->curx >= this->x2) {
				this->curx = this->x1;
				if (this->y1 >= this->y2) {
					return false;
				}
				this->y1 += T(1);
			}

			result = BasicVector<T, 2>(this->curx, this->y1);
			this->curx += T(1);
			return true;
		}

		bool operator!=(RowMajorRange const&) const {
			if (this->y1 >= this->y2) {
				return false;
			}

			return true;
		}

		RowMajorRange& operator++() {
			this->curx += T(1);
			if (this->curx >= this->x2) {
				this->curx = this->x1;
				this->y1 += T(1);
			}

			return *this;
		}

		BasicVector<T, 2> operator*() {
			return BasicVector<T, 2>(this->curx, this->y1);
		}

		RowMajorRange begin() {
			return *this;
		}

		RowMajorRange end() {
			return *this;
		}
	};

	RowMajorRange row_major_range() {
		return RowMajorRange(*this);
	}

	bool valid() {
		return x1 <= x2 && y1 <= y2;
	}
	
	void join(BasicRect const& b) {
		x1 = std::min(b.x1, x1);
		y1 = std::min(b.y1, y1);
		x2 = std::max(b.x2, x2);
		y2 = std::max(b.y2, y2);
	}
		
	bool precise_join(BasicRect const& b) {
		bool ok = false;
		if(x1 == b.x1 && x2 == b.x2) {
			if(b.y2 >= y1
			&& b.y1 <= y2)
				ok = true;
		} else if(y1 == b.y1 && y2 == b.y2)	{
			if(b.x2 >= x1
			&& b.x1 <= x2)
				ok = true;
		} else {
			ok = inside(b) || b.inside(*this);
		}
		
		if(ok)
			join(b);
		return ok;
	}
	
	bool inside(BasicRect const& b) const {
		return x1 <= b.x1 && x2 >= b.x2
		    && y1 <= b.y1 && y2 >= b.y2;
	}

	bool inside(T vx, T vy) const {
		T diffX = vx - x1;
		T diffY = vy - y1;
		
		return diffX < width() && diffX >= T(0)
		    && diffY < height() && diffY >= T(0);
	}
	
	bool join_h(BasicRect const& b) {
		T new_x1 = std::min(b.x1, x1);
		T new_x2 = std::max(b.x2, x2);
		
		bool changed = new_x1 != x1 || new_x2 != x2;
		x1 = new_x1;
		x2 = new_x2;
		
		return changed;
	}
	
	bool join_v(BasicRect const& b) {
		T new_y1 = std::min(b.y1, y1);
		T new_y2 = std::max(b.y2, y2);
		
		bool changed = new_y1 != y1 || new_y2 != y2;
		y1 = new_y1;
		y2 = new_y2;
		
		return changed;
	}

	// Extend *this and b to their maximal size without
	// changing their joint coverage.
	int maximal_extend(BasicRect& b) {
		int change_mask = 0;
		if (intersecting_v(b)) {
			if (encloses_h(b)) {
				// Extend b vertically into *this
				if (b.join_v(*this))
					change_mask |= 2;
			}

			if(b.encloses_h(*this)) {
				// Extend *this vertically into b
				if (join_v(b))
					change_mask |= 1;
			}
		}
		
		if (intersecting_h(b)) {
			if (encloses_v(b)) {
				// Extend b horizontally into *this
				if (b.join_h(*this))
					change_mask |= 2;
			}

			if (b.encloses_v(*this)) {
				// Extend *this horizontally into b
				if (join_h(b))
					change_mask |= 1;
			}
		}
		
		return change_mask;
	}
	
	// Is b inside *this?
	bool encloses(BasicRect const& b) const {
		return encloses_h(b) && encloses_v(b);
	}
	
	// Is the horizontal span of b inside *this?
	bool encloses_h(BasicRect const& b) const {
		return x1 <= b.x1 && x2 >= b.x2;
	}
	
	// Is the vertical span of b inside *this?
	bool encloses_v(BasicRect const& b) const {
		return y1 <= b.y1 && y2 >= b.y2;
	}
	
	bool proper_intersecting(BasicRect const& b) const{
		return (b.y2 > y1
		     && b.y1 < y2
		     && b.x2 > x1
		     && b.x1 < x2);
	}
	
	bool intersecting_h(BasicRect const& b) const {
		return (b.x2 >= x1
		     && b.x1 <= x2);
	}
	
	bool intersecting_v(BasicRect const& b) const {
		return (b.y2 >= y1
		     && b.y1 <= y2);
	}

	// TODO: This isn't really intersecting!
	// Also returns true when the rectangles are merely touching.
	// What to do about that? Added proper_intersecting above for now.
	bool intersecting(BasicRect const& b) const {
		return intersecting_h(b) && intersecting_v(b);
	}

	bool intersect(BasicRect const& b) {
		x1 = std::max(b.x1, x1);
		y1 = std::max(b.y1, y1);
		x2 = std::min(b.x2, x2);
		y2 = std::min(b.y2, y2);
		
		return valid();
	}

	bool encloses(BasicVector<T, 2> v) const {
		return encloses(v.x, v.y);
	}
	
	bool encloses(T x, T y) const {
		T diffX = x - x1;
		T diffY = y - y1;
		
		return diffX < width() && diffX >= T(0)
		    && diffY < height() && diffY >= T(0);
	}
	
	BasicRect operator&(BasicRect const& b) const {
		BasicRect ret(*this); ret &= b; return ret;
	}
	
	BasicRect& operator&=(BasicRect const& b) {
		intersect(b);
		return *this;
	}
	
	BasicRect operator|(BasicRect const& b) const {
		basic_rect ret(*this); ret |= b; return ret;
	}
	
	BasicRect& operator|=(BasicRect const& b) {
		join(b);
		return *this;
	}
	
	BasicRect operator+(BasicVector<T, 2> const& b) {
		basic_rect ret(*this); ret += b; return ret;
	}
	
	BasicRect& operator+=(BasicVector<T, 2> const& b) {
		x1 += b.x; x2 += b.x;
		y1 += b.y; y2 += b.y;
		return *this;
	}
	
	BasicRect operator-(BasicVector<T, 2> const& b) {
		BasicRect ret(*this); ret -= b; return ret;
	}
	
	BasicRect& operator-=(BasicVector<T, 2> const& b)	{
		x1 -= b.x; x2 -= b.x;
		y1 -= b.y; y2 -= b.y;
		return *this;
	}
	
	BasicRect translated(T x, T y) { return *this + BasicVector<T, 2>(x, y); }
	
	void translate_v(T y) { y1 += y; y2 += y; }
	
	void translate_h(T x) { x1 += x; x2 += x; }
	
	BasicVector<T, 2> ul() { return BasicVector<T, 2>(x1, y1); }
	BasicVector<T, 2> ur() { return BasicVector<T, 2>(x2, y1); }
	BasicVector<T, 2> ll() { return BasicVector<T, 2>(x1, y2); }
	BasicVector<T, 2> lr() { return BasicVector<T, 2>(x2, y2); }
	
	bool operator==(BasicRect const& b) const
	{ return x1 == b.x1 && y1 == b.y1 && x2 == b.x2 && y2 == b.y2; }
	
	bool operator!=(BasicRect const& b) const
	{ return !operator==(b); }
};

typedef BasicRect<i32> Rect;
typedef BasicRect<u32> RectU;
typedef BasicRect<f32> RectF;

}

#endif