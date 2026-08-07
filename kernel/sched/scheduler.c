#include "sched/scheduler.h"
#include "aarch64/context.h"
#include "core/cpu.h"
#include "debug/assert.h"
#include "sched/scheduler_policy.h"
#include "core/task.h"
#include "debug/log.h"
#include "debug/panic.h"

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

	current->state = TASK_BLOCKED;

	waitqueue_enqueue(wq, current);

	scheduler_switch();

	/*
	 * execution resumes here when this task
	 * is eventually woken.
	 */
}

void scheduler_switch(void)
{
	Cpu *cpu = this_cpu();
	TRACE("schedule cpu = %d", cpu->cpuid);

	Task *prev = cpu->current;

	/*
	 * if the current task is still runnable, put it back on the
	 * run queue before selecting the next task
	 */
	if (nullptr != prev && prev != cpu->idle &&
	    TASK_RUNNING == prev->state) {
		ASSERT(prev->state != TASK_BLOCKED ||
			       prev->waiting_on != nullptr,
		       "task is not blocked but is waiting on something");
		ASSERT(prev->state == TASK_BLOCKED ||
			       prev->waiting_on == nullptr,
		       "task is not blocked but is waiting on something");
		prev->state = TASK_READY;
		runqueue_enqueue(&cpu->runnable_tasks, prev);
		TRACE("putting prev task (%s) back in runqueue", prev->name);
	}

	/* pick the next task */
	Task *next = scheduler_policy_pick_next(cpu);

	if (next == nullptr) {
		next = cpu->idle;
	}

	DEBUG("switching task from = %s to = %s", prev ? prev->name : "nil",
	      next->name);

	ASSERT(next->state != TASK_BLOCKED || next->waiting_on != nullptr,
	       "task is not blocked but is waiting on something");
	ASSERT(next->state == TASK_BLOCKED || next->waiting_on == nullptr,
	       "task is not blocked but is waiting on something");

	next->state = TASK_RUNNING;
	cpu->current = next;

	/*
	 * if we are already running the chosen task, there is nothing to do
	 * this happens when the run queue is empty and we're already in idle
	 */
	if (prev == next) {
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
	 *  first task ever scheduled on this cpu
	 */
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
