#include "core/task_manager.h"
#include "core/task.h"
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
	task_init(task, task_fn, arg, name);

	spinlock_acquire_scoped(&task_manager.lock);
	task->cpu = (struct Cpu *)cpu;
	task->tid = task_manager.task_id_count++;
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
