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

#endif // UUID_E4C54B1B70704E6FE73DE99BC631C8A4
