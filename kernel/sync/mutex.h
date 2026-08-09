#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "sync/spinlock.h"
#include "sync/wait_queue.h"

typedef struct {
	SpinLock lock;
	Task *owner;
	WaitQueue waiters;
} Mutex;

void mutex_init(Mutex *mt, const i8 *name);

void mutex_acquire(Mutex *mt);

bool mutex_try_acquire(Mutex *mt);

void mutex_release(Mutex *mt);

bool mutex_is_acquired(Mutex *mt);

#endif // _MUTEX_H_
