#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

/* spinlock implementation */

#include "common_defs.h"
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

USED static void _spinlock_autocleanup(SpinLock **sp)
{
	if (nullptr != sp && nullptr != *sp) {
		spinlock_release(*sp);
	}
}

#define _SPINLOCK_SCOPED_IMPL(sp, n)           \
	spinlock_acquire((sp));                \
	SpinLock *CONCAT(_spinlock_scoped_, n) \
		__attribute__((cleanup(_spinlock_autocleanup))) = (sp)

/*
 * locks the spinlock and automatically unlocks it at the end of scope.
 * sp : pointer to the spinlock
 */
#define spinlock_acquire_scoped(sp) _SPINLOCK_SCOPED_IMPL(sp, __LINE__)

#define spinlock_assert_locked(sp) \
	ASSERT((sp)->locked, "requires lock = %s to be held", (sp)->name)

#define spinlock_assert_unlocked(sp) \
	ASSERT(!(sp)->locked, "requires lock = %s to be released", (sp)->name)

#endif // _SPINLOCK_H_
