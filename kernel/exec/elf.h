#ifndef _ELF_H_
#define _ELF_H_

#include "mm/address_space.h"
#include "lib/error.h"

typedef struct {
	usize entry;
} ElfLoadResult;

errno_t elf_load(const void *file, const u64 file_size,
		 AddressSpace *addr_space, ElfLoadResult *out_result);

#endif // _ELF_H_
