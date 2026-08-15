#ifndef _VM_H_
#define _VM_H_

#include "mm/paging.h"
#include "allocator/region.h"
#include "types.h"

/* initialize virtual memory manager */
void vm_init(void);

/* set kernal page table for current cpu */
void vm_set_kernel_page_table(void);

/* get kernal user page table */
TableDescriptor *vm_get_kernel_user_page_table(void);

/*
 * Allocates and maps virtual address
 *
 * prefer using specializations like vm_mem_map, vm_mmio_map
 */
void *vm_map(TableDescriptor *table, usize pa, usize size, AllocRegion *region,
	     PagePerms perms, AttrIndex attr_index,
	     PageShareability shareability, ExecPerms privilege_execution,
	     ExecPerms underprivilege_execution);

/* map a new virtual memory page, doesnt allocate it. */
void *vm_mem_map(usize pa, usize size);

/* map a new virtual device page, doesnt allocate it. */
void *vm_mmio_map(usize pa, usize size);

/*
 * unmaps an already mapped page, doesnt deallocate it.
 *
 * Prefer using specializations like vm_mem_unmap, vm_mmio_unmap
 */
errno_t vm_unmap(TableDescriptor *table, void *va, usize size,
		 AllocRegion *region);

/* unmaps an already mapped mem page, doesnt deallocate it. */
void vm_mem_unmap(void *page, usize size);

/* unmaps an already mapped mmio page, doesnt deallocate it. */
void vm_mmio_unmap(void *page, usize size);

/*
 * allocates and map n virtual pages
 *
 * Prefer using specializations like vm_alloc_mem, vm_alloc_mmio
 */
void *vm_alloc(TableDescriptor *table, usize size, AllocRegion *region,
	       PagePerms perms, AttrIndex attr_index,
	       PageShareability shareability, ExecPerms privilege_execution,
	       ExecPerms underprivilege_execution);

/*
 * allocates and map n virtual pages from mem region
 */
void *vm_alloc_mem(usize size);

/*
 * allocates and map n virtual pages from mmio region
 */
void *vm_alloc_mmio(usize size);

/*
 * unmaps and deallocates already mapped page.
 * prefer using specializations: vm_free_mmio, vm_free_mem
 */
void vm_free(TableDescriptor *table, void *addr, AllocRegion *region);

/* frees and unmaps n virtual pages from mem region */
void vm_free_mem(void *addr);

/* frees and unmaps n virtual pages from mmio region */
void vm_free_mmio(void *addr);

#endif
