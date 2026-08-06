#ifndef _TASK_H_
#define _TASK_H_

#include "aarch64/context.h"
#include "ds/intrusivelist.h"

typedef void (*task_fn_type)(void *arg);

typedef enum {
	TASK_READY = 0,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_DEAD,
} TaskState;

typedef struct {
	const i8 *name;

	void *stack;
	ExecutionContext context;

	TaskState state;
	IntrusiveNode runnable_node;

	task_fn_type entry;
	void *arg;
} Task;

void task_trampoline(void);

Task *task_create(task_fn_type task_fn, void *arg, const i8 *name);

/* exits current task */
void task_exit(Task *task);

/* destroies given task */
void task_destroy(Task *task);

void task_idle(void *arg);

#endif // _TASK_H_
