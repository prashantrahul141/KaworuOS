#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "aarch64/exception.h"

enum {
	SYS_READ = 0,
	SYS_WRITE = 1,
	SYS_OPEN = 2,
	SYS_CLOSE = 3,
	SYS_YIELD = 24,
	SYS_CLONE = 56,
	SYS_FORK = 57,
	SYS_VFORK = 58,
	SYS_EXECVE = 59,
	SYS_EXIT = 60,
	SYS_WAIT4 = 61,
	SYS_KILL = 62,
	SYS_UNAME = 63,
};

/* helper to check if exception is a syscall */
static inline bool is_syscall(ExceptionFrame *frame)
{
	return EXTRACT_BITS(frame->ESR_EL1, 31, 26) == 0b010101;
}

/*
 * dispatch syscall
 * returns if true if rescheduling is required
 */
bool syscall_dispatch(ExceptionFrame *frame);

#define SYSCALL_DEFINE1(name, type1, arg1)   i64 sys_##name(type1 arg1)
#define SYSCALL_DEFINE1_H(name, type1, arg1) i64 sys_##name(type1 arg1);

#endif // _SYSCALL_H_
