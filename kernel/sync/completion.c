#include "completion.h"
#include "core/task.h"
#include "debug/assert.h"
#include "sched/scheduler.h"

void completion_init(Completion *cm, const i8 *name)
{
	waitqueue_init(&cm->waiters, name);
	cm->done = false;
}

void completion_wait(Completion *cm)
{
	for (;;) {
		spinlock_acquire(&cm->waiters.lock);

		/* if we are done, return*/
		if (cm->done) {
			spinlock_release(&cm->waiters.lock);
			return;
		}

		Task *cur = task_get_current();
		ASSERT(!IS_ERR(cur), "current task is null");

		/* otherwise block the current task */
		cur->state = TASK_BLOCKED;
		spinlock_release(&cm->waiters.lock);
		waitqueue_enqueue(&cm->waiters, cur);
		scheduler_switch();
	}
}

void completion_signal(Completion *cm)
{
	ASSERT(!cm->done, "completion already marked done");
	spinlock_acquire_scoped(&cm->waiters.lock);
	cm->done = true;
	Task *task;

	while ((task = waitqueue_dequeue_unlocked(&cm->waiters)) != nullptr) {
		scheduler_wake_blocked(task);
	}
}

void completion_reset(Completion *cm)
{
	spinlock_acquire_scoped(&cm->waiters.lock);
	cm->done = false;
}
