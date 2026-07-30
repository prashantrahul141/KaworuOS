#ifndef _SEMIHOSTING_H_
#define _SEMIHOSTING_H_

#include "types.h"

void semihosting_init(void);

/*
 * https://github.com/ARM-software/abi-aa/blob/2025Q4/semihosting/semihosting.rst#6semihosting-operations
 */
typedef enum : usize {
	SYS_CLOCK = 0x10,
	SYS_CLOSE = 0x02,
	SYS_ELAPSED = 0x30,
	SYS_ERRNO = 0x13,
	SYS_EXIT = 0x18,
	SYS_EXIT_EXTENDED = 0x20,
	SYS_FLEN = 0x0c,
	SYS_GET_CMDLINE = 0x15,
	SYS_HEAPINFO = 0x16,
	SYS_ISERROR = 0x08,
	SYS_ISTTY = 0x09,
	SYS_OPEN = 0x01,
	SYS_READ = 0x06,
	SYS_READC = 0x07,
	SYS_REMOVE = 0x0e,
	SYS_RENAME = 0x0f,
	SYS_SEEK = 0x0a,
	SYS_SYSTEM = 0x12,
	SYS_TICKFREQ = 0x31,
	SYS_TIME = 0x11,
	SYS_TMPNAM = 0x0d,
	SYS_WRITE = 0x05,
	SYS_WRITEC = 0x03,
	SYS_WRITE0 = 0x04,
} SemiHostingOperations;

usize sh_call(SemiHostingOperations op, void *params);

void sh_exit(u64 code);

void sh_write0(u8 *str);

void sh_writec(u8 ch);

#endif
