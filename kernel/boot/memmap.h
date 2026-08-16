#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#include "types.h"

constexpr usize MAX_MEMMAP_ENTRIES = (1 << 7);

typedef enum {
	MEMMAP_USABLE,
	MEMMAP_RESERVED,
	MEMMAP_ACPI_RECLAIMABLE,
	MEMMAP_ACPI_NVS,
	MEMMAP_BAD_MEMORY,
	MEMMAP_BOOTLOADER_RECLAIMABLE,
	MEMMAP_EXECUTABLE_AND_MODULES,
	MEMMAP_FRAMEBUFFER,
	MEMMAP_RESERVED_MAPPED,
	MEMMAP_UNKNOWN
} MemMapEntryType;

typedef struct {
	usize base;
	usize length;
	MemMapEntryType type;
} MemMapEntry;

typedef struct {
	MemMapEntry entries[MAX_MEMMAP_ENTRIES];
	usize count;
} MemMapTable;

void memmap_save_init(void);

/* mem map table */
MemMapTable *memmap_get_table(void);

/* hddm offset */
usize memmap_hddm_offset(void);

/* converts virtual to physical for kernel symbols */
usize kernel_virt_to_phys(usize va);

#endif // _MEMMAP_H_
