#include "core/task.h"
#include "aarch64/aarch64.h"
#include "core/cpu.h"
#include "core/task_manager.h"
#include "debug/assert.h"
#include "ds/intrusivelist.h"
#include "irq/irq_controller.h"
#include "sched/scheduler.h"
#include "debug/log.h"
#include "debug/panic.h"
#include "mm/kheap.h"
#include "string.h"
#include "config.h"
#include "sync/wait_queue.h"

typedef void (*task_fn)(void *arg);

constexpr usize TASK_STACK_SIZE = (1 << CONFIG_PER_TASK_STACK_SIZE_SHIFT);

void task_trampoline(void)
{
	Cpu *cpu = this_cpu();
	Task *task = cpu->current;
	/* enable irq when starting a new task */
	irq_local_enable();
	task->entry(task->arg);
	DEBUG("task = %s exited", task->name);
	cpu->needs_cleanup = true;
	task_exit(task);
	panic("trampoline ended?");
}

void task_init(Task *task, task_fn_type task_fn, void *arg, usize tid,
	       const i8 *name)
{
	task->tid = tid;
	task->name = name;
	task->stack = kalloc(TASK_STACK_SIZE);

	memset(&task->context, 0, sizeof(task->context));
	task->context.sp = (usize)task->stack + TASK_STACK_SIZE;
	task->context.lr = (u64)task_trampoline;

	task->state = TASK_READY;
	intrusivelist_node_init(&task->global_node);
	intrusivelist_node_init(&task->runnable_node);
	intrusivelist_node_init(&task->wait_node);
	task->waiting_on = nullptr;

	task->sleep_until = 0;

	task->entry = task_fn;
	task->arg = arg;
}

void task_exit(Task *task)
{
	task->state = TASK_DEAD;
	yield();
}

void task_destroy(Task *task)
{
	TRACE("destroying task = %s", task->name);
	kfree(task->stack);
	kfree(task);
}

/*
 * comparator function for sleeping tasks
 */
bool task_comparator_sleep_until(IntrusiveNode *a, IntrusiveNode *b)
{
	Task *first = container_of(a, Task, wait_node);
	Task *second = container_of(b, Task, wait_node);
	return first->sleep_until > second->sleep_until;
}
