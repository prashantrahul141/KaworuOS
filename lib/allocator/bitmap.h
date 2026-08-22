/*
 * A bitmap allocator which also requires users of it to remember the size of
 * allocations, so as to support ranged allocations.
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "error.h"
#include "types.h"

#define SIZE_TO_BITMAP_BYTES(size) (((size) / PAGE_SIZE + 7) / 8)

typedef struct {
	u8 *bitmap;
	usize page_count;
	u8 *pool;
} AllocBitMap;

void *bitmap_alloc(AllocBitMap *alloc, usize page_count);

/*
 * mark the given range of pages as allocated, without allocating from the
 * pool
 */
errno_t bitmap_reserve(AllocBitMap *alloc, void *addr, usize page_count);

void bitmap_free(AllocBitMap *alloc, void *addr, usize size);

/* copy allocation state from src to dst */
errno_t bitmap_copy(AllocBitMap *dst, const AllocBitMap *src);

void bitmap_init_alloc(AllocBitMap *alloc, void *pool, usize pool_size);

#endif // _BITMAP_H_
