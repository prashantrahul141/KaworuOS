#include "sync/mutex.h"
#include "core/cpu.h"
#include "debug/assert.h"
#include "sched/scheduler.h"
#include "sync/spinlock.h"
#include "sync/wait_queue.h"

void mutex_init(Mutex *mt, const i8 *name)
{
	spinlock_init(&mt->lock, name);
	waitqueue_init(&mt->waiters, name);
	mt->owner = nullptr;
}

void mutex_acquire(Mutex *mt)
{
	for (;;) {
		spinlock_acquire(&mt->lock);
		Cpu *cpu = this_cpu();
		Task *current = cpu->current;

		/* we can take the lock */
		if (mt->owner == nullptr) {
			mt->owner = current;
			spinlock_release(&mt->lock);
			return;
		}

		/* otherwise just block the task */
		waitqueue_enqueue(&mt->waiters, current);
		spinlock_release(&mt->lock);
		scheduler_block_current();
	}
}

bool mutex_try_acquire(Mutex *mt)
{
	spinlock_acquire(&mt->lock);
	Cpu *cpu = this_cpu();
	Task *current = cpu->current;

	/* we can take the lock */
	if (mt->owner == nullptr) {
		mt->owner = current;
		spinlock_release(&mt->lock);
		return true;
	}

	/* otherwise instead of blocking js return */
	spinlock_release(&mt->lock);
	return false;
}

void mutex_release(Mutex *mt)
{
	spinlock_acquire(&mt->lock);
	Cpu *cpu = this_cpu();
	Task *current = cpu->current;

	ASSERT(mt->owner != nullptr, "mutex (%s) is not acquired",
	       mt->lock.name);
	ASSERT(mt->owner == current,
	       "mutex (%s) is not owned by the task trying to "
	       "free it",
	       mt->lock.name);

	mt->owner = nullptr;

	Task *blocked = waitqueue_dequeue(&mt->waiters);
	spinlock_release(&mt->lock);

	if (nullptr == blocked) {
		return;
	}

	scheduler_wake_blocked(blocked);
}

bool mutex_is_acquired(Mutex *mt)
{
	spinlock_acquire_scoped(&mt->lock);
	return mt->owner != nullptr;
}
