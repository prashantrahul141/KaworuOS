#include "core/task_manager.h"
#include "aarch64/aarch64.h"
#include "aarch64/context.h"
#include "aarch64/user_entry_trampoline.h"
#include "core/process.h"
#include "core/task.h"
#include "debug/assert.h"
#include "debug/log.h"
#include "ds/intrusivelist.h"
#include "error.h"
#include "irq/irq_controller.h"
#include "mm/kheap.h"
#include "sched/scheduler.h"
#include "sync/spinlock.h"
#include "string.h"

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

Task *task_manager_create_user(struct Process *p, usize user_entry,
			       usize user_stack, const i8 *name)
{
	Cpu *cpu = scheduler_pick_cpu();
	DEBUG("creating user task = %s on cpu = %d", name, cpu->cpuid);

	Task *task = kalloc(sizeof(Task));
	if (IS_ERR(task)) {
		WARN("failed allocating for task: %s", name);
		return task;
	}

	task->cpu = (struct Cpu *)cpu;

	spinlock_acquire_scoped(&task_manager.lock);

	errno_t err = task_init_user(p, task, user_entry, user_stack,
				     task_manager.task_id_count++, name);
	if (EOK != err) {
		return ERR_TO_PTR(err);
	}

	intrusivelist_insert_tail(&task_manager.tasks, &task->global_node);

	return task;
}

Task *task_manager_create_user_from(struct Process *p, const Task *src,
				    const ExceptionFrame *src_frame)
{
	Task *dst = task_manager_create_user(p, (usize)src->entry,
					     src->user_stack, src->name);
	if (IS_ERR(dst)) {
		WARN("failed to create new task");
		return dst;
	}

	if (nullptr == src_frame) {
		return dst;
	}

	usize stack_top = ((usize)dst->stack + TASK_STACK_SIZE);

	ExceptionFrame *child_frame =
		(ExceptionFrame *)(stack_top - sizeof(ExceptionFrame));
	memcpy(child_frame, src_frame, sizeof(ExceptionFrame));

	child_frame->x0 = 0;
	dst->context.lr = (u64)child_return_trampoline;
	dst->context.sp = (usize)child_frame;

	return dst;
}

Task *task_manager_create(struct Process *p, task_fn_type task_fn, void *arg,
			  const i8 *name)
{
	Cpu *cpu = scheduler_pick_cpu();
	return task_manager_create_with_cpu(p, task_fn, arg, name, cpu);
}

Task *task_manager_create_with_cpu(struct Process *p, task_fn_type task_fn,
				   void *arg, const i8 *name, Cpu *cpu)
{
	DEBUG("creating task = %s on cpu = %d", name, cpu->cpuid);
	Task *task = kalloc(sizeof(Task));
	task->cpu = (struct Cpu *)cpu;

	spinlock_acquire_scoped(&task_manager.lock);

	errno_t err = task_init(p, task, task_fn, arg,
				task_manager.task_id_count++, name);
	if (EOK != err) {
		return ERR_TO_PTR(err);
	}

	intrusivelist_insert_tail(&task_manager.tasks, &task->global_node);

	return task;
}

Task *task_manager_create_idle_task(void)
{
	Task *task = kalloc(sizeof(Task));
	if (IS_ERR(task)) {
		ERROR("failed to allocate for idle task");
		return ERR_TO_PTR(-ENOMEM);
	}
	task->cpu = (struct Cpu *)this_cpu();

	errno_t err =
		task_init(nullptr, task, task_idle, nullptr, 0, "Idle task");
	if (EOK != err) {
		return ERR_TO_PTR(err);
	}

	return task;
}

Task *task_manager_create_cleanup_task(void)
{
	Task *task = kalloc(sizeof(Task));
	if (IS_ERR(task)) {
		ERROR("failed to allocate for cleanup task");
		return ERR_TO_PTR(-ENOMEM);
	}

	task->cpu = (struct Cpu *)this_cpu();

	errno_t err = task_init(nullptr, task, task_cleanup, nullptr,
				UINT64_MAX, "Clean up");
	if (EOK != err) {
		return ERR_TO_PTR(err);
	}

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

void task_manager_set_state(Task *task, TaskState state)
{
	spinlock_acquire_scoped(&task_manager.lock);
	task->state = state;
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
	irq_local_enable();
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
		Process *proc = (Process *)dead->process;
		if (proc != nullptr) {
			process_remove_thread((Process *)dead->process, dead);

			if (proc->exiting && 0 == process_thread_count(proc)) {
				DEBUG("last thread existed for process %s, "
				      "cleaning address space",
				      proc->name);
				address_space_destroy(proc->address_space);
				proc->address_space = nullptr;
			}
		}
		task_destroy(dead);
	}
}
