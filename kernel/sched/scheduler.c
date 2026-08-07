#include "sched/scheduler.h"
#include "aarch64/context.h"
#include "core/cpu.h"
#include "debug/assert.h"
#include "sched/scheduler_policy.h"
#include "core/task.h"
#include "debug/log.h"
#include "debug/panic.h"
#include "sync/run_queue.h"
#include "sync/wait_queue.h"
#include <stdint.h>

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
 * Blocks current task
 */
void scheduler_block_current(WaitQueue *wq)
{
	Cpu *cpu = this_cpu();
	Task *current = cpu->current;

	ASSERT(current != nullptr, "current is null");
	ASSERT(current != cpu->idle, "idle task cannot block");
	ASSERT((Cpu *)current->cpu == cpu, "current task belongs to another "
					   "cpu");
	ASSERT(TASK_RUNNING == current->state, "only running task can block");
	ASSERT(current->waiting_on == nullptr, "running task is already "
					       "waiting");

	current->state = TASK_BLOCKED;
	waitqueue_enqueue(wq, current);
	scheduler_switch();

	/*
	 * execution resumes here when this task
	 * is eventually woken.
	 */
}

/*
 * Wake one
 */
void scheduler_wake_one(WaitQueue *wq)
{
	Task *blocked = waitqueue_dequeue(wq);
	if (nullptr == blocked) {
		return;
	}

	ASSERT(TASK_BLOCKED == blocked->state, "wait queue contains "
					       "non blocked task");
	ASSERT(blocked->waiting_on == nullptr ||
		       (WaitQueue *)blocked->waiting_on == wq,
	       "task is waiting on a different wait queue");

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

void scheduler_switch(void)
{
	Cpu *cpu = this_cpu();
	TRACE("schedule cpu = %d", cpu->cpuid);

	Task *prev = cpu->current;

	if (nullptr != prev) {
		ASSERT((Cpu *)prev->cpu == cpu, "current task belongs to "
						"another cpu");

		ASSERT(prev->state != TASK_BLOCKED ||
			       prev->waiting_on != nullptr,
		       "blocked task is not waiting on a wait queue");

		ASSERT(prev->state == TASK_BLOCKED ||
			       prev->waiting_on == nullptr,
		       "non blocked task is waiting on a wait queue");

		ASSERT(prev->state != TASK_READY || prev->waiting_on == nullptr,
		       "ready task is waiting on a wait queue");
	}

	/*
	 * if the current task is still runnable, put it back on the
	 * run queue before selecting the next task
	 */
	if (nullptr != prev && prev != cpu->idle &&
	    TASK_RUNNING == prev->state) {
		ASSERT(prev->waiting_on == nullptr, "running task is waiting "
						    "on a wait queue");
		ASSERT((Cpu *)prev->cpu == cpu, "running task belongs to "
						"another cpu");

		prev->state = TASK_READY;
		runqueue_enqueue(&cpu->runnable_tasks, prev);
		TRACE("putting prev task (%s) back in runqueue", prev->name);
	}

	/* pick the next task */
	Task *next = scheduler_policy_pick_next(cpu);
	if (next == nullptr) {
		next = cpu->idle;
	}

	ASSERT((Cpu *)next->cpu == cpu, "selected task belongs to another cpu");
	ASSERT(next->state == TASK_READY || next == cpu->idle, "scheduler "
							       "selected non "
							       "ready task");
	ASSERT(next->waiting_on == nullptr, "scheduler selected task waiting "
					    "on a wait queue");

	DEBUG("switching task from = %s to = %s", prev ? prev->name : "nil",
	      next->name);

	next->state = TASK_RUNNING;
	cpu->current = next;

	/*
	 * if we are already running the chosen task, there is nothing to do
	 * this happens when the run queue is empty and we're already in idle
	 */
	if (prev == next) {
		ASSERT(next == cpu->idle || next->state == TASK_RUNNING,
		       "current task is not running");
		return;
	}

	if (nullptr != prev) {
		context_switch(&prev->context, &next->context);

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
