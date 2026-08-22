#ifndef _SYSCALL_TABLE_H_
#define _SYSCALL_TABLE_H_

#include "aarch64/exception.h"
#include "types.h"

enum {
	SYS_READ = 0,
	SYS_WRITE = 1,
	SYS_OPEN = 2,
	SYS_CLOSE = 3,
	SYS_YIELD = 24,
	SYS_GETPID = 39,
	SYS_CLONE = 56,
	SYS_FORK = 57,
	SYS_VFORK = 58,
	SYS_EXECVE = 59,
	SYS_EXIT = 60,
	SYS_WAIT4 = 61,
	SYS_KILL = 62,
	SYS_UNAME = 63,
	SYS_GETPPID = 110,
};

/* this should be the last syscall id + 1 */
constexpr usize MAX_SYSCALL_COUNT = 111;

typedef struct {
	i64 ret;
	bool should_resched;
} SyscallReturn;

typedef SyscallReturn (*syscall_function_type)(const ExceptionFrame *frame);

syscall_function_type syscall_table_retrieve(usize syscall_id);

void syscall_build_table(void);

#endif // _SYSCALL_TABLE_H_
