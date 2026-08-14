#include "mm/vmm.h"
#include "aarch64/aarch64.h"
#include "debug/log.h"
#include "debug/panic.h"
#include "error.h"
#include "memlayout.h"
#include "mm/pmm.h"
#include "mm/paging.h"
#include "mm/vmm_region.h"
#include "error.h"

/*
 * Kernel root page table, start of address translation tree for kernel.
 * On aarch64, the virtual address space is divided into user space (each
 * process has its own) and kernel space.
 * The kernel virtual address space's root translation table is stored in
 * TTB1_EL1
 * And user space address space's root translation table is stored in
 * TTB0_EL1
 */
TableDescriptor *kernel_page_table;

/*
 * Each region has its own lock and its own apis.
 */
STATIC_ALLOC_VM_REGION(kernel_mem_region, KERNEL_VM_RANGE_BASE,
		       KERNEL_VM_RANGE_SIZE)
STATIC_ALLOC_VM_REGION(mmio_region, KERNEL_MMIO_RANGE_START,
		       KERNEL_MMIO_RANGE_SIZE)

/* initialize virtual memory manager */
void vm_init(void)
{
	INFO("Initializing virtual memory manager");

	kernel_page_table = paging_create_table();
	if (IS_ERR(kernel_page_table)) {
		panic("could not allocate for kernel page table, returned = %s",
		      str_err(PTR_TO_ERR(kernel_page_table)));
	}

	/* setup kernel paging */
	paging_kernel_init(kernel_page_table);
	vm_set_kernel_page_table();

	/* setup regions */
	region_init(&kernel_mem_region, "kernel heap region");
	region_init(&mmio_region, "mmio region");
}

/* set kernal page table for current cpu */
void vm_set_kernel_page_table(void)
{
	paging_switch_kernel_table(kernel_page_table);
}

/*
 * Allocates and maps virtual address
 *
 * prefer using specializations like vm_mem_map, vm_mmio_map
 */
void *vm_map(usize pa, usize size, VMRegion *region, PagePerms perms,
	     AttrIndex attr_index, PageShareability shareability,
	     ExecPerms privilege_execution, ExecPerms underprivilege_execution)

{
	if (!IS_PAGE_ALIGNED(size)) {
		WARN("given size is not page aligned");
		return ERR_TO_PTR(-EINVAL);
	}

	usize page_count = size / PAGE_SIZE;
	void *va = region_alloc(region, page_count);

	errno_t err = paging_map(kernel_page_table, (usize)va, pa, size, perms,
				 attr_index, shareability, privilege_execution,
				 underprivilege_execution);

	if (EOK != err) {
		WARN("failed mapping va = %p, pa = %p, size = %p", (usize)va,
		     pa, size);
		return ERR_TO_PTR(err);
	}

	return va;
}

/* map a new virtual memory page, doesnt allocate it. */
void *vm_mem_map(usize pa, usize size)
{
	DEBUG("mapping mem: pa = %p size = %d", pa, size);
	return vm_map(pa, size, &kernel_mem_region, EL1_READ_WRITE_EL0_NONE,
		      ATTR_INDEX_NORMAL, SHAREABLE_INNER_SHAREABLE,
		      NOT_EXECUTABLE, NOT_EXECUTABLE);
}

/* map a new virtual device page, doesnt allocate it. */
void *vm_mmio_map(usize pa, usize size)
{
	DEBUG("mapping mmio: pa = %p size = %d", pa, size);
	return vm_map(pa, size, &mmio_region, EL1_READ_WRITE_EL0_NONE,
		      ATTR_INDEX_DEVICE, SHAREABLE_NON_SHAREABLE,
		      NOT_EXECUTABLE, NOT_EXECUTABLE);
}

/*
 * unmaps an already mapped page, doesnt deallocate it.
 *
 * Prefer using specializations like vm_mem_unmap, vm_mmio_unmap
 */
errno_t vm_unmap(void *va, usize size, VMRegion *region)
{
	if (!IS_PAGE_ALIGNED(size)) {
		WARN("given size is not page aligned");
		return -EINVAL;
	}

	region_free(region, va);
	paging_unmap(kernel_page_table, (usize)va, size);
	return EOK;
}

/* unmaps an already mapped mem page, doesnt deallocate it. */
void vm_mem_unmap(void *addr, usize size)
{
	vm_unmap(addr, size, &kernel_mem_region);
}

/* unmaps an already mapped mmio page, doesnt deallocate it. */
void vm_mmio_unmap(void *addr, usize size)
{
	vm_unmap(addr, size, &mmio_region);
}

/* allocates and map n virtual pages  */
void *vm_alloc(usize size, VMRegion *region, PagePerms perms,
	       AttrIndex attr_index, PageShareability shareability,
	       ExecPerms privilege_execution,
	       ExecPerms underprivilege_execution)
{
	usize page_count = size / PAGE_SIZE;
	u8 *start = region_alloc(region, page_count);
	if (IS_ERR(start)) {
		WARN("failed to allocate vm size = %d, region name = %s", size,
		     region->lock.name);
		return ERR_TO_PTR(-ENOMEM);
	}

	u8 *va = start;
	for (usize page = 0; page < page_count; page++, va += PAGE_SIZE) {
		usize pa = pmm_alloc();
		if (IS_ERR_VALUE((void *)pa)) {
			panic("ran out of physical memory");
		}

		errno_t err = paging_map(kernel_page_table, (usize)va, pa,
					 PAGE_SIZE, perms, attr_index,
					 shareability, privilege_execution,
					 underprivilege_execution);

		if (EOK != err) {
			WARN("failed mapping va = %p, pa = %p, size = %p",
			     (usize)va, pa, size);
			return ERR_TO_PTR(err);
		}
	}

	tlb_flush();
	return start;
}

/*
 * allocates and map n virtual pages from mem region
 */
void *vm_alloc_mem(usize size)
{
	DEBUG("alloc & map mem: size = %d", size);
	return vm_alloc(size, &kernel_mem_region, EL1_READ_WRITE_EL0_NONE,
			ATTR_INDEX_NORMAL, SHAREABLE_INNER_SHAREABLE,
			NOT_EXECUTABLE, NOT_EXECUTABLE);
}

/*
 * allocates and map n virtual pages from mmio region
 */
void *vm_alloc_mmio(usize size)
{
	DEBUG("mapping mmio: size = %d", size);
	return vm_alloc(size, &mmio_region, EL1_READ_WRITE_EL0_NONE,
			ATTR_INDEX_DEVICE, SHAREABLE_NON_SHAREABLE,
			NOT_EXECUTABLE, NOT_EXECUTABLE);
}

/* frees and unmaps n virtual pages  */
void vm_free(void *addr, VMRegion *region)
{
	VMAllocation *allocation = region_find(region, addr);
	if (IS_ERR(allocation)) {
		panic("trying to free non existent pages, addr = %p", addr);
	}

	for (usize page = 0; page < allocation->page_count; page++) {
		void *va = (u8 *)addr + page * PAGE_SIZE;
		usize pa = paging_lookup(kernel_page_table, (usize)va);
		vm_unmap(va, PAGE_SIZE, region);
		pmm_free(pa);
	}

	region_free(region, addr);
	tlb_flush();
}

/* frees and unmaps n virtual pages from mem region */
void vm_free_mem(void *addr)
{
	vm_free(addr, &kernel_mem_region);
}

/* frees and unmaps n virtual pages from mmio region */
void vm_free_mmio(void *addr)
{
	vm_free(addr, &mmio_region);
}
