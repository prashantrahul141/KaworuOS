#ifndef _PROCESS_MANAGER_H_
#define _PROCESS_MANAGER_H_

#include "core/process.h"
#include "core/syscall.h"
#include "core/syscall_table.h"
#include "types.h"

void proc_manager_init(void);

/*
 * creates new process without a single task in it
 */
Process *proc_manager_create(const i8 *name);

/*
 * destroys process
 */
void proc_manager_remove_destroy(Process *proc);

/*
 * creates a new process with the given process entry point
 */
Process *proc_manager_create_exec(const i8 *name, usize program_pa,
				  usize program_size, usize entry);

SYSCALL_DECLARE(fork);

SYSCALL_DECLARE(exit);

SYSCALL_DECLARE(yield);

#endif // _PROCESS_MANAGER_H_
