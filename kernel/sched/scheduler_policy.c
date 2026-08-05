#include "sched/scheduler_policy.h"
#include "common_defs.h"
#include "core/cpu.h"
#include "sched/run_queue.h"

static Task *sched_policy_round_robin(RunQueue *queue, Task *current)
{
	UNUSED_ARG(current);
	return run_queue_dequeue(queue);
}

Task *scheduler_policy_pick_next(Cpu *cpu)
{
#ifdef CONFIG_SCHEDULER_POLICY_ROUND_ROBIN
	return sched_policy_round_robin(&cpu->tasks, cpu->current);
#endif
}
