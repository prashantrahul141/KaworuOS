#ifndef _TASK_H_
#define _TASK_H_

#include "aarch64/context.h"
#include "ds/intrusivelist.h"

typedef void (*task_fn_type)(void *arg);

struct WaitQueue;
struct Cpu;

typedef enum {
	TASK_READY = 0,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_DEAD,
} TaskState;

typedef struct {
	usize tid;
	const i8 *name;

	struct Cpu *cpu;
	void *stack;
	ExecutionContext context;
	TaskState state;

	IntrusiveNode global_node;

	IntrusiveNode runnable_node;

	IntrusiveNode wait_node;
	struct WaitQueue *waiting_on;

	usize sleep_until;

	task_fn_type entry;
	void *arg;
} Task;

void task_trampoline(void);

void task_init(Task *task, task_fn_type task_fn, void *arg, const i8 *name);

/* exits current task */
void task_exit(Task *task);

/* destroies given task */
void task_destroy(Task *task);

/*
 * The idle task.
 * Each cpu owns a separate copy of this task.
 * This task's responsiblity is to
 *    1. use minimum resources
 *    2. yield if the owning cpu has other tasks
 *    3. free zombie tasks
 */
void task_idle(void *arg);

/*
 * The cleanup task.
 * Each cpu owns a separate copy of this task.
 * Cleans up dead tasks
 */
void task_cleanup(void *arg);

/*
 * comparator function for sleeping tasks
 */
bool task_comparator_sleep_until(IntrusiveNode *a, IntrusiveNode *b);

#endif // _TASK_H_
