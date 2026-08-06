/*
 * A bitmap allocator which also requires users of it to remember the size of
 * allocations, so as to support ranged allocations.
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "types.h"
#include "memlayout.h"

#define SIZE_TO_BITMAP_BYTES(size) (((size) / PAGE_SIZE + 7) / 8)

typedef struct {
	u8 *bitmap;
	usize page_count;
	u8 *pool;
} AllocBitMap;

void *bitmap_alloc(AllocBitMap *alloc, usize page_count);
void bitmap_free(AllocBitMap *alloc, void *addr, usize size);

/* TODO: implement */
void alloc_bitmap_init(AllocBitMap *alloc, size_t start_addr, size_t end_addr);

#endif // _BITMAP_H_
