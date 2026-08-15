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

typedef struct {
	TableDescriptor *table;
	AllocRegion *user_region;
} AddressSpace;

/*
 * create new virtual address space
 */
AddressSpace *address_space_create(void);

/*
 * allocate and map size bytes memory
 */
void *address_space_alloc(AddressSpace *as, usize size, PagePerms perms,
			  ExecPerms uxn);

/*
 * mapping for user space
 */
errno_t address_space_map(AddressSpace *as, usize va, usize pa, usize size,
			  PagePerms perms, ExecPerms uxn);

/*
 * destroy and clean entire address space and the memory it allocated
 */
void address_space_destroy(AddressSpace *as);

#endif // _ADDRESS_SPACE_H_
