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
bool waitqueue_is_empty(const WaitQueue *q);
Task *waitqueue_peek(const WaitQueue *q);

#endif // _WAIT_QUEUE_H_
