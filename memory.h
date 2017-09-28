#ifndef UUID_E4C54B1B70704E6FE73DE99BC631C8A4
#define UUID_E4C54B1B70704E6FE73DE99BC631C8A4

#include "platform.h"
#include "bits.h"
#include <stdint.h>

// Adapted from PortAudio

#if defined(__APPLE__)
#   include <libkern/OSAtomic.h>
	/* Here are the memory barrier functions. Mac OS X only provides
	   full memory barriers, so the three types of barriers are the same,
	   however, these barriers are superior to compiler-based ones. */
#   define TL_FULL_SYNC()  OSMemoryBarrier()
#   define TL_READ_SYNC()  OSMemoryBarrier()
#   define TL_WRITE_SYNC() OSMemoryBarrier()
#elif defined(__GNUC__)
	/* GCC >= 4.1 has built-in intrinsics. We'll use those */
#   if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
#      define TL_FULL_SYNC()  __sync_synchronize()
#      define TL_READ_SYNC()  __sync_synchronize()
#      define TL_WRITE_SYNC() __sync_synchronize()
	/* as a fallback, GCC understands volatile asm and "memory" to mean it
	 * should not reorder memory read/writes */
	/* Note that it is not clear that any compiler actually defines __PPC__,
	 * it can probably be safely removed. */
#   elif defined( __ppc__ ) || defined( __powerpc__) || defined( __PPC__ )
#      define TL_FULL_SYNC()  asm volatile("sync":::"memory")
#      define TL_READ_SYNC()  asm volatile("sync":::"memory")
#      define TL_WRITE_SYNC() asm volatile("sync":::"memory")
#   elif defined( __i386__ ) || defined( __i486__ ) || defined( __i586__ ) || \
		 defined( __i686__ ) || defined( __x86_64__ )
#      define TL_FULL_SYNC()  asm volatile("mfence":::"memory")
#      define TL_READ_SYNC()  asm volatile("lfence":::"memory")
#      define TL_WRITE_SYNC() asm volatile("sfence":::"memory")
#   else
#      ifdef ALLOW_SMP_DANGERS
#         warning Memory barriers not defined on this system or system unknown
#         warning For SMP safety, you should fix this.
#         define TL_FULL_SYNC() (void)0
#         define TL_READ_SYNC() (void)0
#         define TL_WRITE_SYNC() (void)0
#      else
#         error Memory barriers are not defined on this system. You can still compile by defining ALLOW_SMP_DANGERS, but SMP safety will not be guaranteed.
#      endif
#   endif
#elif (_MSC_VER >= 1400) && !defined(_WIN32_WCE)
#   include <intrin.h>
#   pragma intrinsic(_ReadWriteBarrier)
#   pragma intrinsic(_ReadBarrier)
#   pragma intrinsic(_WriteBarrier)
#   define TL_FULL_SYNC()  _ReadWriteBarrier()
#   define TL_READ_SYNC()  _ReadBarrier()
#   define TL_WRITE_SYNC() _WriteBarrier()
#elif defined(_WIN32_WCE)
#   define TL_FULL_SYNC() (void)0
#   define TL_READ_SYNC() (void)0
#   define TL_WRITE_SYNC() (void)0
#elif defined(_MSC_VER) || defined(__BORLANDC__)
#   define TL_FULL_SYNC()  _asm { lock add    [esp], 0 }
#   define TL_READ_SYNC()  _asm { lock add    [esp], 0 }
#   define TL_WRITE_SYNC() _asm { lock add    [esp], 0 }
#else
#   ifdef ALLOW_SMP_DANGERS
#      warning Memory barriers not defined on this system or system unknown
#      warning For SMP safety, you should fix this.
#      define TL_FULL_SYNC() (void)0
#      define TL_READ_SYNC() (void)0
#      define TL_WRITE_SYNC() (void)0
#   else
#      error Memory barriers are not defined on this system. You can still compile by defining ALLOW_SMP_DANGERS, but SMP safety will not be guaranteed.
#   endif
#endif

#if TL_UNALIGNED_ACCESS

TL_INLINE uint32_t tl_read_le32(uint8_t const* p) { return tl_le32(*(uint32_t const*)p); }
TL_INLINE uint32_t tl_read_be32(uint8_t const* p) { return tl_be32(*(uint32_t const*)p); }
TL_INLINE uint16_t tl_read_le16(uint8_t const* p) { return tl_le16(*(uint16_t const*)p); }
TL_INLINE uint16_t tl_read_be16(uint8_t const* p) { return tl_be16(*(uint16_t const*)p); }

