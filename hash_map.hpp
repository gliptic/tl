#ifndef TL_HASH_MAP_HPP
#define TL_HASH_MAP_HPP 1

#include "std.h"
#include "cstdint.h"
#include <utility>

namespace tl {

using std::move;
	
template<typename S>
struct HashSet {
	HashSet();

	template<typename K>
	S* get(K key);

	template<typename K>
	S remove(K const& key);

	u32 hshift, keycount;
	S* tab;
};

template<typename K, typename V, typename H>
struct HashMapSlot {
	K key;
	V value;

	HashMapSlot(K k, V v)
		: key(move(k)), value(move(v)) {
	}

	static u32 hash(HashMapSlot const& slot) {
		return H::hash(slot.key);
	}

	static u32 hash(K const& k) {
		return H::hash(k);
	}

	bool is_empty() const {
		return key == K();
	}

	void make_empty() {
		key = K();
	}

	u32 hash() const {
		return hash(key);
	}

	bool is_same_key(K const& other) const {
		return key == other;
	}

	bool is_same_key(HashMapSlot const& other) const {
		return key == other.key;
	}
};

template<typename K, typename V, typename H>
struct HashMap : HashSet<HashMapSlot<K, V, H>> {

	HashMap() {

	}
};

static u32 const default_local_set_loglen = 8;

template<typename S>
HashSet<S>::HashSet()
	: hshift(32 - default_local_set_loglen), keycount(0) {

	u32 len = 1 << default_local_set_loglen;

	//this->prof_hprobes = 0;
	//this->prof_getcount = 0;

	this->tab = (S *)calloc(len, sizeof(S));
}

inline u32 hash_key(u64 x) {
	u32 v = (u32)((x >> 32) ^ x);
	return v * (v * 2 + 1);
}

#define HASH2IDX(h) ((h) >> hshift)
#define STAB(x) (tab[x])

template<typename S>
void resize(HashSet<S>& self);

template<typename S>
bool insert(
	HashSet<S>&,
	u32 hash,
	S s,
	u32 hshift, S* tab,
	S** ret) {

	u32 hmask = ((u32)-1) >> hshift;
	u32 p = HASH2IDX(hash);
	u32 dist = 0;

	S* r = 0;

	//++self->prof_getcount;

	for (;;) {

		//++self->prof_hprobes;

		auto* p_slot = &STAB(p);

		if (p_slot->is_same_key(s)) {
			// Exists
			*ret = p_slot;
			return true;
		}

		u32 p_hash = p_slot->hash();
		u32 p_dist = (p - HASH2IDX(p_hash)) & hmask;

		bool was_empty = p_slot->is_empty();
		if (was_empty || dist > p_dist) {
			if (!r) {
				r = p_slot;
			}

			std::swap(*p_slot, s);

			if (was_empty) {
				// Slot was empty. We're done.
				*ret = r;
				return false;
			}

			// Not empty so we need to push it down
			dist = p_dist;
			hash = p_hash;
		}

		p = (p + 1) & hmask;
		++dist;
	}
}

template<typename S>
void resize(HashSet<S>& self) {
	u32 newhshift = self.hshift - 1;
	u32 newloglen = 32 - newhshift;
	u32 newlen = 1 << newloglen;

	S *newtab = (S *)calloc(newlen, sizeof(S));
	S *oldtab = self.tab;

	u32 oldhmask = ((u32)-1) >> self.hshift;

	for (u32 p = 0; p <= oldhmask; ++p) {
		auto& p_slot = oldtab[p];

		if (!p_slot.is_empty()) {
			S* dummy;

			insert(self, p_slot.hash(), move(p_slot), newhshift, newtab, &dummy);
		}
	}

	self.tab = newtab;
	self.hshift = newhshift;
	free(oldtab);
}

template<typename S>
template<typename K>
S* HashSet<S>::get(K key) {
	S* v;

	u32 hmask = ((u32)-1) >> hshift;

	auto hash = S::hash(key);

	// TODO: Proper conversion key -> slot
	if (!insert(*this, hash, S(key, 0), this->hshift, this->tab, &v)) {
		if (++this->keycount > hmask - (hmask >> 2)) {
			resize(*this);
			// We do this to find the new slot pointer
			// TODO: Use simple get instead of insert here
			insert(*this, hash, S(key, 0), this->hshift, this->tab, &v);
		}
	}

	return v;
}

template<typename S>
template<typename K>
S HashSet<S>::remove(K const& key) {
	u32 hash = S::hash(key);
	u32 hmask = ((u32)-1) >> hshift;
	u32 p = HASH2IDX(hash);
	u32 dist = 0;

	//++self->prof_getcount;

	for (;;) {

		//++self->prof_hprobes;

		auto *p_slot = &STAB(p);

		if (p_slot->is_same_key(key)) {
			S value = move(*p_slot);
			for (;;) {
				u32 prev_p = p;
				p = (p + 1) & hmask;
				++dist;

				p_slot = &STAB(p);
				u32 p_hash = S::hash(*p_slot);

				if (p_slot->is_empty() || p == HASH2IDX(p_hash)) {
					// Chain is bridged. Just make sure previous slot is empty.
					STAB(prev_p).make_empty();
					--this->keycount;
					return move(value);
				}

				STAB(prev_p) = move(*p_slot);
			}
		}

#if 0
		// TODO: This should never happen, so we should skip it.
		// However, the performance impact seems to be insignificant.
		u32 p_hash = S::hash(*p_slot);
		u32 p_dist = (p - HASH2IDX(p_hash)) & hmask;

		if (p_slot->is_empty() || dist > p_dist) {
			return S();
		}
#endif

		p = (p + 1) & hmask;
		++dist;
	}
}

#undef HASH2IDX
#undef STAB

}

#endif // !TL_HASH_MAP_HPP

