#include "mm/uaccess.h"
#include "core/cpu.h"
#include "core/process.h"
#include "core/task.h"
#include "debug/assert.h"
#include "error.h"
#include "memlayout.h"
#include "mm/paging.h"
#include "string.h"

static bool is_within_user_vm_range(const void *ptr, usize size)
{
	usize start = (usize)ptr;

	if (start < USER_VM_RANGE_BASE) {
		return false;
	}

	if (start > USER_VM_RANGE_END - size) {
		return false;
	}

	return true;
}

static inline bool is_page_writable_by_user(PageDescriptor *pde)
{
	return pde->field.ap == EL1_READ_WRITE_EL0_READ_WRITE;
}

static inline bool is_page_readable_by_user(PageDescriptor *pde)
{
	return pde->field.ap == EL1_READ_WRITE_EL0_READ_WRITE ||
	       pde->field.ap == EL1_READ_ONLY_EL0_READ_ONLY;
}

/*
 * Verifies if the given pointer (and its range) is within the safe user VM
 * range
 */
static bool user_range_valid(const void *ptr, usize size,
			     bool required_user_writable,
			     bool required_user_readable)

{
	if (!is_within_user_vm_range(ptr, size)) {
		return false;
	}

	Task *current = this_cpu()->current;
	ASSERT(current != nullptr, "task cant be null");

	Process *proc = (Process *)current->process;
	ASSERT(proc != nullptr, "proc cant be null");

	usize end = (usize)ptr + size - 1;
	for (usize va = round_down(end, PAGE_SIZE); va <= end;
	     va += PAGE_SIZE) {
		PageDescriptor *pde =
			paging_lookup_desc(proc->address_space->table, va);

		/* verify if its within the mapped pages */
		if (0 == paging_page_to_pa(pde, va)) {
			return false;
		}

		/* verify perms */
		if (required_user_writable && !is_page_writable_by_user(pde)) {
			return false;
		}

		if (required_user_readable && !is_page_readable_by_user(pde)) {
			return false;
		}

		if (va > end - PAGE_SIZE) {
			break;
		}
	}

	return true;
}

/*
 * copy user data to kernel memory
 */
errno_t copy_from_user(void *kernel_dst, const void *user_src, usize size)
{
	if (0 == size) {
		return EOK;
	}

	/* should atleast have read perms on the page */
	if (!user_range_valid(user_src, size, false, true)) {
		return -EFAULT;
	}

	memcpy(kernel_dst, user_src, size);

	return EOK;
}

/*
 * copy kernel data to user memory
 */
errno_t copy_to_user(void *user_dst, const void *kernel_src, usize size)
{
	if (0 == size) {
		return EOK;
	}

	/* should have write perms on the page */
	if (!user_range_valid(user_dst, size, true, false)) {
		return -EFAULT;
	}

	memcpy(user_dst, kernel_src, size);

	return EOK;
}

/*
 *
 * wrapper around copy_from_user
 */
errno_t copy_from_user_u8(u8 *dst, const void *src)
{
	return copy_from_user(dst, src, sizeof(u8));
}

errno_t copy_from_user_u32(u32 *dst, const void *src)
{
	return copy_from_user(dst, src, sizeof(*dst));
}

errno_t copy_from_user_u64(u64 *dst, const void *src)
{
	return copy_from_user(dst, src, sizeof(*dst));
}

/* wrapper around copy_to_user */
errno_t copy_to_user_u8(u8 *dst, const u8 value)
{
	return copy_to_user(dst, &value, sizeof(value));
}

errno_t copy_to_user_u32(u32 *dst, const u32 value)
{
	return copy_to_user(dst, &value, sizeof(value));
}

errno_t copy_to_user_u64(u64 *dst, const u64 value)
{
	return copy_to_user(dst, &value, sizeof(value));
}

errno_t copy_from_user_string(i8 *dst, const void *src, usize max_len)
{
	if (0 == max_len) {
		return EOK;
	}

	const i8 *_src = src;
	for (usize index = 0; index < max_len; index++) {
		usize index_va = index + (usize)src;
		if (!user_range_valid((void *)index_va, max_len, false, true)) {
			return -EFAULT;
		}

		dst[index] = _src[index];

		if ('\0' == dst[index]) {
			return EOK;
		}
	}

	return -ENAMETOOLONG;
}
