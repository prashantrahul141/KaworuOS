#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "core/cpu.h"
#include "core/task.h"
#include "sync/wait_queue.h"

/*
 * init
 */
void scheduler_init(void);

/*
 * Add task to runnable
 */
void scheduler_enqueue(Task *task);

/*
 * Remove task from runnable
 */
void scheduler_dequeue(Task *task);

/*
 * Blocks current task
 */
void scheduler_block_current(WaitQueue *wq);

/*
 * Wake one
 */
void scheduler_wake_one(WaitQueue *wq);

/*
 * Wake all
 */
void scheduler_wake_all(WaitQueue *wq);

/*
 * switch task to the next READY one.
 */
void scheduler_switch(void);

/*
 * wrapper around scheduler_switch
 */
void yield(void);

/*
 * picks cpu with least tasks queued on it
 */
Cpu *scheduler_pick_cpu(void);

#endif // _SCHEDULER_H_
