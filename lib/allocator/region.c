#include "region.h"
#include "allocator/bitmap.h"
#include "debug/assert.h"
#include "debug/panic.h"
#include "error.h"
#include "memlayout.h"
#include "mm/kheap.h"
#include "sync/spinlock.h"
#include "string.h"

void region_init(AllocRegion *region, const i8 *msg)
{
	spinlock_init(&region->lock, msg);
	DEBUG("region init name = %s, allocations_count = %p", msg, 1);
	memset(region->allocator.bitmap, 0,
	       SIZE_TO_BITMAP_BYTES(region->allocator.page_count * PAGE_SIZE));
	memset(region->allocations, 0,
	       sizeof(*region->allocations) * region->allocations_size);
}

AllocRegion *region_create(usize base, usize size, usize max_allocations,
			   const i8 *name)
{
	AllocRegion *region = kalloc(sizeof(AllocRegion));
	if (IS_ERR(region)) {
		return ERR_TO_PTR(-ENOMEM);
	}

	usize bitmap_bytes_count = SIZE_TO_BITMAP_BYTES(size);
	u8 *bitmap = kalloc(bitmap_bytes_count);
	if (IS_ERR(bitmap)) {
		kfree(region);
		return ERR_TO_PTR(-ENOMEM);
	}

	RegionAllocation *allocs =
		kalloc(sizeof(RegionAllocation) * max_allocations);
	if (IS_ERR(bitmap)) {
		kfree(bitmap);
		kfree(region);
		return ERR_TO_PTR(-ENOMEM);
	}

	spinlock_init(&region->lock, name);
	region->allocator.bitmap = bitmap;
	region->allocator.page_count = size / PAGE_SIZE;
	region->allocator.pool = (u8 *)base;
	region->allocations = allocs;
	region->allocations_size = max_allocations;

	return region;
}

/*
 * fre dynamically creates region
 */
void region_destroy(AllocRegion *region)
{
	kfree(region->allocator.bitmap);
	kfree(region->allocations);
	kfree(region);
}

RegionAllocation *region_find(AllocRegion *region, void *addr)
{
	for (size_t i = 0; i < region->allocations_size; i++) {
		if (addr == region->allocations[i].va) {
			return &region->allocations[i];
		}
	}
	return ERR_TO_PTR(-ENOENT);
}

void *region_alloc(AllocRegion *region, usize page_count)
{
	ASSERT(page_count > 0, "Page count is zero?");
	spinlock_acquire_scoped(&region->lock);
	RegionAllocation *vm_allocation = region_find(region, nullptr);
	if (IS_ERR(vm_allocation)) {
		return ERR_TO_PTR(-ENOMEM);
	}

	void *va = bitmap_alloc(&region->allocator, page_count);
	if (IS_ERR(va)) {
		return va;
	}

	*vm_allocation =
		(RegionAllocation){ .va = va, .page_count = page_count };
	return va;
}

usize region_free(AllocRegion *region, void *addr)
{
	spinlock_acquire_scoped(&region->lock);

	RegionAllocation *vm_allocation = region_find(region, addr);
	if (IS_ERR(vm_allocation)) {
		panic("tried freeing non existent allocation addr = %p", addr);
		return 0;
	}

	usize page_count = vm_allocation->page_count;

	bitmap_free(&region->allocator, addr, vm_allocation->page_count);
	vm_allocation->va = nullptr;
	vm_allocation->page_count = 0;

	return page_count;
}
