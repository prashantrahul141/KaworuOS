#ifndef _SYSCALL_TABLE_H_
#define _SYSCALL_TABLE_H_

#include "aarch64/exception.h"
#include "types.h"

#define SYSCALL_LIST      \
	X(SYS_READ, 0)    \
	X(SYS_WRITE, 1)   \
	X(SYS_OPEN, 2)    \
	X(SYS_CLOSE, 3)   \
	X(SYS_YIELD, 24)  \
	X(SYS_GETPID, 39) \
	X(SYS_CLONE, 56)  \
	X(SYS_FORK, 57)   \
	X(SYS_VFORK, 58)  \
	X(SYS_EXECVE, 59) \
	X(SYS_EXIT, 60)   \
	X(SYS_WAIT4, 61)  \
	X(SYS_KILL, 62)   \
	X(SYS_UNAME, 63)  \
	X(SYS_GETPPID, 110)

#define X(variant, num) variant = (num),
enum { SYSCALL_LIST };
#undef X

/* this should be the last syscall id + 1 */
constexpr usize MAX_SYSCALL_COUNT = 111;

typedef struct {
	i64 ret;
	bool should_resched;
} SyscallReturn;

typedef SyscallReturn (*syscall_function_type)(const ExceptionFrame *frame);

syscall_function_type syscall_table_retrieve(usize syscall_id);

void syscall_build_table(void);

const i8 *syscall_to_str(usize syscall);

#endif // _SYSCALL_TABLE_H_
