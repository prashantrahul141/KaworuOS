#include "sched/scheduler.h"
#include "aarch64/context.h"
#include "core/cpu.h"
#include "core/timer.h"
#include "debug/assert.h"
#include "ds/intrusivelist.h"
#include "sync/wait_queue.h"
#include "irq/irq_controller.h"
#include "sched/scheduler_policy.h"
#include "core/task.h"
#include "debug/log.h"
#include "debug/panic.h"
#include "sync/run_queue.h"
#include "sync/wait_queue.h"

void scheduler_init(void)
{
	scheduler_switch();
}

void yield(void)
{
	scheduler_switch();
}

/*
 * Add task to runnables
 */
void scheduler_enqueue(Task *task)
{
	ASSERT(task->cpu != nullptr, "task cpu is null");
	ASSERT(task->state != TASK_RUNNING, "cannot enqueue a running task");
	ASSERT(task->state != TASK_BLOCKED, "cannot enqueue a blocked task");
	ASSERT(task->waiting_on == nullptr, "cannot enqueue task that is "
					    "waiting");

	Cpu *cpu = (Cpu *)task->cpu;
	task->waiting_on = nullptr;
	task->state = TASK_READY;
	runqueue_enqueue(&cpu->runnable_tasks, task);
}

/*
 * Remove task from runnables
 */
void scheduler_dequeue(Task *task)
{
	ASSERT(task->cpu != nullptr, "task cpu is null");
	ASSERT(task->state == TASK_READY, "only ready tasks can be dequeued");
	ASSERT(task->waiting_on == nullptr, "runnable task is waiting on a "
					    "wait queue");

	Cpu *cpu = (Cpu *)task->cpu;
	runqueue_remove(&cpu->runnable_tasks, task);
}

/*
 * Sleeps current task for n ms
 */
void scheduler_sleep_current(usize ms)
{
	Cpu *cpu = this_cpu();
	Task *current = cpu->current;

	ASSERT(current != nullptr, "current is null");
	ASSERT(current != cpu->idle, "idle task cannot block");
	ASSERT((Cpu *)current->cpu == cpu, "current task belongs to another "
					   "cpu");
	ASSERT(TASK_RUNNING == current->state, "only running task can sleep");

	WaitQueue *sleeping = &cpu->sleeping_tasks;
	spinlock_acquire(&sleeping->lock);

	current->sleep_until = timer_current_tick() + timer_ms_to_ticks(ms);

	intrusivelist_insert_tail(&sleeping->waiters, &current->wait_node);

	current->waiting_on = (void *)sleeping;
	current->state = TASK_BLOCKED;

	spinlock_release(&sleeping->lock);
	scheduler_switch();
}

/*
 * wake timer sleeping task
 */
void scheduler_wake_sleepers(void)
{
	Cpu *cpu = this_cpu();

	WaitQueue *sleeping = &cpu->sleeping_tasks;
	usize now = timer_current_tick();

	spinlock_acquire(&sleeping->lock);
	IntrusiveNode *node = intrusivelist_peek_head(&sleeping->waiters);

	while (node != nullptr) {
		IntrusiveNode *next = node->next;
		Task *task = container_of(node, Task, wait_node);

		if (now >= task->sleep_until) {
			intrusivelist_remove(&sleeping->waiters, node);
			task->waiting_on = nullptr;
			task->sleep_until = 0;
			spinlock_release(&sleeping->lock);
			/* set ready and enqueue */
			scheduler_wake_blocked(task);
			spinlock_acquire(&sleeping->lock);
		}
		node = next;
	}

	spinlock_release(&sleeping->lock);
}

/*
 * blocks the current task
 *
 */
void scheduler_block_current(void)
{
	Cpu *cpu = this_cpu();
	Task *current = cpu->current;

	ASSERT(current != nullptr, "current is null");
	ASSERT(current != cpu->idle, "idle task cannot block");
	ASSERT((Cpu *)current->cpu == cpu, "current task belongs to another "
					   "cpu");
	ASSERT(TASK_RUNNING == current->state, "only running task can block");
	ASSERT(current->waiting_on != nullptr, "current task is not waiting on "
					       "a wait queue");

	current->state = TASK_BLOCKED;
	scheduler_switch();

	/*
	 * execution resumes here when this task
	 * is eventually woken.
	 */
}

/*
 * Wake given task
 */
void scheduler_wake_blocked(Task *blocked)
{
	if (nullptr == blocked) {
		return;
	}

	ASSERT(TASK_BLOCKED == blocked->state, "wait queue contains "
					       "non blocked task");
	ASSERT(blocked->waiting_on == nullptr, "blocked is not waiting on "
					       "any waitqueue");

	blocked->state = TASK_READY;
	scheduler_enqueue(blocked);
}

