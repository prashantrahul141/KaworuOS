#ifndef _RUN_QUEUE_H_
#define _RUN_QUEUE_H_

#include "core/task.h"
#include "ds/linkedlist.h"
#include "sync/spinlock.h"

typedef struct {
	LinkedList tasks;
	SpinLock lock;
} RunQueue;

void run_queue_init(RunQueue *run_queue);

void run_queue_enqueue(RunQueue *run_queue, Task *task);

Task *run_queue_dequeue(RunQueue *run_queue);

Task *run_queue_peek(RunQueue *run_queue);

usize run_queue_count(const RunQueue *run_queue);

#endif // _RUN_QUEUE_H_
