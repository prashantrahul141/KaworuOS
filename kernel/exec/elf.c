#include "exec/elf.h"
#include "error.h"
#include "memlayout.h"
#include "mm/address_space.h"
#include "mm/paging.h"
#include "mm/pmm.h"
#include "string.h"
#include "debug/log.h"

#define ALLOW_ELF_INTERNAL_INCLUDE
#include "exec/elf_internal.h"

static constexpr u8 KNOWN_HEADER_MAGIC[] = { 0x7f, 0x45, 0x4c, 0x46 };

static inline bool
elf_verify_header_magic(const u8 magic[static ELF_HEADER_MAGIC_SIZE])
{
	return 0 == memcmp(KNOWN_HEADER_MAGIC, magic, ELF_HEADER_MAGIC_SIZE);
}

static bool elf_verify_header(const ElfHeader *header)
{
	if (!elf_verify_header_magic(header->ident_magic)) {
		ERROR("elf header mismatch");
		return false;
	}

	if (header->ident_class != ELF_HEADER_IDENT_CLASS_CLASS64) {
		ERROR("elf ident class not 64 bit");
		return false;
	}

	if (header->ident_data != ELF_HEADER_IDENT_DATA_LSB) {
		ERROR("elf ident data not lsb");
		return false;
	}

	return true;
}

static inline errno_t verify_program_header(const ElfProgramHeader *prog_header,
					    const u64 elf_size)
{
	/* mem size should always be equal or mroe than than filesize */
	if (prog_header->filesz > prog_header->memsz ||

	    /* offset cant be outside elf file */
	    prog_header->offset > elf_size ||

	    /* header's file size + offset cant be outside elf file */
	    prog_header->filesz > elf_size - prog_header->offset ||

	    /* entry point cant be outside max user vm range */
	    prog_header->vaddr > USER_VM_RANGE_END ||

	    /* memsize + virtual address cant be outside user vm range */
	    prog_header->memsz > USER_VM_RANGE_END - prog_header->vaddr

	) {
		return -EINVAL;
	}

	return EOK;
}

static errno_t map_section_from_header(const u8 *elf, const u64 elf_size,
				       const ElfProgramHeader *ph,
				       AddressSpace *as)
{
	TRACE("mapping section type = %d at va = %p of memsize = %p", ph->type,
	      ph->vaddr, ph->memsz);

	errno_t ret = verify_program_header(ph, elf_size);
	if (EOK != ret) {
		ERROR("failed verifying program header");
		return ret;
	}

	if (0 == ph->memsz) {
		return EOK;
	}

	/* page flags */
	PagePerms read_write_perms = ph->flags & PF_W ?
					     EL1_READ_WRITE_EL0_READ_WRITE :
					     EL1_READ_ONLY_EL0_READ_ONLY;
	ExecPerms exec_perms = ph->flags & PF_X ? EXECUTABLE : NOT_EXECUTABLE;

	/* calc top and bottom address, aligned */
	usize segment_start = ph->vaddr;
	usize segment_end = ph->vaddr + ph->memsz;

	usize map_start = round_down(segment_start, PAGE_SIZE);
	usize map_end = round_up(segment_end, PAGE_SIZE);

	/* map pages */
	for (usize va = map_start; va < map_end; va += PAGE_SIZE) {
		usize pa = pmm_alloc();
		if (IS_ERR((void *)pa)) {
			return PTR_TO_ERR((void *)pa);
		}

		ret = address_space_map_owned(as, va, pa, PAGE_SIZE,
					      read_write_perms, exec_perms);
		if (EOK != ret) {
			pmm_free(pa);
			return ret;
		}

		u8 *page = pmm_phys_to_virt(pa);

		/* determine how much to copy */
		usize copy_start = 0;
		usize copy_end = PAGE_SIZE;

		if (va < segment_start) {
			copy_start = segment_start - va;
		}

		if (va + PAGE_SIZE > segment_start + ph->filesz) {
			copy_end = (segment_start + ph->filesz) - va;
		}

		if (copy_end <= copy_start) {
			continue;
		}

		usize copy_size = copy_end - copy_start;
		usize segment_offset = (va + copy_start) - segment_start;
		usize file_offset = ph->offset + segment_offset;

		/* copy segments */
		memcpy(page + copy_start, elf + file_offset, copy_size);
	}

	return EOK;
}

errno_t elf_load(const void *file, const u64 file_size,
		 AddressSpace *addr_space, ElfLoadResult *out_result)
{
	DEBUG("loading elf = %p, file_size = %p", file, file_size);

	/* mind = blown */
	const ElfHeader *header = file;

	if (!elf_verify_header(header)) {
		return -EINVAL;
	}

	const ElfProgramHeader *program_headers =
		(const ElfProgramHeader *)((const u8 *)file +
					   header->program_header_off);

	for (usize prog_header_count = 0;
	     prog_header_count < header->program_header_table_entry_count;
	     prog_header_count++) {
		const ElfProgramHeader *program_header =
			&program_headers[prog_header_count];

		if (ELF_PROGRAM_HEADER_TYPE_LOAD == program_header->type) {
			errno_t ret = map_section_from_header(
				file, file_size, program_header, addr_space);
			if (EOK != ret) {
				return ret;
			}
		}
	}

	out_result->entry = header->entry;
	return EOK;
}