/*
 * Wake all
 */
void scheduler_wake_all(WaitQueue *wq)
{
	for (;;) {
		Task *blocked = waitqueue_dequeue(wq);
		if (nullptr == blocked) {
			return;
		}

		ASSERT(blocked->state == TASK_BLOCKED, "wait queue contains "
						       "non-blocked task");
		ASSERT(blocked->waiting_on == nullptr ||
			       (WaitQueue *)blocked->waiting_on == wq,
		       "task is waiting on a different wait queue");

		scheduler_enqueue(blocked);
	}
}

static Task *scheduler_pick_next(Cpu *cpu)
{
	if (cpu->needs_cleanup) {
		return cpu->cleanup_task;
	}

	Task *next = scheduler_policy_pick_next(cpu);
	if (next == nullptr) {
		next = cpu->idle;
	}

	return next;
}

void scheduler_switch(void)
{
	/* save current irq state locally in this task's scheduler "context" */
	bool irq_was_enabled = irq_local_is_enable();
	irq_local_disable();

	Cpu *cpu = this_cpu();
	TRACE("schedule cpu = %d", cpu->cpuid);

	Task *prev = cpu->current;

	if (nullptr != prev) {
		ASSERT((Cpu *)prev->cpu == cpu, "current task belongs to "
						"another cpu");
		ASSERT(prev->state != TASK_BLOCKED ||
			       prev->waiting_on != nullptr,
		       "blocked task has nothing waiting on");
		ASSERT(prev->state != TASK_READY || prev->waiting_on == nullptr,
		       "ready task is waiting on a wait queue");

		/*
		 * if the current task is still runnable, put it back on the
		 * run queue before selecting the next task
		 */
		if (prev != cpu->idle && prev != cpu->cleanup_task &&
		    TASK_RUNNING == prev->state) {
			ASSERT(prev->waiting_on == nullptr, "running task is "
							    "waiting "
							    "on a wait queue");
			ASSERT((Cpu *)prev->cpu == cpu, "running task belongs "
							"to "
							"another cpu");

			prev->state = TASK_READY;
			runqueue_enqueue(&cpu->runnable_tasks, prev);
			TRACE("putting prev task (%s) back in runqueue",
			      prev->name);
		}
	}

	/* pick the next task */
	Task *next = scheduler_pick_next(cpu);

	ASSERT((Cpu *)next->cpu == cpu, "selected task belongs to another cpu");
	ASSERT(next->state == TASK_READY || next == cpu->idle ||
		       next == cpu->cleanup_task,
	       "scheduler "
	       "selected non "
	       "ready task");
	ASSERT(next->waiting_on == nullptr, "scheduler selected task waiting "
					    "on a wait queue");

	next->state = TASK_RUNNING;
	cpu->current = next;

	/*
	 * if we are already running the chosen task, there is nothing to do
	 * this happens when the run queue is empty and we're already in idle
	 */
	if (prev == next) {
		ASSERT(next == cpu->idle || next->state == TASK_RUNNING,
		       "current task is not running");
		/* restore this task's irq state after coming back */
		if (irq_was_enabled) {
			irq_local_enable();
		}
		return;
	}

	DEBUG("switching task from = %s to = %s", prev ? prev->name : "nil",
	      next->name);

	if (nullptr != prev) {
		context_switch(&prev->context, &next->context);

		/* restore this task's irq state after coming back */
		if (irq_was_enabled) {
			irq_local_enable();
		}

		/*
		 * we resume execution here when this task is scheduled again.
		 */
		return;
	}

	/*
	 * first task ever scheduled on this cpu
	 */
	ASSERT(next != cpu->idle || scheduler_cpu_has_runnable_tasks() == false,
	       "idle selected while runnable task exists");

	ExecutionContext bootstrap;
	context_switch(&bootstrap, &next->context);

	panic("bootstrap context returned");
}

/*
 * picks cpu with least tasks queued on it
 */
Cpu *scheduler_pick_cpu(void)
{
	Cpu *cpu = nullptr;
	Cpu *best_cpu = nullptr;
	usize best_count = INT64_MAX;
	cpu_foreach(cpu)
	{
		ASSERT(cpu != nullptr, "cpu is null");
		if (!cpu->online) {
			continue;
		}

		usize task_count = runqueue_count(&cpu->runnable_tasks);
		if (task_count < best_count) {
			best_cpu = cpu;
			best_count = task_count;
		}
	}

	ASSERT(best_cpu != nullptr, "no cpu was picked");
	return best_cpu;
}

/*
 * check if current cpu has runnable tasks
 */
bool scheduler_cpu_has_runnable_tasks(void)
{
	Cpu *cpu = this_cpu();
	return runqueue_count(&cpu->runnable_tasks) > 0;
}
