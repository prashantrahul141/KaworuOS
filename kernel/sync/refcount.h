#ifndef _REF_COUNT_H_
#define _REF_COUNT_H_

#include "error.h"
#include <stdatomic.h>

typedef struct {
	atomic_uint count;
} RefCount;

typedef void (*refcount_cleanup_fn_type)(RefCount *rc);

/*
 * initialize reference coounter
 */
void refcount_init(RefCount *rc);

/*
 * increment
 */
void refcount_get(RefCount *rc);

/*
 * decrement, cleanup if needed
 */
errno_t refcount_put(RefCount *rc, refcount_cleanup_fn_type fn);

#endif // _REF_COUNT_H_
