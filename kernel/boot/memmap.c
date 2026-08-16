#include "boot/memmap.h"
#include "boot/limine_responses.h"
#include "limine.h"

static MemMapTable g_memmap_table = { .count = 0 };
static usize g_memmap_hddm_offset = 0;
static usize g_kernel_executable_physical_base = 0;
static usize g_kernel_executable_virtual_base = 0;

static MemMapEntryType limine_memmap_type_to_memmap(usize limine_memmap_type)
{
	switch (limine_memmap_type) {
	case LIMINE_MEMMAP_USABLE:
		return MEMMAP_USABLE;
	case LIMINE_MEMMAP_RESERVED:
		return MEMMAP_RESERVED;
	case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
		return MEMMAP_ACPI_RECLAIMABLE;
	case LIMINE_MEMMAP_ACPI_NVS:
		return MEMMAP_ACPI_NVS;
	case LIMINE_MEMMAP_BAD_MEMORY:
		return MEMMAP_BAD_MEMORY;
	case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
		return MEMMAP_BOOTLOADER_RECLAIMABLE;
	case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
		return MEMMAP_EXECUTABLE_AND_MODULES;
	case LIMINE_MEMMAP_FRAMEBUFFER:
		return MEMMAP_FRAMEBUFFER;
	case LIMINE_MEMMAP_RESERVED_MAPPED:
		return MEMMAP_RESERVED_MAPPED;
	default:
		return MEMMAP_UNKNOWN;
	}
}

static void _memmap_save_init_limine(void)
{
	volatile struct limine_memmap_response *resp = limine_memmap();
	g_memmap_table.count = resp->entry_count;
	for (usize index = 0; index < resp->entry_count; index++) {
		struct limine_memmap_entry *limine_entry = resp->entries[index];
		MemMapEntry *entry = &g_memmap_table.entries[index];
		entry->base = limine_entry->base;
		entry->length = limine_entry->length;
		entry->type = limine_memmap_type_to_memmap(limine_entry->type);
	}
}

MemMapTable *memmap_get_table(void)
{
	return &g_memmap_table;
}

usize memmap_hddm_offset(void)
{
	return g_memmap_hddm_offset;
}

/* converts virtual to physical for kernel symbols */
usize kernel_virt_to_phys(usize va)
{
	return g_kernel_executable_physical_base +
	       (va - g_kernel_executable_virtual_base);
}

void memmap_save_init(void)
{
	_memmap_save_init_limine();
	g_memmap_hddm_offset = limine_hhdm()->offset;
	g_kernel_executable_physical_base =
		limine_kernel_address()->physical_base;
	g_kernel_executable_virtual_base =
		limine_kernel_address()->virtual_base;
}
