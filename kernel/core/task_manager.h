#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include "aarch64/exception.h"
#include "core/task.h"
#include "core/cpu.h"

void task_manager_init(void);

Task *task_manager_create_user(struct Process *p, usize user_entry,
			       usize user_stack, const i8 *name);

/* creates a new task and copies fields from the old one */
Task *task_manager_create_user_from(struct Process *p, const Task *src,
				    const ExceptionFrame *src_frame);

Task *task_manager_create(struct Process *p, task_fn_type task_fn, void *arg,
			  const i8 *name);

Task *task_manager_create_with_cpu(struct Process *p, task_fn_type task_fn,
				   void *arg, const i8 *name, Cpu *cpu);

Task *task_manager_lookup(usize task_id);

Task *task_manager_create_idle_task(void);

Task *task_manager_create_cleanup_task(void);

Task *task_manager_find_dead_task(void);

void task_manager_remove_task(Task *task);

/* set state of a task with lock held */
void task_manager_set_state(Task *task, TaskState state);

#endif // _TASK_MANAGER_H_
