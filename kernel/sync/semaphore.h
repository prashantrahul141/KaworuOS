#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "sync/wait_queue.h"

typedef struct {
	SpinLock lock;
	WaitQueue waiters;
	usize count;

	/* for internal use */
	usize __initial_count;
} Semaphore;

void semaphore_init(Semaphore *sm, usize count, const i8 *name);

void semaphore_acquire(Semaphore *sm);

bool semaphore_try_acquire(Semaphore *sm);

void semaphore_release(Semaphore *sm);

#endif // _SEMAPHORE_H_
