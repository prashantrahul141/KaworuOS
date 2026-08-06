#include "sched/scheduler_policy.h"
#include "common_defs.h"
#include "core/cpu.h"
#include "sync/run_queue.h"

static Task *sched_policy_round_robin(RunQueue *queue, Task *current)
{
	UNUSED_ARG(current);
	return runqueue_dequeue(queue);
}

Task *scheduler_policy_pick_next(Cpu *cpu)
{
	if (runqueue_is_empty(&cpu->runnable_tasks)) {
		return nullptr;
	}

#ifdef CONFIG_SCHEDULER_POLICY_ROUND_ROBIN
	return sched_policy_round_robin(&cpu->runnable_tasks, cpu->current);
#endif
}
