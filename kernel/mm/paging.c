#include "mm/paging.h"
#include "aarch64/aarch64.h"
#include "common_defs.h"
#include "debug/log.h"
#include "error.h"
#include "limine.h"
#include "boot/limine_responses.h"
#include "memlayout.h"
#include "mm/kmem.h"
#include "common_defs.h"
#include "memlayout.h"
#include "string.h"
#include "types.h"
#include "debug/panic.h"
#include "mm/kmem.h"

constexpr usize PAGE_SHIFT = 12;
constexpr usize PT_INDEX_MASK = 0x1FF;

/* convert address to index for a translation level  */
#define L0_INDEX(va) (((u64)(va) >> 39) & PT_INDEX_MASK)
#define L1_INDEX(va) (((u64)(va) >> 30) & PT_INDEX_MASK)
#define L2_INDEX(va) (((u64)(va) >> 21) & PT_INDEX_MASK)
#define L3_INDEX(va) (((u64)(va) >> 12) & PT_INDEX_MASK)

/* convert */
#define PA_TO_PAGE_DESC(pa)	   (((u64)(pa)) >> PAGE_SHIFT)
#define PAGE_DESC_TO_PA(desc_addr) (((u64)(desc_addr)) << PAGE_SHIFT)

/* static declarations */
static PageDescriptor *walk_pagetable(TableDescriptor *page, u64 va,
				      bool allocate);

static inline errno_t map_device(TableDescriptor *table, usize va, usize pa,
				 usize size);
static inline errno_t map_text(TableDescriptor *table, usize va, usize pa,
			       usize size);
static inline errno_t map_data(TableDescriptor *table, usize va, usize pa,
			       usize size);
static MUST_CHECK TableDescriptor *table_next_table(TableDescriptor *td,
						    bool allocate);

/*
 * creates a new page table
 */
TableDescriptor *paging_create_table(void)
{
	TableDescriptor *td = kmem_alloc();
	if (IS_ERR(td)) {
		return td;
	}

	memset(td, 0, PAGE_SIZE);
	return td;
}

/*
 * destroys a page table
 */
void paging_destroy_table(TableDescriptor *table)
{
	kmem_free(table);
}

void paging_kernel_init(TableDescriptor *kernel_page_table)
{
	DEBUG("Initializing paging and mapping kernel pages");
	/* map kernel's text section */
	DEBUG("mapping kernel text section");
	map_text(kernel_page_table, __KERNEL_START,
		 kernel_virt_to_phys(__KERNEL_START),
		 __KERNEL_TEXT_END - __KERNEL_START);

	/* map kernel's data sections */
	DEBUG("mapping kernel data section");
	map_data(kernel_page_table, __KERNEL_DATA_START,
		 kernel_virt_to_phys(__KERNEL_DATA_START),
		 __KERNEL_DATA_END - __KERNEL_DATA_START);

	/* map hhdm entries */
	DEBUG("mapping hhdm entries");
	for (usize index = 0; index < limine_memmap()->entry_count; index++) {
		struct limine_memmap_entry *entry =
			limine_memmap()->entries[index];
		switch (entry->type) {
			// TODO: research about other types of entries and map
			// them too.
		case LIMINE_MEMMAP_FRAMEBUFFER: {
			map_device(kernel_page_table,
				   (usize)phys_to_virt(entry->base),
				   entry->base, entry->length);
			break;
		}

		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
		case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
		case LIMINE_MEMMAP_USABLE: {
			map_data(kernel_page_table,
				 (usize)phys_to_virt(entry->base), entry->base,
				 entry->length);
			break;
		}
		default:
			break;
		}
	}

	/* make sure all writes are complete at this point. */
	dsb(BARRIER_ALL);
	isb();

	DEBUG("mapping complete");
}

void paging_switch_kernel_table(TableDescriptor *kernel_page_table)
{
	DEBUG("swapping kernel page table");

	/* write kernel pages to TTB1_EL1 */
	w_ttbr1_el1(virt_to_phys(kernel_page_table));

	/* flush flush flush! */
	dsb(BARRIER_ALL);
	isb();

	tlb_flush();

	dsb(BARRIER_ALL);
	isb();
}

void paging_switch_user_table(TableDescriptor *user_page_table)
{
	DEBUG("swapping user page table");

	/* write user pages to TTB0_EL1 */
	w_ttbr0_el1(virt_to_phys(user_page_table));

	/* flush flush flush! */
	dsb(BARRIER_ALL);
	isb();

	tlb_flush();

	dsb(BARRIER_ALL);
	isb();
}