TL_INLINE uint32_t tl_read32(uint8_t const* p) { return *(uint32_t const*)p; }
TL_INLINE uint16_t tl_read16(uint8_t const* p) { return *(uint16_t const*)p; }

TL_INLINE void tl_write32(uint8_t* p, uint32_t v) { *(uint32_t*)p = v; }
TL_INLINE void tl_write16(uint8_t* p, uint16_t v) { *(uint16_t*)p = v; }

TL_INLINE void tl_write_le32(uint8_t* p, uint32_t v) { *(uint32_t*)p = tl_le32(v); }
TL_INLINE void tl_write_be32(uint8_t* p, uint32_t v) { *(uint32_t*)p = tl_be32(v); }
TL_INLINE void tl_write_le16(uint8_t* p, uint16_t v) { *(uint16_t*)p = tl_le16(v); }
TL_INLINE void tl_write_be16(uint8_t* p, uint16_t v) { *(uint16_t*)p = tl_be16(v); }


#else

TL_INLINE uint32_t tl_read_le32(uint8_t const* p) { return p[0] + ((uint32_t)p[1] << 8) + ((uint32_t)p[2] << 16) + ((uint32_t)p[3] << 24); }
TL_INLINE uint32_t tl_read_be32(uint8_t const* p) { return p[3] + ((uint32_t)p[2] << 8) + ((uint32_t)p[1] << 16) + ((uint32_t)p[0] << 24); }
TL_INLINE uint16_t tl_read_le16(uint8_t const* p) { return p[0] + ((uint16_t)p[1] << 8); }
TL_INLINE uint16_t tl_read_be16(uint8_t const* p) { return p[1] + ((uint16_t)p[0] << 8); }

// TODO tl_write*

#define tl_read32 tl_read_le32
#define tl_read16 tl_read_le16

#endif

#if TL_CPP

namespace tl {

template<typename T>
struct Unaligned {
	Unaligned() {

	}

	explicit Unaligned(T v_init) {
		this->set(v_init);
	}

	void set(T v_new) {
#if TL_UNALIGNED_ACCESS
		this->v = v_new;
#else
		memcpy(&v, &v_new, sizeof(T));
#endif
	}

	T get() const {
#if TL_UNALIGNED_ACCESS
		return v;
#else
		T r;
		memcpy(&r, &v, sizeof(T));
		return r;
#endif
	}

	T& raw() {
		return this->v;
	}

private:
	T v;
};

inline u8 read_le(u8 const& r) { return r; }
inline u8 read_be(u8 const& r) { return r; }

inline u16 read_le(u16 const& r) { u16 v; memcpy(&v, &r, sizeof(u16)); return tl_le16(v); }
inline u16 read_be(u16 const& r) { u16 v; memcpy(&v, &r, sizeof(u16)); return tl_be16(v); }

inline u32 read_le(u32 const& r) { u32 v; memcpy(&v, &r, sizeof(u32)); return tl_le32(v); }
inline u32 read_be(u32 const& r) { u32 v; memcpy(&v, &r, sizeof(u32)); return tl_be32(v); }

template<typename T>
inline T swap_for_le(T v);

template<> inline u32 swap_for_le<u32>(u32 v) { return tl_le32(v); }
template<> inline i32 swap_for_le<i32>(i32 v) { return tl_le32(v); }
template<> inline u16 swap_for_le<u16>(u16 v) { return tl_le16(v); }
template<> inline i16 swap_for_le<i16>(i16 v) { return tl_le16(v); }
#if TL_PTRSIZE == 64
template<> inline usize swap_for_le<usize>(usize v) { return tl_lesize(v); }
template<> inline isize swap_for_le<isize>(isize v) { return tl_lesize(v); }
#else
template<> inline u64 swap_for_le<u64>(u64 v) { return tl_le64(v); }
template<> inline i64 swap_for_le<i64>(i64 v) { return tl_le64(v); }
#endif

template<typename T>
inline void write_le(void* dest, T v) {
	v = swap_for_le(v);
	memcpy(dest, &v, sizeof(v));
}

template<typename T>
inline T read_le(void const* src) {
	T v;
	memcpy(&v, src, sizeof(v));
	return swap_for_le(v);
}

}

#endif

#endif // UUID_E4C54B1B70704E6FE73DE99BC631C8A4
