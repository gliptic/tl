#ifndef UUID_412A1D6C55034518F1C26BB9CA854B2C
#define UUID_412A1D6C55034518F1C26BB9CA854B2C

#include "config.h"
#include "rect.hpp"
#include "vector.hpp"

namespace tl {

typedef RectU PackedRect;

struct RectPack /* : PackedRect */ {
	PackedRect enclosing;
	VectorU2 largest_free;
	RectPack* parent;
	RectPack* ch[2];
	bool occupied;

	RectPack(PackedRect const& r);
	~RectPack();
	PackedRect try_fit(VectorU2 dim, bool allow_rotate);
	void remove(PackedRect* rect);
	void clear();
};

}

#endif // UUID_412A1D6C55034518F1C26BB9CA854B2C
