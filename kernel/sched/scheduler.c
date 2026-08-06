#include "sched/scheduler.h"
#include "aarch64/context.h"
#include "core/cpu.h"
#include "sched/scheduler_policy.h"
#include "core/task.h"
#include "debug/log.h"
#include "debug/panic.h"

void yield(void)
{
	scheduler_switch();
}

void scheduler_switch(void)
{
	Cpu *cpu = this_cpu();
	DEBUG("schedule cpu = %d", cpu->cpuid);

	Task *prev = cpu->current;

	/*
	 * if the current task is still runnable, put it back on the
	 * run queue before selecting the next task
	 */
	if (nullptr != prev && prev != cpu->idle &&
	    TASK_RUNNING == prev->state) {
		prev->state = TASK_READY;
		intrusivelist_insert_tail(&cpu->runnable_tasks,
					  &prev->runnable_node);
	}

	/* pick the next task */
	Task *next = scheduler_policy_pick_next(cpu);

	if (next == nullptr) {
		next = cpu->idle;
	}

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
