#include "mm/address_space.h"
#include "allocator/region.h"
#include "memlayout.h"
#include "config.h"
#include "mm/kheap.h"
#include "mm/paging.h"
#include "mm/vmm.h"

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

void *address_space_alloc(AddressSpace *as, usize size, PagePerms perms,
			  ExecPerms uxn)
{
	return vm_alloc(as->table, size, as->user_region, perms,
			ATTR_INDEX_NORMAL, SHAREABLE_INNER_SHAREABLE,
			NOT_EXECUTABLE, uxn);
}

errno_t address_space_map(AddressSpace *as, usize va, usize pa, usize size,
			  PagePerms perms, ExecPerms uxn)
{
	return paging_map(as->table, va, pa, size, perms, ATTR_INDEX_NORMAL,
			  SHAREABLE_INNER_SHAREABLE, NOT_EXECUTABLE, uxn);
}

void address_space_destroy(AddressSpace *as)
{
	for (usize i = 0; i < as->user_region->allocations_size; i++) {
		RegionAllocation *alloc = &as->user_region->allocations[i];
		if (nullptr != alloc->va) {
			vm_free(as->table, alloc->va, as->user_region);
		}
	}

	region_destroy(as->user_region);
	paging_destroy_table(as->table);
	kfree(as);
}
