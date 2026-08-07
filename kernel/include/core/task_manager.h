#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include "core/task.h"
#include "core/cpu.h"

void task_manager_init(void);

Task *task_manager_create(task_fn_type task_fn, void *arg, const i8 *name);

Task *task_manager_create_with_cpu(task_fn_type task_fn, void *arg,
				   const i8 *name, Cpu *cpu);

Task *task_manager_lookup(usize task_id);

#endif // _TASK_MANAGER_H_
