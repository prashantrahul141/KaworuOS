#include "core/task_manager.h"
#include "core/task.h"
#include "debug/log.h"
#include "ds/intrusivelist.h"
#include "mm/kheap.h"
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

Task *task_manager_create_new(task_fn_type task_fn, void *arg, const i8 *name)
{
	DEBUG("creating task = %s", name);

	Task *task = kalloc(sizeof(Task));
	task_init(task, task_fn, arg, name);

	spinlock_acquire_scoped(&task_manager.lock);
	task->tid = task_manager.task_id_count++;
	intrusivelist_insert_tail(&task_manager.tasks, &task->global_node);

	return task;
}

Task *task_manager_lookup(usize task_id)
{
	spinlock_acquire_scoped(&task_manager.lock);
	IntrusiveNode *node;
	intrusivelist_foreach(&task_manager.tasks, node)
	{
		Task *task = container_of(node, Task, global_node);
		if (task_id == task->tid) {
			return task;
		}
	}

	return nullptr;
}
