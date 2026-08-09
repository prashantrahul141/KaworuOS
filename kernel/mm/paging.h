#ifndef _VM_MEM_H_
#define _VM_MEM_H_

#include "error.h"
#include "types.h"
#include "common_defs.h"

typedef u64 pagetable_entry_t;
typedef pagetable_entry_t *pagetable_t;

typedef enum {
	PAGE_IS_INVALID = 0b0,
	PAGE_IS_VALID = 0b1,
} IsValid;

typedef enum {
	EXECUTABLE = 0b0,
	NOT_EXECUTABLE = 0b1,
} ExecPerms;

typedef enum {
	SHAREABLE_NON_SHAREABLE = 0b00,
	SHAREABLE_RESERVED = 0b01,
	SHAREABLE_OUTER_SHAREABLE = 0b10,
	SHAREABLE_INNER_SHAREABLE = 0b11,
} PageShareability;

typedef enum {
	EL1_READ_WRITE_EL0_NONE = 0b00,
	EL1_READ_WRITE_EL0_READ_WRITE = 0b01,
	EL1_READ_ONLY_EL0_NONE = 0b10,
	EL1_READ_ONLY_EL0_READ_ONLY = 0b11,
} PagePerms;

typedef enum {
	ATTR_INDEX_DEVICE = 0,
	ATTR_INDEX_NORMAL = 1,
} AttrIndex;

typedef enum {
	AF_CLEAR = 0,
	AF_SET = 1,
} AccessFlag;

typedef union {
	u64 raw;

	struct {
		u64 is_valid : 1;
		u64 is_table : 1;
		u64 ignored1 : 10;
		u64 next_level_address : 36;
		u64 reserved : 3;
		u64 ignored2 : 8;
		u64 pxn_table : 1;
		u64 xn_table : 1;
		u64 ap_table : 2;
		u64 ns_table : 1;
	} field;
} PACKED TableDescriptor;
static_assert(sizeof(TableDescriptor) == 8, "TableDescriptor is not 8 bytes?");

typedef union {
	u64 raw;

	struct {
		u64 is_valid : 1;
		u64 is_page : 1;
		u64 attr_index : 3;
		u64 ns : 1;
		u64 ap : 2;
		u64 sh : 2;
		u64 af : 1;
		u64 nG : 1;
		u64 output_address : 36;
		u64 reserved0 : 3;
		u64 dbm : 1;
		u64 contiguous : 1;
		u64 pxn : 1;
		u64 uxn_xn : 1;
		u64 reserved1 : 4;
		u64 pbha : 4;
		u64 ignored : 1;
	} field;
} PACKED PageDescriptor;
static_assert(sizeof(PageDescriptor) == 8, "PageDescriptor is not 8 bytes?");

/*
 * Init pages for kernel
 * sets up page table entries AND switches page table.
 */
void paging_kernel_init(TableDescriptor *kernel_page_table);

/*
 * switch page table to our own page table
 */
void paging_switch_kernel_table(TableDescriptor *kernel_page_table);

/*
 * maps given virtual address range to given physical address
 */
errno_t paging_map(TableDescriptor *table, usize va, usize pa, usize size,
		   PagePerms perms, AttrIndex attr_index,
		   PageShareability shareability, ExecPerms privilege_execution,
		   ExecPerms underprivilege_execution);

/*
 * unmaps given virtual address range
 */
void paging_unmap(TableDescriptor *page, usize va, usize size);

/* loops up physical address for the given va. */
usize paging_lookup(TableDescriptor *table, u64 va);

#endif // _VM_MEM_H_
