#include "core/task.h"
#include "core/cpu.h"
#include "ds/intrusivelist.h"
#include "error.h"
#include "irq/irq_controller.h"
#include "sched/scheduler.h"
#include "debug/log.h"
#include "debug/panic.h"
#include "mm/kheap.h"
#include "string.h"
#include "config.h"
#include "aarch64/user_entry_trampoline.h"

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

errno_t task_init_user(struct Process *p, Task *task, usize user_entry,
		       usize stack, usize tid, const i8 *name)
{
	errno_t err = task_init(p, task, nullptr, nullptr, tid, name);
	if (EOK != err) {
		return -ENOMEM;
	}

	task->user_stack = stack;
	task->context.lr = (usize)user_entry_trampoline;

	/* we put user entry and stack in kernel stack frame */
	u64 *frame = (u64 *)task->context.sp - 2;
	frame[0] = user_entry;
	frame[1] = stack;
	task->context.sp = (usize)frame;

	return EOK;
}

errno_t task_init(struct Process *p, Task *task, task_fn_type task_fn,
		  void *arg, usize tid, const i8 *name)
{
	task->tid = tid;
	task->name = name;

	task->stack = kalloc(TASK_STACK_SIZE);
	if (IS_ERR(task->stack)) {
		WARN("failed to allocate for task stack: %s", name);
		return -ENOMEM;
	}

	task->process = p;

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
	return EOK;
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
