#include "mm/kheap.h"
#include "debug/assert.h"
#include "mm/vmm.h"
#include "sync/spinlock.h"
#include "allocator/freelist.h"
#include "memlayout.h"

constexpr usize INITIAL_HEAP_SIZE = round_up((usize)5 * 1000 * 1000, PAGE_SIZE);
constexpr usize MAX_HEAP_SIZE = (usize)50 * 1000 * 1000;

typedef struct {
	SpinLock lock;
	FreeList freelist;
} KHeap;

static KHeap kheap = {};

static void grow_heap(usize size);

void kheap_init()
{
	INFO("Initializing kernel heap");
	spinlock_init(&kheap.lock, "kheap");
	void *pool = vm_alloc_mem(INITIAL_HEAP_SIZE);
	freelist_init(&kheap.freelist, pool, INITIAL_HEAP_SIZE);
}

void *kalloc(usize size)
{
	TRACE("kalloc size = %d", size);
	spinlock_acquire_scoped(&kheap.lock);
	void *alloc = freelist_alloc(&kheap.freelist, size);
	if (IS_ERR(alloc)) {
		DEBUG("growing heap");
		grow_heap(size);
		alloc = freelist_alloc(&kheap.freelist, size);
	}

	if (IS_ERR(alloc)) {
		return ERR_TO_PTR(-ENOMEM);
	}
	return alloc;
}

void kfree(void *ptr)
{
	TRACE("kfree ptr = %p", ptr);
	ASSERT(ptr != nullptr, "ptr is null");
	spinlock_acquire_scoped(&kheap.lock);
	freelist_free(&kheap.freelist, ptr);
}

static void grow_heap(usize size)
{
	DEBUG("growing heap current = %d, more = %d", kheap.freelist.capacity,
	      size);
	ASSERT(size != 0, "size is 0");
	if (kheap.freelist.capacity + size > MAX_HEAP_SIZE) {
		WARN("trying to grow heap more (by %p) than its max size (%p)",
		     size, MAX_HEAP_SIZE);
		return;
	}

	usize rounded_size = round_up(size, PAGE_SIZE);
	void *pool = vm_alloc_mem(rounded_size);
	freelist_add_region(&kheap.freelist, pool, rounded_size);
}
