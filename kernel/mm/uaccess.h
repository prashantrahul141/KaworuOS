#ifndef _UACCESS_H_
#define _UACCESS_H_

#include "error.h"
#include "types.h"

/*
 * copy user data to kernel memory
 */
errno_t copy_from_user(void *kernel_dst, const void *user_src, usize size);

/*
 * copy kernel data to user memory
 */
errno_t copy_to_user(void *user_dst, const void *kernel_src, usize size);

/*
 *
 * wrapper around copy_from_user
 */
errno_t copy_from_user_u8(u8 *dst, const void *src);
errno_t copy_from_user_u32(u32 *dst, const void *src);
errno_t copy_from_user_u64(u64 *dst, const void *src);

/* wrapper around copy_to_user */
errno_t copy_to_user_u8(u8 *dst, const u8 value);
errno_t copy_to_user_u32(u32 *dst, const u32 value);
errno_t copy_to_user_u64(u64 *dst, const u64 value);

/*
 * for strings
 */
errno_t copy_from_user_string(i8 *dst, const void *src, usize max_len);

#endif // _UACCESS_H_
