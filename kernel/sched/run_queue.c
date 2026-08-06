#include "core/task.h"
#include "sched/run_queue.h"
#include "ds/linkedlist.h"

void run_queue_init(RunQueue *run_queue)
{
	spinlock_init(&run_queue->lock, "RunQueue");
	linkedlist_init(&run_queue->tasks);
}

void run_queue_enqueue(RunQueue *run_queue, Task *task)
{
	spinlock_acquire_scoped(&run_queue->lock);
	linkedlist_insert_tail(&run_queue->tasks, task);
}

Task *run_queue_dequeue(RunQueue *run_queue)
{
	spinlock_acquire_scoped(&run_queue->lock);
	return linkedlist_remove_head(&run_queue->tasks);
}

Task *run_queue_peek(RunQueue *run_queue)
{
	spinlock_acquire_scoped(&run_queue->lock);
	return linkedlist_peek_head(&run_queue->tasks);
}

usize run_queue_count(const RunQueue *run_queue)
{
	return linkedlist_count(&run_queue->tasks);
}
