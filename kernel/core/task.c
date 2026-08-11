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

void task_init(Task *task, task_fn_type task_fn, void *arg, const i8 *name)
{
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
 * The idle task.
 * Each cpu owns a separate copy of this task.
 * This task's responsiblity is to
 *    1. use minimum resources
 *    2. yield if the owning cpu has other tasks
 */
void task_idle(void *arg)
{
	UNUSED_ARG(arg);
	for (;;) {
		TRACE("idle task | cpuid = %d", this_cpu()->cpuid);
		if (scheduler_cpu_has_runnable_tasks()) {
			yield();
		}
		wfi();
	}
}

/*
 * The cleanup task.
 * Each cpu owns a separate copy of this task.
 * Cleans up dead tasks
 */
void task_cleanup(void *arg)
{
	UNUSED_ARG(arg);
	Cpu *cpu = this_cpu();
	for (;;) {
		TRACE("cleanup task | cpuid = %d", cpu->cpuid);
		Task *dead = task_manager_find_dead_task();
		if (nullptr == dead) {
			cpu->needs_cleanup = false;
			yield();
			continue;
		}

		WaitQueue *waiting_on = (WaitQueue *)dead->waiting_on;

		ASSERT(waiting_on == nullptr,
		       "dead task is still "
		       "waiting on %s",
		       waiting_on->lock.name);
		ASSERT(intrusivelist_node_is_null(&dead->wait_node), "dead "
								     "task is "
								     "still "
								     "waiting");
		ASSERT(intrusivelist_node_is_null(&dead->runnable_node),
		       "dead task is still "
		       "waiting ");

		task_manager_remove_task(dead);
		task_destroy(dead);
	}
}
