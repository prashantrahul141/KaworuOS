#include "mm/address_space.h"
#include "allocator/region.h"
#include "debug/assert.h"
#include "debug/log.h"
#include "error.h"
#include "memlayout.h"
#include "config.h"
#include "mm/kheap.h"
#include "mm/paging.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "string.h"

AddressSpace *address_space_create(void)
{
	AddressSpace *as = kalloc(sizeof(AddressSpace));
	if (IS_ERR(as)) {
		return ERR_TO_PTR(-ENOMEM);
	}

	as->table = paging_create_table();
	if (IS_ERR(as->table)) {
		kfree(as);
		return ERR_TO_PTR(-ENOMEM);
	}

	as->user_region = region_create(USER_VM_RANGE_BASE, USER_VM_RANGE_SIZE,
					CONFIG_VM_MAX_USER_ALLOCATIONS,
					"address space");
	if (IS_ERR(as->user_region)) {
		paging_destroy_table(as->table);
		kfree(as);
		return ERR_TO_PTR(-ENOMEM);
	}

	return as;
}

static errno_t address_space_copy(AddressSpace *dst, const AddressSpace *src)
{
	usize mapped_pages = 0;
	errno_t ret = EOK;

	/* for each allocation in this address space */
	for (usize src_alloc_count = 0;
	     src_alloc_count < src->user_region->max_allocations_count;
	     src_alloc_count++) {
		RegionAllocation *src_alloc =
			&src->user_region->allocations[src_alloc_count];

		/* for each page in this allocation */
		for (usize src_page_count = 0;
		     src_page_count < src_alloc->page_count; src_page_count++) {
			usize va = (usize)src_alloc->va +
				   (src_page_count * PAGE_SIZE);

			PageDescriptor *src_pde =
				paging_lookup_desc(src->table, va);
			if (!src_pde->field.is_valid ||
			    !src_pde->field.is_page) {
				WARN("tried copying unmapped page");
				continue;
			}

			usize src_pa = paging_page_to_pa(src_pde, va);
			usize dst_pa = pmm_alloc();
			if (IS_ERR((void *)dst_pa)) {
				ERROR("failed to allocate for copying address "
				      "space");
				goto copy_cleanup;
			}

			memcpy(pmm_phys_to_virt(dst_pa),
			       pmm_phys_to_virt(src_pa), PAGE_SIZE);

			ret = paging_map(dst->table, va, dst_pa, PAGE_SIZE,
					 src_pde->field.ap,
					 src_pde->field.attr_index,
					 src_pde->field.sh, src_pde->field.pxn,
					 src_pde->field.uxn_xn);
			if (EOK != ret) {
				pmm_free(dst_pa);
				goto copy_cleanup;
			}

			mapped_pages++;
		}
	}

	return ret;

copy_cleanup:
	for (usize mapped_alloc_count = 0;
	     mapped_alloc_count < dst->user_region->max_allocations_count &&
	     mapped_pages > 0;
	     mapped_alloc_count++) {
		RegionAllocation *mapped_alloc =
			&dst->user_region->allocations[mapped_alloc_count];
		if (nullptr == mapped_alloc) {
			continue;
		}

		for (usize mapped_page_count = 0;
		     mapped_page_count < mapped_alloc->page_count &&
		     mapped_pages > 0;
		     mapped_page_count++, mapped_pages--) {
			usize va = (usize)mapped_alloc->va +
				   (mapped_page_count * PAGE_SIZE);

			usize pa = paging_lookup(dst->table, va);
			if (0 != pa) {
				pmm_free(pa);
			}
		}
	}

	return ret;
}

/*
 * creates a new virtual address space from an existing one,
 * copying it
 */
AddressSpace *address_space_create_from(const AddressSpace *src)
{
	AddressSpace *dst = address_space_create();
	if (IS_ERR(dst)) {
		return dst;
	}

	/* copy allocator state */
	errno_t ret = region_copy(dst->user_region, src->user_region);
	if (EOK != ret) {
		address_space_destroy(dst);
		WARN("failed copying region for address space");
		return ERR_TO_PTR(ret);
	}

	/* copy page table entries */
	ret = address_space_copy(dst, src);
	if (EOK != ret) {
		address_space_destroy(dst);
		WARN("failed copying mappings for address space");
		return ERR_TO_PTR(ret);
	}

	return dst;
}

void *address_space_alloc(AddressSpace *as, usize size, PagePerms perms,
			  ExecPerms uxn)
{
	return vm_alloc(as->table, size, as->user_region, perms,
			ATTR_INDEX_NORMAL, SHAREABLE_INNER_SHAREABLE,
			NOT_EXECUTABLE, uxn);
}

/*
 * mapping for user space, takes the ownership
 */
errno_t address_space_map_owned(AddressSpace *as, usize va, usize pa,
				usize size, PagePerms perms, ExecPerms uxn)
{
	ASSERT(IS_PAGE_ALIGNED(size), "size is not page aligned");

	usize page_count = size / PAGE_SIZE;
	if (0 == page_count) {
		return EOK;
	}

	errno_t err = region_reserve(as->user_region, (void *)va, page_count);
	if (EOK != err) {
		WARN("failed reserving region for owned mapping address space");
		return err;
	}

	err = address_space_map_unowned(as, va, pa, size, perms, uxn);
	if (EOK != err) {
		region_free(as->user_region, (void *)va);
		return err;
	}

	return EOK;
}

errno_t address_space_map_unowned(AddressSpace *as, usize va, usize pa,
				  usize size, PagePerms perms, ExecPerms uxn)
{
	return paging_map(as->table, va, pa, size, perms, ATTR_INDEX_NORMAL,
			  SHAREABLE_INNER_SHAREABLE, NOT_EXECUTABLE, uxn);
}

void address_space_destroy(AddressSpace *as)
{
	for (usize i = 0; i < as->user_region->max_allocations_count; i++) {
		RegionAllocation *alloc = &as->user_region->allocations[i];
		if (nullptr != alloc->va) {
			vm_free(as->table, alloc->va, as->user_region);
		}
	}

	region_destroy(as->user_region);
	paging_destroy_table(as->table);
	kfree(as);
}
