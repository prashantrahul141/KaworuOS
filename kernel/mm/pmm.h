/*
 * Physical memory allocator for user and kernel space.
 */

#ifndef _PMM_H_
#define _PMM_H_

#include "types.h"

/*
 * Init kernel physical memory allocator.
 */
void pmm_init(void);

/* Get a physical memory allocation of PAGE_SIZE size
 */
usize pmm_alloc(void);

/*
 * Return back an allocation
 */
void pmm_free(usize phy_addr);

/*
 * converts physical address to virtual address
 */
void *pmm_phys_to_virt(usize phy);

/*
 * converts virtual address to physical address
 */
usize pmm_virt_to_phys(const void *virt);

/* converts virtual to physical for kernel symbols */
usize kernel_virt_to_phys(usize va);

#endif // _PMM_H_
