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
	spinlock_acquire(&run_queue->lock);
	linkedlist_insert_tail(&run_queue->tasks, task);
	spinlock_release(&run_queue->lock);
}

Task *run_queue_dequeue(RunQueue *run_queue)
{
	spinlock_acquire(&run_queue->lock);
	Task *t = linkedlist_remove_head(&run_queue->tasks);
	spinlock_release(&run_queue->lock);
	return t;
}

Task *run_queue_peek(RunQueue *run_queue)
{
	spinlock_acquire(&run_queue->lock);
	Task *t = linkedlist_peek_head(&run_queue->tasks);
	spinlock_release(&run_queue->lock);
	return t;
}

usize run_queue_count(const RunQueue *run_queue)
{
	return linkedlist_count(&run_queue->tasks);
}
