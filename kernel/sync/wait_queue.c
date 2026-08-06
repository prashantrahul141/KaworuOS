#include "sync/wait_queue.h"
#include "common_defs.h"
#include "core/task.h"
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
	intrusivelist_insert_tail(&wq->waiters, &task->wait_node);
	task->waiting_on = (void *)wq;
}

Task *waitqueue_dequeue(WaitQueue *wq)
{
	spinlock_acquire_scoped(&wq->lock);
	IntrusiveNode *wait_node = intrusivelist_remove_head(&wq->waiters);
	if (nullptr == wait_node) {
		return nullptr;
	}
	Task *waiting_task = container_of(wait_node, Task, wait_node);
	waiting_task->waiting_on = nullptr;
	return waiting_task;
}

void waitqueue_remove(WaitQueue *wq, Task *task)
{
	spinlock_acquire_scoped(&wq->lock);
	intrusivelist_remove(&wq->waiters, &task->wait_node);
}

bool waitqueue_is_empty(const WaitQueue *wq)
{
	return intrusivelist_is_empty(&wq->waiters);
}

Task *waitqueue_peek(const WaitQueue *wq)
{
	IntrusiveNode *wait_node = intrusivelist_peek_head(&wq->waiters);
	if (nullptr == wait_node) {
		return nullptr;
	}
	Task *waiting_task = container_of(wait_node, Task, wait_node);
	return waiting_task;
}
