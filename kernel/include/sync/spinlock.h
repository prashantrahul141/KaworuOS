#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

/* spinlock implementation */

#include "types.h"
#include "config.h"

typedef struct {
	/* is spinlock locked */
	bool locked;

	/* name of this spinlock, for debugging. */
	const i8 *name;

	/* which cpu has acquired this lock*/
	void *cpu;
} SpinLock;

void spinlock_init(SpinLock *sp, const i8 *name);
void spinlock_acquire(SpinLock *sp);
void spinlock_release(SpinLock *sp);

#endif // _SPINLOCK_H_
