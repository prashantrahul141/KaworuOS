#include "mm/pmm.h"
#include "debug/assert.h"
#include "error.h"
#include "limine.h"
#include "debug/panic.h"
#include "sync/spinlock.h"
#include "memlayout.h"
#include "common_defs.h"
#include "string.h"
#include "boot/limine_responses.h"

typedef struct PhyChunk PhyChunk;

struct PhyChunk {
	PhyChunk *next;
};

typedef struct {
	SpinLock spinlock;
	PhyChunk *free_list;
} Pmm;

Pmm pmm = { .free_list = nullptr };

static void pmm_free_range(usize start, usize end);

void pmm_init(void)
{
	INFO("Initializing physical page allocator");
	if (nullptr == limine_memmap()) {
		panic("memory was not mapped by limine");
	}

	if (nullptr == limine_hhdm()) {
		panic("hhdm failed?");
	}

	DEBUG("hddm offset = %p", limine_hhdm()->offset);

	spinlock_init(&pmm.spinlock, "pmm");

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
			pmm_free_range(entry->base,
				       entry->base + entry->length);
			count++;
			break;
		}
		default:
			break;
		}
	}
}

usize pmm_alloc(void)
{
	spinlock_acquire_scoped(&pmm.spinlock);
	PhyChunk *ret = pmm.free_list;
	if (nullptr == ret) {
		return (usize)ERR_TO_PTR(-ENOMEM);
	}
	pmm.free_list = ret->next;
	memset(ret, 0, PAGE_SIZE);
	return pmm_virt_to_phys(ret);
}

void pmm_free(usize phy_addr)
{
	TRACE("freeing addr = %p", phy_addr);
	if (!IS_PAGE_ALIGNED(phy_addr)) {
		panic("physical address = %p not aligned with PAGE_SIZE = %d",
		      phy_addr, PAGE_SIZE);
	}

	PhyChunk *p = pmm_phys_to_virt(phy_addr);
	spinlock_acquire_scoped(&pmm.spinlock);
	p->next = pmm.free_list;
	pmm.free_list = p;
	TRACE("freed addr = %p", phy_addr);
}

/*
 * converts physical address to virtual address
 */
void *pmm_phys_to_virt(usize phy)
{
	return (void *)(phy + limine_hhdm()->offset);
}

/*
 * converts virtual address to physical address
 */
usize pmm_virt_to_phys(const void *virt)
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

static void pmm_free_range(usize start, usize end)
{
	u8 *p = (u8 *)start;
	UNUSED usize free_count = 0;
	for (; p + PAGE_SIZE <= (u8 *)end; p += PAGE_SIZE) {
		pmm_free((usize)p);
		free_count += 1;
	}
	DEBUG("freed %d pages", free_count);
}
