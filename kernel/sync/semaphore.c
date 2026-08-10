#include "sync/semaphore.h"
#include "core/cpu.h"
#include "debug/assert.h"
#include "sched/scheduler.h"

void semaphore_init(Semaphore *sm, usize count, const i8 *name)
{
	spinlock_init(&sm->lock, name);
	waitqueue_init(&sm->waiters, name);
	sm->count = count;
	sm->__initial_count = count;
}

void semaphore_acquire(Semaphore *sm)
{
	for (;;) {
		spinlock_acquire(&sm->lock);
		Cpu *cpu = this_cpu();
		Task *current = cpu->current;

		/* we can take the lock */
		if (sm->count > 0) {
			sm->count--;
			spinlock_release(&sm->lock);
			return;
		}

		/* otherwise just block the task */
		waitqueue_enqueue(&sm->waiters, current);
		spinlock_release(&sm->lock);
		scheduler_block_current();
	}
}

bool semaphore_try_acquire(Semaphore *sm)
{
	spinlock_acquire(&sm->lock);

	/* we can take the lock */
	if (sm->count > 0) {
		sm->count--;
		spinlock_release(&sm->lock);
		return true;
	}

	/* otherwise instead of blocking js return */
	spinlock_release(&sm->lock);
	return false;
}

void semaphore_release(Semaphore *sm)
{
	spinlock_acquire(&sm->lock);
	ASSERT(sm->count < sm->__initial_count,
	       "no one has held this "
	       "semaphore (%s)",
	       sm->lock.name);

	sm->count++;

	Task *blocked = waitqueue_dequeue(&sm->waiters);
	spinlock_release(&sm->lock);

	if (nullptr == blocked) {
		return;
	}

	scheduler_wake(blocked);
}
