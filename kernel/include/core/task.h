#ifndef _TASK_H_
#define _TASK_H_

#include "aarch64/context.h"

typedef void (*task_fn_type)(void *arg);

typedef enum {
	TASK_READY = 0,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_DEAD,
} TaskState;

typedef struct {
	ExecutionContext context;
	void *stack;
	TaskState state;
	task_fn_type entry;
	void *arg;
} Task;

void task_trampoline(void);

Task *task_create(task_fn_type task_fn, void *arg);

/* exits current task */
void task_exit(void);

/* destroies given task */
void task_destroy(Task *task);

void task_idle(void *arg);

#endif // _TASK_H_
