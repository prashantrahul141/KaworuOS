#ifndef _SEMIHOSTING_H_
#define _SEMIHOSTING_H_

#include "types.h"

void semihosting_init(void);

/*
 * https://github.com/ARM-software/abi-aa/blob/2025Q4/semihosting/semihosting.rst#6semihosting-operations
 */
typedef enum : usize {
	SH_SYS_CLOCK = 0x10,
	SH_SYS_CLOSE = 0x02,
	SH_SYS_ELAPSED = 0x30,
	SH_SYS_ERRNO = 0x13,
	SH_SYS_EXIT = 0x18,
	SH_SYS_EXIT_EXTENDED = 0x20,
	SH_SYS_FLEN = 0x0c,
	SH_SYS_GET_CMDLINE = 0x15,
	SH_SYS_HEAPINFO = 0x16,
	SH_SYS_ISERROR = 0x08,
	SH_SYS_ISTTY = 0x09,
	SH_SYS_OPEN = 0x01,
	SH_SYS_READ = 0x06,
	SH_SYS_READC = 0x07,
	SH_SYS_REMOVE = 0x0e,
	SH_SYS_RENAME = 0x0f,
	SH_SYS_SEEK = 0x0a,
	SH_SYS_SYSTEM = 0x12,
	SH_SYS_TICKFREQ = 0x31,
	SH_SYS_TIME = 0x11,
	SH_SYS_TMPNAM = 0x0d,
	SH_SYS_WRITE = 0x05,
	SH_SYS_WRITEC = 0x03,
	SH_SYS_WRITE0 = 0x04,
} SemiHostingOperations;

usize sh_call(SemiHostingOperations op, void *params);

void sh_exit(u64 code);

void sh_write0(u8 *str);

void sh_writec(u8 ch);

#endif
