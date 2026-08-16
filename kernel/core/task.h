#ifndef _TASK_H_
#define _TASK_H_

#include "aarch64/context.h"
#include "ds/intrusivelist.h"
#include "error.h"

typedef void (*task_fn_type)(void *arg);

struct WaitQueue;
struct Cpu;
struct Process;

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

	IntrusiveNode process_node;
	struct Process *process;

	usize sleep_until;

	task_fn_type entry;
	void *arg;

	usize user_stack;
} Task;

void task_trampoline(void);

errno_t task_init(struct Process *p, Task *task, task_fn_type task_fn,
		  void *arg, usize tid, const i8 *name);

errno_t task_init_user(struct Process *p, Task *task, usize user_entry,
		       usize stack, usize tid, const i8 *name);

/* exits current task */
NORETURN void task_exit(Task *task);

/* destroies given task */
void task_destroy(Task *task);

/*
 * comparator function for sleeping tasks
 */
bool task_comparator_sleep_until(IntrusiveNode *a, IntrusiveNode *b);

#endif // _TASK_H_
