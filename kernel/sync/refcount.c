#include "sync/refcount.h"
#include "debug/assert.h"
#include "error.h"
#include <limits.h>
#include <stdatomic.h>

void refcount_init(RefCount *rc)
{
	atomic_store(&rc->count, 1);
}

/*
 * increment
 */
void refcount_get(RefCount *rc)
{
	unsigned int old = atomic_load(&rc->count);

	for (;;) {
		ASSERT(old != UINT_MAX, "reference count overflow");

		if (atomic_compare_exchange_weak(&rc->count, &old, old + 1)) {
			return;
		}
	}
}

/*
 * decrement, cleanup if needed
 * return
 *  - +ve = number of refs left after decrement
 *  - -ve = error
 *  - 0   = file cleaned
 *
 */
errno_t refcount_put(RefCount *rc, refcount_cleanup_fn_type fn)
{
	unsigned int old = atomic_load(&rc->count);

	for (;;) {
		if (old == 0) {
			return -ENOENT;
		}

		if (atomic_compare_exchange_weak(&rc->count, &old, old - 1)) {
			break;
		}
	}

	if (old == 1) {
		fn(rc);
		return EOK;
	}

	return old - 1;
}
