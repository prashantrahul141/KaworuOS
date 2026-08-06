#include "mm/kmem.h"
#include "debug/assert.h"
#include "error.h"
#include "limine.h"
#include "debug/panic.h"
#include "sync/spinlock.h"
#include "memlayout.h"
#include "common_defs.h"
#include "string.h"
#include "boot/limine_responses.h"

Kmem kmem = { .free_list = nullptr };

static void kmem_free_range(usize start, usize end);

void kmem_init(void)
{
	INFO("Initializing physical page allocator");
	if (nullptr == limine_memmap()) {
		panic("memory was not mapped by limine");
	}

	if (nullptr == limine_hhdm()) {
		panic("hhdm failed?");
	}

	DEBUG("hddm offset = %p", limine_hhdm()->offset);

	spinlock_init(&kmem.spinlock, "kmem");

	DEBUG("creating freelist");
	UNUSED usize count = 1;
	for (usize index = 0; index < limine_memmap()->entry_count; index++) {
		struct limine_memmap_entry *entry =
			limine_memmap()->entries[index];
		switch (entry->type) {
		case LIMINE_MEMMAP_USABLE: {
			DEBUG("entry [%d] base = %p, length = %p, end = "
			      "%p",
			      count, entry->base, entry->length,
			      entry->base + entry->length);
			kmem_free_range(entry->base,
					entry->base + entry->length);
			count++;
			break;
		}
		default:
			break;
		}
	}
}

void *kmem_alloc(void)
{
	spinlock_acquire_scoped(&kmem.spinlock);
	PhyChunk *ret = kmem.free_list;
	if (nullptr == ret) {
		return ERR_TO_PTR(-ENOMEM);
	}
	kmem.free_list = ret->next;
	memset(ret, 0, PAGE_SIZE);
	return (void *)ret;
}

void kmem_free(void *py_addr)
{
	TRACE("freeing addr = %p", py_addr);
	if (0 != (usize)py_addr % PAGE_SIZE) {
		panic("physical address = %p not aligned with PAGE_SIZE = %d",
		      py_addr, PAGE_SIZE);
	}

	// zeroout
	memset(py_addr, 0, PAGE_SIZE);

	PhyChunk *p = py_addr;
	spinlock_acquire_scoped(&kmem.spinlock);
	p->next = kmem.free_list;
	kmem.free_list = p;
	TRACE("freed addr = %p", py_addr);
}

/*
 * converts physical address to virtual address
 */
void *phys_to_virt(usize phy)
{
	return (void *)(phy + limine_hhdm()->offset);
}

/*
 * converts virtual address to physical address
 */
usize virt_to_phys(const void *virt)
{
	usize v = (usize)virt;
	ASSERT(v >= limine_hhdm()->offset,
	       "virtual address (%p) is smaller "
	       "than hhdm offset (%p)",
	       v, limine_hhdm()->offset);
	return v - limine_hhdm()->offset;
}

/* converts virtual to physical for kernel symbols */
usize kernel_virt_to_phys(usize va)
{
	return limine_kernel_address()->physical_base +
	       (va - limine_kernel_address()->virtual_base);
}

static void kmem_free_range(usize start, usize end)
{
	u8 *p = (u8 *)start;
	UNUSED usize free_count = 0;
	for (; p + PAGE_SIZE <= (u8 *)end; p += PAGE_SIZE) {
		u8 *v = phys_to_virt((usize)p);
		kmem_free(v);
		free_count += 1;
	}
	DEBUG("freed %d pages", free_count);
}
