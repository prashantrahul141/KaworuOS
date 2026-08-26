/*
 * A very thin wrapper around bitmap allocator. it stores allocation
 * sizes
 */

#ifndef _REGION_H_
#define _REGION_H_

#include "allocator/bitmap.h"

typedef struct {
	void *va;
	usize page_count;
} RegionAllocation;
static_assert(sizeof(RegionAllocation) == 16, "VMAllocation is not 16 bytes?");

typedef struct {
	const i8 *name;
	AllocBitMap allocator;
	RegionAllocation *allocations;
	usize max_allocations_count;
} AllocRegion;

#define STATIC_ALLOC_VM_REGION(name, addr, size)                                \
	static u8 _bitmap_storage_##name[SIZE_TO_BITMAP_BYTES((size))] = { 0 }; \
	static RegionAllocation                                                 \
		_allocations_storage_##name[(size) / PAGE_SIZE] = { 0 };        \
	static AllocRegion name = {                                             \
		.allocator = { .bitmap = _bitmap_storage_##name,                \
			       .page_count = (size) / (PAGE_SIZE),              \
			       .pool = (u8 *)(addr) },                          \
		.allocations = _allocations_storage_##name,                     \
		.max_allocations_count = SIZE_TO_BITMAP_BYTES((size))           \
	};

/*
 * for dynamically creating new regions
 */
AllocRegion *region_create(usize base, usize size, usize max_allocations,
			   const i8 *name);

/*
 * init static allocated region
 */
void region_init(AllocRegion *region, const i8 *msg);

RegionAllocation *region_find(AllocRegion *region, void *addr);

void *region_alloc(AllocRegion *region, usize page_count);

/*
 * reserve the given va range in the region, marking it as allocated
 */
errno_t region_reserve(AllocRegion *region, void *va, usize page_count);

/* copy state of region allocator from src to dst */
errno_t region_copy(AllocRegion *dst, AllocRegion *src);

usize region_free(AllocRegion *region, void *addr);

void region_destroy(AllocRegion *region);

#endif // _REGION_H_
