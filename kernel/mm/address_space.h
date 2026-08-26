#ifndef _ADDRESS_SPACE_H_
#define _ADDRESS_SPACE_H_

/*
 * Address space for user processes
 *
 * perms table:
 *            |  PagePerms                    |  UserExec
 *            |                               |
 * code sect  | EL1_READ_ONLY_EL0_READ_ONLY   |  EXECUTABLE
 * data sect  | EL1_READ_ONLY_EL0_READ_WRITE  |  NOT_EXECUTABLE
 * stack sect | EL1_READ_ONLY_EL0_READ_WRITE  |  NOT_EXECUTABLE
 *
 */

#include "allocator/region.h"
#include "mm/paging.h"
#include "sync/spinlock.h"

typedef struct {
	SpinLock lock;
	TableDescriptor *table;
	AllocRegion *user_region;
} AddressSpace;

/*
 * create new virtual address space
 */
AddressSpace *address_space_create(const i8 *name);

/*
 * allocate and map size bytes memory
 */
void *address_space_alloc(AddressSpace *as, usize size, PagePerms perms,
			  ExecPerms uxn);

/*
 * mapping for user space, does not take the ownership
 */
errno_t address_space_map_unowned(AddressSpace *as, usize va, usize pa,
				  usize size, PagePerms perms, ExecPerms uxn);

/*
 * mapping for user space, takes the ownership
 */
errno_t address_space_map_owned(AddressSpace *as, usize va, usize pa,
				usize size, PagePerms perms, ExecPerms uxn);

/*
 * creates a new virtual address space from an existing one
 */
AddressSpace *address_space_create_from(AddressSpace *src);

/*
 * destroy and clean entire address space and the memory it allocated
 */
void address_space_destroy(AddressSpace *as);

#endif // _ADDRESS_SPACE_H_
