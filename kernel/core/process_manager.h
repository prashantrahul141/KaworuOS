#ifndef _PROCESS_MANAGER_H_
#define _PROCESS_MANAGER_H_

#include "aarch64/exception.h"
#include "core/process.h"
#include "types.h"

void proc_manager_init(void);

/*
 * starts process from an elf file
 */
Process *proc_manager_create_exec_from_elf(const i8 *name, const void *elf,
					   const usize elf_size);

/*
 * creates new process without a single task in it
 */
Process *proc_manager_create(const i8 *name);

/*
 * creates child from an existing process
 */
Process *
proc_manager_create_exec_child_from(Process *parent_proc,
				    const Task *parent_task,
				    const ExceptionFrame *parent_frame);

/*
 * creates from an existing process
 */
Process *proc_manager_create_exec_from(const Process *src_proc,
				       const Task *src_task,
				       const ExceptionFrame *src_frame);

/*
 * destroys process
 */
void proc_manager_remove_destroy(Process *proc);

/*
 * creates a new process with the given process entry point
 */
Process *proc_manager_create_exec(const i8 *name, usize program_pa,
				  usize program_size, usize entry);

#endif // _PROCESS_MANAGER_H_
