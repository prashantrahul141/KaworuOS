#include "core/syscall_table.h"
#include "core/process_manager.h"
#include "core/process.h"

/* keep this sorted for sanity */
static syscall_function_type global_syscall_table[] = {
	[SYS_YIELD] = sys_yield,
	[SYS_GETPID] = sys_getpid,
	[SYS_EXIT] = sys_exit,
};

syscall_function_type syscall_table_retrieve(usize syscall_id)
{
	if (syscall_id > sizeof(global_syscall_table)) {
		return nullptr;
	}

	return global_syscall_table[syscall_id];
}
