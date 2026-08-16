#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

#include "types.h"

/*
 * shamelessly stolen from:
 * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/linux/math.h?id=ddd664bbff63e09e7a7f9acae9c43605d4cf185f#n10
 */
#define __round_mask(x, y) ((__typeof__(x))((y) - 1))
#define round_up(x, y)	   ((((x) - 1) | __round_mask(x, y)) + 1)
#define round_down(x, y)   ((x) & ~__round_mask(x, y))
#define div_round_up(n, d) (((n) + (d) - 1) / (d))

/* compiler attributes */
#define NONNULL(...)   __attribute__((nonnull(__VA_ARGS__)))
#define ALIGNED(align) __attribute__((aligned(align)))
#define SECTION(sec)   __attribute__((section(sec)))
#define MUST_CHECK     __attribute__((warn_unused_result))
#define PACKED	       __attribute__((packed))
#define USED	       __attribute__((used))
#define UNUSED	       __attribute__((unused))
#define NORETURN       __attribute__((noreturn))

#define NO_SANITIZE(rule) __attribute__((no_sanitize(rule)))

#define IS_ALIGNED(value, alignment) ((value) % (alignment) == 0)

#define BIT(n)			(1UL << (n))
#define SET_BIT(value, bit_idx) ((value) | ((__typeof__(value))1 << (bit_idx)))
#define CLEAR_BIT(value, bit_idx) \
	((value) & ~((__typeof__(value))1 << (bit_idx)))
#define GET_BIT(value, bit_idx) (((value) >> (bit_idx)) & 1)
#define EXTRACT_BITS(value, high, low) \
	(((value) >> (low)) &          \
	 (((__typeof__(value))1 << ((high) - (low) + 1)) - 1))

#define UNUSED_ARG(value) (void)(value)

/* compiler intrinsics */
#define UNREACHABLE() __builtin_unreachable()

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define TODO() panic("TODO")

#define _CONCAT2(a, b) a##b
#define CONCAT(a, b)   _CONCAT2(a, b)

NO_SANITIZE("alignment")
USED static inline void *container_of_impl(void *ptr, usize off)
{
	return (char *)ptr - off;
}

#define container_of(ptr, type, member) \
	((type *)container_of_impl((ptr), offsetof(type, member)))

#endif // _COMMON_DEFS_H_
