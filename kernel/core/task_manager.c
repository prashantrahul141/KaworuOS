#include "core/task_manager.h"
#include "aarch64/aarch64.h"
#include "core/task.h"
#include "debug/assert.h"
#include "debug/log.h"
#include "ds/intrusivelist.h"
#include "mm/kheap.h"
#include "sched/scheduler.h"
#include "sync/spinlock.h"

typedef struct {
	SpinLock lock;
	IntrusiveList tasks;
	usize task_id_count;
} TaskManager;

static TaskManager task_manager = {};

/*
 * The idle task.
 * Each cpu owns a separate copy of this task.
 * This task's responsiblity is to
 *    1. use minimum resources
 *    2. yield if the owning cpu has other tasks
 *    3. free zombie tasks
 */
static void task_idle(void *arg);

/*
 * The cleanup task.
 * Each cpu owns a separate copy of this task.
 * Cleans up dead tasks
 */
static void task_cleanup(void *arg);

void task_manager_init(void)
{
	spinlock_init(&task_manager.lock, "TaskManager");
	intrusivelist_init(&task_manager.tasks);
	task_manager.task_id_count = 1;
}

Task *task_manager_create(task_fn_type task_fn, void *arg, const i8 *name)
{
	Cpu *cpu = scheduler_pick_cpu();
	return task_manager_create_with_cpu(task_fn, arg, name, cpu);
}

Task *task_manager_create_with_cpu(task_fn_type task_fn, void *arg,
				   const i8 *name, Cpu *cpu)
{
	DEBUG("creating task = %s on cpu = %d", name, cpu->cpuid);
	Task *task = kalloc(sizeof(Task));
	task->cpu = (struct Cpu *)cpu;

	spinlock_acquire_scoped(&task_manager.lock);

	task_init(task, task_fn, arg, task_manager.task_id_count++, name);
	intrusivelist_insert_tail(&task_manager.tasks, &task->global_node);

	return task;
}

Task *task_manager_lookup(usize task_id)
{
	spinlock_acquire_scoped(&task_manager.lock);
	IntrusiveNode *node;
	intrusivelist_foreach(&task_manager.tasks, node) {
		Task *task = container_of(node, Task, global_node);
		if (task_id == task->tid) {
			return task;
		}
	}

	return nullptr;
}

Task *task_manager_find_dead_task(void)
{
	spinlock_acquire_scoped(&task_manager.lock);
	IntrusiveNode *node;
	intrusivelist_foreach(&task_manager.tasks, node) {
		Task *task = container_of(node, Task, global_node);
		if (TASK_DEAD == task->state) {
			return task;
		}
	}

	return nullptr;
}

void task_manager_remove_task(Task *task)
{
	spinlock_acquire_scoped(&task_manager.lock);
	intrusivelist_remove(&task_manager.tasks, &task->global_node);
}

/*
 * The idle task.
 * Each cpu owns a separate copy of this task.
 * This task's responsiblity is to
 *    1. use minimum resources
 *    2. yield if the owning cpu has other tasks
 */
static void task_idle(void *arg)
{
	UNUSED_ARG(arg);
	for (;;) {
		TRACE("idle task | cpuid = %d", this_cpu()->cpuid);
		if (scheduler_cpu_has_runnable_tasks()) {
			yield();
		}
		cpu_relax();
	}
}

/*
 * The cleanup task.
 * Each cpu owns a separate copy of this task.
 * Cleans up dead tasks
 */
static void task_cleanup(void *arg)
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
