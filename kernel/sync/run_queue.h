#ifndef _RUN_QUEUE_H
#define _RUN_QUEUE_H

#include "core/task.h"
#include "ds/intrusivelist.h"
#include "sync/spinlock.h"

typedef struct {
	SpinLock lock;
	IntrusiveList runnables;
} RunQueue;

void runqueue_init(RunQueue *rq, const i8 *name);
void runqueue_enqueue(RunQueue *rq, Task *task);
Task *runqueue_dequeue(RunQueue *rq);
void runqueue_remove(RunQueue *rq, Task *task);
bool runqueue_is_empty(RunQueue *rq);
usize runqueue_count(RunQueue *rq);
Task *runqueue_peek(RunQueue *rq);

#endif // _RUN_QUEUE_H
