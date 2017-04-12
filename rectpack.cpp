#include "rectpack.hpp"

#include "std.h"
#include "platform.h"

namespace tl {

RectPack::RectPack(PackedRect const& r)
	: enclosing(r)
	, largest_free(r.width(), r.height())
	, parent(0)
	, occupied(false)
{
	this->ch[0] = 0;
	this->ch[1] = 0;
}

static bool may_fit(RectPack const& self, u32 w, u32 h, bool allow_rotate) {
	return (w <= self.largest_free.x && h <= self.largest_free.y)
	    || (allow_rotate && h <= self.largest_free.x && w <= self.largest_free.y);
}

static bool should_rotate(u32 cw, u32 ch, u32 w, u32 h) {
	if (w > ch || h > cw)
		return false; // Cannot rotate
	else if (h > ch || w > cw)
		return true; // Must rotate

	// Try to counter-act the shape of the container (making the
	// uncovered parts more square).
	return (cw < ch) == (w < h);
}

static void propagate_largest(RectPack& self, u32 w, u32 h);

static void propagate_largest_nonleaf(RectPack& self) {
	RectPack* ch0 = self.ch[0];
	RectPack* ch1 = self.ch[1];

	if (!ch1)
		propagate_largest(self, ch0->largest_free.x, ch0->largest_free.y);
	else if (!ch0)
		propagate_largest(self, ch1->largest_free.x, ch1->largest_free.y);
	else {
		u32 new_width = ch0->largest_free.x;
		u32 new_height = ch0->largest_free.y;
		if (ch1->largest_free.x > new_width)
			new_width = ch1->largest_free.x;
		if (ch1->largest_free.y > new_height)
			new_height = ch1->largest_free.y;

		propagate_largest(self, new_width, new_height);
	}
}

static void propagate_largest(RectPack& self, u32 w, u32 h) {
	self.largest_free.x = w;
	self.largest_free.y = h;
	if (self.parent)
		propagate_largest_nonleaf(*self.parent);
}

static void propagate_largest_(RectPack& self) {
	if (!self.ch[0] && !self.ch[1]) {
		if (self.occupied)
			propagate_largest(self, 0, 0);
		else
			propagate_largest(self, self.enclosing.width(), self.enclosing.height());
	} else {
		propagate_largest_nonleaf(self);
	}
}

static RectPack* rectpack_create(RectPack& parent, u32 x1, u32 y1, u32 x2, u32 y2) {
	RectU r(x1, y1, x2, y2);
	RectPack* self = new RectPack(r);
	self->parent = &parent;
	return self;
}

static PackedRect known_fit(RectPack& self, u32 w, u32 h) {
	assert(w <= self.largest_free.x && h <= self.largest_free.y);
	assert(!self.occupied);
	assert(self.largest_free.x == self.enclosing.width());
	assert(self.largest_free.y == self.enclosing.height());

	self.occupied = true;

	PackedRect ret(
		self.enclosing.x1,
		self.enclosing.y1,
		self.enclosing.x1 + w,
		self.enclosing.y1 + h);
	/*
	self.x1 = self.enclosing.x1;
	self.y1 = self.enclosing.y1;
	self.x2 = self.enclosing.x1 + w;
	self.y2 = self.enclosing.y1 + h;
	*/

	if (w == self.largest_free.x && h == self.largest_free.y) {
		propagate_largest(self, 0, 0);
		return ret;
	}

	{
		u32 enc_w = self.largest_free.x;
		u32 enc_h = self.largest_free.y;
		u32 space_w = (enc_w - w);
		u32 space_h = (enc_h - h);

		bool cut_horiz = space_w < space_h;

		if (cut_horiz) {
			if (w < enc_w)
				self.ch[0] = rectpack_create(self, self.enclosing.x1 + w, self.enclosing.y1, self.enclosing.x2, self.enclosing.y1 + h);
			self.ch[1] = rectpack_create(self, self.enclosing.x1, self.enclosing.y1 + h, self.enclosing.x2, self.enclosing.y2);
		} else {
			if (h < enc_h)
				self.ch[0] = rectpack_create(self, self.enclosing.x1, self.enclosing.y1 + h, self.enclosing.x1 + w, self.enclosing.y2);
			self.ch[1] = rectpack_create(self, self.enclosing.x1 + w, self.enclosing.y1, self.enclosing.x2, self.enclosing.y2);
		}

		propagate_largest_nonleaf(self);
	}

	//return &self;
	return ret;
}

PackedRect RectPack::try_fit(VectorU2 dim, bool allow_rotate) {
	RectPack* ch0 = this->ch[0];
	RectPack* ch1 = this->ch[1];
	if (!may_fit(*this, dim.x, dim.y, allow_rotate))
		return PackedRect();

	if (ch1) {
		if (ch0) {
			PackedRect r = ch0->try_fit(dim, allow_rotate);
			if (!r.empty()) return r;
		}
		return ch1->try_fit(dim, allow_rotate);
	}

	if (this->occupied)
		return PackedRect();

	if (allow_rotate && should_rotate(this->largest_free.x, this->largest_free.y, dim.x, dim.y))
		return known_fit(*this, dim.y, dim.x);
	return known_fit(*this, dim.x, dim.y);
}

void RectPack::remove(PackedRect* r) {
	TL_UNUSED(r);
	assert(!"unimplemented");
}

void RectPack::clear() {
	if (this->ch[0]) { delete this->ch[0]; this->ch[0] = 0; }
	if (this->ch[1]) { delete this->ch[1]; this->ch[1] = 0; }
	// propagate largest_free
	this->occupied = false;
	propagate_largest_(*this);
}

RectPack::~RectPack() {
	this->clear();
}

}