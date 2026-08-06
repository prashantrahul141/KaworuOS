#ifndef _TASK_H_
#define _TASK_H_

#include "aarch64/context.h"
#include "ds/intrusivelist.h"

typedef void (*task_fn_type)(void *arg);

struct WaitQueue;

typedef enum {
	TASK_READY = 0,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_DEAD,
} TaskState;

typedef struct {
	usize tid;
	const i8 *name;

	void *stack;
	ExecutionContext context;

	TaskState state;
	IntrusiveNode global_node;
	IntrusiveNode runnable_node;
	IntrusiveNode wait_node;
	struct WaitQueue *waiting_on;

	task_fn_type entry;
	void *arg;
} Task;

void task_trampoline(void);

void task_init(Task *task, task_fn_type task_fn, void *arg, const i8 *name);

/* exits current task */
void task_exit(Task *task);

/* destroies given task */
void task_destroy(Task *task);

void task_idle(void *arg);

#endif // _TASK_H_
