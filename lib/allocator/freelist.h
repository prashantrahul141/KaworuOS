#ifndef _FREE_LIST_H_
#define _FREE_LIST_H_

#include "types.h"

typedef struct FreeListBlockEntry FreeListBlockEntry;

struct FreeListBlockEntry {
	usize size;
	FreeListBlockEntry *next;
};

typedef struct {
	FreeListBlockEntry *head;
	usize capacity;
} FreeList;

/*
 * Initialize
 */
void freelist_init(FreeList *freelist, void *pool, const usize size);

/*
 * allocate from the freelist
 */
void *freelist_alloc(FreeList *freelist, const usize size);

/*
 * return back to the freelist
 */
void freelist_free(FreeList *freelist, void *ptr);

/*
 * add new region to allocator pool
 */
void freelist_add_region(FreeList *freelist, void *ptr, usize size);

#endif // _FREE_LIST_H_
