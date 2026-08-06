#include "sched/scheduler_policy.h"
#include "common_defs.h"
#include "core/cpu.h"
#include "ds/intrusivelist.h"

static Task *sched_policy_round_robin(IntrusiveList *queue, Task *current)
{
	UNUSED_ARG(current);
	IntrusiveNode *node = intrusivelist_remove_head(queue);
	return container_of(node, Task, runnable_node);
}

Task *scheduler_policy_pick_next(Cpu *cpu)
{
	if (intrusivelist_is_empty(&cpu->runnable_tasks)) {
		return nullptr;
	}

#ifdef CONFIG_SCHEDULER_POLICY_ROUND_ROBIN
	return sched_policy_round_robin(&cpu->runnable_tasks, cpu->current);
#endif
}
