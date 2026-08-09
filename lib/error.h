#ifndef _ERROR_H_
#define _ERROR_H_

#include "common_defs.h"
#include "types.h"

#define ERR_LIST                         \
	X(EOK) /* ok! */                 \
	X(ENOMEM) /* no memory */        \
	X(EINVAL) /* invalid argument */ \
	X(ENODEV) /* no such device */   \
	X(ENOENT) /* no such entity */   \
	X(EBUSY) /* in use */            \
	X(EPERM) /* no perms */          \
	X(ENOSYS) /* not implemented */  \
	X(EFAULT) /* bad address? */     \
	X(EMAXERR) /* max count for errors */

#define X(i) i,
typedef isize errno_t;
enum : i32 { ERR_LIST };
#undef X

/*
 * Returns a string for the given error value.
 * The value could be negative in which case its magnitude will be taken.
 */
const i8 *str_err(isize err);

/*
 * Returns a string for the given error value.
 * Its the calle's responsibility to make sure the error code does exist.
 */
const i8 *str_err_raw(errno_t err);

/*
 * converting the value to u64 gives a small optimisation.
 * Any value between -1 to -EMAXERR will roll to (U64::MAX - value) which are
 * invalid pointer values in themselves, so we dont have to do double comparison
 * to check wehther the value lies between -1 and -EMAXERR, single comparison is
 * enough.
 */
#define IS_ERR_VALUE(x) unlikely((u64)(void *)(x) >= (u64) - EMAXERR)

/*
 * check whether the given value is an error or not
 */
static inline MUST_CHECK bool IS_ERR(const void *value)
{
	// NOLINTNEXTLINE
	return IS_ERR_VALUE((u64)value);
}

/*
 * Converts a negative error value to an error pointer
 */
static inline MUST_CHECK void *ERR_TO_PTR(isize error)
{
	return (void *)error;
}

/*
 * Converts a pointer value to an error
 */
static inline MUST_CHECK isize PTR_TO_ERR(const void *ptr)
{
	return (isize)ptr;
}

/*
 * helper for this pattern:
 *      if (IS_ERR(ptr))
 *          return PTR_TO_ERR(ptr);
 *      else
 *          return EOK;
 *
 */
static inline isize MUST_CHECK PTR_ERR_OR_OK(const void *ptr)
{
	if (IS_ERR(ptr))
		return PTR_TO_ERR(ptr);
	else
		return EOK;
}

#endif // _ERROR_H_
