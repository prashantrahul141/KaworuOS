#ifndef _WAIT_QUEUE_H_
#define _WAIT_QUEUE_H_

#include "core/task.h"
#include "ds/intrusivelist.h"
#include "sync/spinlock.h"

typedef struct {
	SpinLock lock;
	IntrusiveList waiters;
} WaitQueue;

void waitqueue_init(WaitQueue *wq, const i8 *name);

void waitqueue_enqueue(WaitQueue *wq, Task *task);
Task *waitqueue_dequeue(WaitQueue *wq);
void waitqueue_remove(WaitQueue *q, Task *task);
bool waitqueue_is_empty(WaitQueue *q);
usize waitqueue_count(WaitQueue *rq);
Task *waitqueue_peek(WaitQueue *q);

/*
 * the callee should gurantee that it holds the internal lock before doing these
 * operations
 */
void waitqueue_enqueue_unlocked(WaitQueue *wq, Task *task);
Task *waitqueue_dequeue_unlocked(WaitQueue *wq);
void waitqueue_remove_unlocked(WaitQueue *wq, Task *task);
bool waitqueue_is_empty_unlocked(WaitQueue *wq);
usize waitqueue_count_unlocked(WaitQueue *wq);
Task *waitqueue_peek_unlocked(WaitQueue *wq);

#endif // _WAIT_QUEUE_H_