/* maps given virtual address to physical address to */
errno_t paging_map(TableDescriptor *table, usize va, usize pa, usize size,
		   PagePerms perms, AttrIndex attr_index,
		   PageShareability shareability, ExecPerms privilege_execution,
		   ExecPerms underprivilege_execution)
{
	TRACE("Map va = %p, pa = %p, size = %p", va, pa, size);

	if (!IS_PAGE_ALIGNED(va)) {
		panic("va = %p is not page aligned (%p)", va, PAGE_SIZE);
	}

	if (!IS_PAGE_ALIGNED(size)) {
		panic("size = %p is not page aligned (%p)", size, PAGE_SIZE);
	}

	for (usize end = va + size; va < end;
	     va += PAGE_SIZE, pa += PAGE_SIZE) {
		/* this contains a leaf node which we can map */
		PageDescriptor *pde = walk_pagetable(table, va, true);
		if (pde->field.is_valid) {
			panic("remaping an already existing page va = %p, pde "
			      "= %p",
			      va, pde);
		}

		pde->field.is_page = 0b1;
		pde->field.is_valid = PAGE_IS_VALID;
		pde->field.attr_index = attr_index;
		pde->field.ap = perms;
		pde->field.sh = shareability;
		pde->field.af = AF_SET;
		pde->field.output_address = PA_TO_PAGE_DESC(pa);
		pde->field.pxn = privilege_execution;
		pde->field.uxn_xn = underprivilege_execution;
	}

	tlb_flush();
	return EOK;
}

void paging_unmap(TableDescriptor *page, usize va, usize size)
{
	DEBUG("Unmap va = %p, size = %p", va, size);

	if (!IS_PAGE_ALIGNED(va)) {
		panic("va = %p is not page aligned (%p)", va, PAGE_SIZE);
	}

	if (!IS_PAGE_ALIGNED(size)) {
		panic("size = %p is not page aligned (%p)", size, PAGE_SIZE);
	}

	for (usize end = va + size; va < end; va += PAGE_SIZE) {
		/* this contains a leaf node */
		PageDescriptor *pde = walk_pagetable(page, va, false);
		if (!pde->field.is_valid) {
			panic("trying to unmap %p which is not a mapped page",
			      va);
		}

		pde->raw = 0;
	}

	tlb_flush();
}

usize paging_lookup(TableDescriptor *table, u64 va)
{
	PageDescriptor *pde = walk_pagetable(table, va, false);
	if (IS_ERR(pde)) {
		return 0;
	}

	return PAGE_DESC_TO_PA(pde->field.output_address) |
	       (va & (PAGE_SIZE - 1));
}

/* walks the given page table and return leaf descriptor */
static PageDescriptor *walk_pagetable(TableDescriptor *table, u64 va,
				      bool allocate)
{
	TRACE("walking table = %p, va = %p, allocate = %b", table, va,
	      allocate);

	usize l0_index = L0_INDEX(va);
	TableDescriptor *l1 = table_next_table(&table[l0_index], allocate);

	TRACE("\tl0_index =  %d, l1_table = %p", l0_index, l1);
	if (IS_ERR(l1)) {
		panic("failed to get next table for l1 va = %p, l0 index = %d, "
		      "allocate = %b",
		      va, l0_index, allocate);
	}

	usize l1_index = L1_INDEX(va);
	TableDescriptor *l2 = table_next_table(&l1[l1_index], allocate);

	TRACE("\tl1_index =  %d, l2_table = %p", l1_index, l2);
	if (IS_ERR(l2)) {
		panic("failed to get next table for l2 va = %p, l1 index = %d, "
		      "allocate = %b",
		      va, l1_index, allocate);
	}

	usize l2_index = L2_INDEX(va);
	TableDescriptor *l3 = table_next_table(&l2[l2_index], allocate);

	TRACE("\tl2_index =  %d, l3_table = %p", l2_index, l3);
	if (IS_ERR(l3)) {
		panic("failed to get next table for l3 va = %p, l2 index = %d, "
		      "allocate = %b",
		      va, l2_index, allocate);
	}

	return (PageDescriptor *)&l3[L3_INDEX(va)];
}

/* gets next level */
static MUST_CHECK TableDescriptor *table_next_table(TableDescriptor *td,
						    bool allocate)
{
	/* table already exists */
	if (td->field.is_valid) {
		usize pa = PAGE_DESC_TO_PA(td->field.next_level_address);
		return phys_to_virt(pa);
	}

	if (!allocate) {
		return ERR_TO_PTR(-ENOMEM);
	}

	TableDescriptor *page = kmem_alloc();
	td->raw = 0;
	td->field.is_valid = true;
	td->field.is_table = true;
	td->field.next_level_address = PA_TO_PAGE_DESC(virt_to_phys(page));
	return page;
}

static inline errno_t map_device(TableDescriptor *page, usize va, usize pa,
				 usize size)
{
	DEBUG("mapping device");
	return paging_map(page, va, pa, size, EL1_READ_WRITE_EL0_NONE,
			  ATTR_INDEX_DEVICE, SHAREABLE_NON_SHAREABLE,
			  NOT_EXECUTABLE, NOT_EXECUTABLE);
}

static inline errno_t map_text(TableDescriptor *page, usize va, usize pa,
			       usize size)
{
	DEBUG("mapping text");
	return paging_map(page, va, pa, size, EL1_READ_ONLY_EL0_NONE,
			  ATTR_INDEX_NORMAL, SHAREABLE_INNER_SHAREABLE,
			  EXECUTABLE, NOT_EXECUTABLE);
}

static inline errno_t map_data(TableDescriptor *page, usize va, usize pa,
			       usize size)
{
	TRACE("mapping data");
	return paging_map(page, va, pa, size, EL1_READ_WRITE_EL0_NONE,
			  ATTR_INDEX_NORMAL, SHAREABLE_INNER_SHAREABLE,
			  NOT_EXECUTABLE, NOT_EXECUTABLE);
}
