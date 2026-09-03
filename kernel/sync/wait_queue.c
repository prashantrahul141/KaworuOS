#include "sync/wait_queue.h"
#include "common_defs.h"
#include "core/task.h"
#include "debug/assert.h"
#include "ds/intrusivelist.h"
#include "sync/spinlock.h"

void waitqueue_init(WaitQueue *wq, const i8 *name)
{
	spinlock_init(&wq->lock, name);
	intrusivelist_init(&wq->waiters);
}

void waitqueue_enqueue(WaitQueue *wq, Task *task)
{
	spinlock_acquire_scoped(&wq->lock);
	waitqueue_enqueue_unlocked(wq, task);
}

Task *waitqueue_dequeue(WaitQueue *wq)
{
	spinlock_acquire_scoped(&wq->lock);
	return waitqueue_dequeue_unlocked(wq);
}

void waitqueue_remove(WaitQueue *wq, Task *task)
{
	spinlock_acquire_scoped(&wq->lock);
	waitqueue_remove_unlocked(wq, task);
}

bool waitqueue_is_empty(WaitQueue *wq)
{
	spinlock_acquire_scoped(&wq->lock);
	return waitqueue_is_empty_unlocked(wq);
}

usize waitqueue_count(WaitQueue *wq)
{
	spinlock_acquire_scoped(&wq->lock);
	return waitqueue_count_unlocked(wq);
}

Task *waitqueue_peek(WaitQueue *wq)
{
	spinlock_acquire_scoped(&wq->lock);
	return waitqueue_peek_unlocked(wq);
}

void waitqueue_enqueue_unlocked(WaitQueue *wq, Task *task)
{
	ASSERT(task->state != TASK_READY, "only NOT READY tasks may enter "
					  "waitqueue");
	spinlock_assert_locked(&wq->lock);
	intrusivelist_insert_tail(&wq->waiters, &task->wait_node);
	task->waiting_on = (void *)wq;
}

Task *waitqueue_dequeue_unlocked(WaitQueue *wq)
{
	spinlock_assert_locked(&wq->lock);
	IntrusiveNode *wait_node = intrusivelist_remove_head(&wq->waiters);
	if (nullptr == wait_node) {
		return nullptr;
	}
	Task *waiting_task = container_of(wait_node, Task, wait_node);
	waiting_task->waiting_on = nullptr;
	return waiting_task;
}

void waitqueue_remove_unlocked(WaitQueue *wq, Task *task)
{
	spinlock_assert_locked(&wq->lock);
	intrusivelist_remove(&wq->waiters, &task->wait_node);
}

bool waitqueue_is_empty_unlocked(WaitQueue *wq)
{
	spinlock_assert_locked(&wq->lock);
	return intrusivelist_is_empty(&wq->waiters);
}

usize waitqueue_count_unlocked(WaitQueue *wq)
{
	spinlock_assert_locked(&wq->lock);
	return intrusivelist_count(&wq->waiters);
}

Task *waitqueue_peek_unlocked(WaitQueue *wq)
{
	spinlock_assert_locked(&wq->lock);
	IntrusiveNode *wait_node = intrusivelist_peek_head(&wq->waiters);
	if (nullptr == wait_node) {
		return nullptr;
	}
	Task *waiting_task = container_of(wait_node, Task, wait_node);
	return waiting_task;
}
