/*
 * A very thin wrapper around bitmap allocator. it stores allocation
 * sizes
 */

#ifndef _REGION_H_
#define _REGION_H_

#include "allocator/bitmap.h"
#include "sync/spinlock.h"

typedef struct {
	void *va;
	usize page_count;
} RegionAllocation;
static_assert(sizeof(RegionAllocation) == 16, "VMAllocation is not 16 bytes?");

typedef struct {
	SpinLock lock;
	AllocBitMap allocator;
	RegionAllocation *allocations;
	usize allocations_size;
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
		.allocations_size = SIZE_TO_BITMAP_BYTES((size))                \
	};

void region_init(AllocRegion *region, const i8 *msg);

RegionAllocation *region_find(AllocRegion *region, void *addr);

void *region_alloc(AllocRegion *region, usize page_count);

usize region_free(AllocRegion *region, void *addr);

#endif // _REGION_H_
