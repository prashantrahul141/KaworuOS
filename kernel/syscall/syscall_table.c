#include "syscall/syscall_table.h"
#include "syscall/sys_process.h"
#include "string.h"

/* keep this sorted for sanity */
static syscall_function_type global_syscall_table[MAX_SYSCALL_COUNT] = {};

syscall_function_type syscall_table_retrieve(usize syscall_id)
{
	if (syscall_id >= MAX_SYSCALL_COUNT) {
		return nullptr;
	}

	return global_syscall_table[syscall_id];
}

void syscall_build_table(void)
{
	memset(&global_syscall_table, 0, sizeof(global_syscall_table));

#define S(c, f) global_syscall_table[(c)] = (f)
	/* keep this sorted */
	S(SYS_YIELD, sys_yield);
	S(SYS_GETPID, sys_getpid);
	S(SYS_FORK, sys_fork);
	S(SYS_EXIT, sys_exit);
	S(SYS_GETPPID, sys_getppid);
#undef S
}
