#include "sync/run_queue.h"
#include "core/task.h"
#include "debug/assert.h"
#include "ds/intrusivelist.h"
#include "sync/spinlock.h"

void runqueue_init(RunQueue *rq, const i8 *name)
{
	spinlock_init(&rq->lock, name);
	intrusivelist_init(&rq->runnables);
}

void runqueue_enqueue(RunQueue *rq, Task *task)
{
	ASSERT(task->state == TASK_READY, "only READY tasks may enter "
					  "runqueue");
	spinlock_acquire_scoped(&rq->lock);
	intrusivelist_insert_tail(&rq->runnables, &task->runnable_node);
}

Task *runqueue_dequeue(RunQueue *rq)
{
	spinlock_acquire_scoped(&rq->lock);
	IntrusiveNode *run_node = intrusivelist_remove_head(&rq->runnables);
	if (nullptr == run_node) {
		return nullptr;
	}
	return container_of(run_node, Task, runnable_node);
}

void runqueue_remove(RunQueue *rq, Task *task)
{
	spinlock_acquire_scoped(&rq->lock);
	intrusivelist_remove(&rq->runnables, &task->runnable_node);
}

bool runqueue_is_empty(RunQueue *rq)
{
	spinlock_acquire_scoped(&rq->lock);
	return intrusivelist_is_empty(&rq->runnables);
}

usize runqueue_count(RunQueue *rq)
{
	spinlock_acquire_scoped(&rq->lock);
	return intrusivelist_count(&rq->runnables);
}

Task *runqueue_peek(RunQueue *rq)
{
	spinlock_acquire_scoped(&rq->lock);
	IntrusiveNode *run_node = intrusivelist_peek_head(&rq->runnables);
	if (nullptr == run_node) {
		return nullptr;
	}
	Task *runnable_node = container_of(run_node, Task, runnable_node);
	return runnable_node;
}
