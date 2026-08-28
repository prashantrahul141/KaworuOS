#include "syscall/sys_io.h"
#include "core/process.h"
#include "debug/assert.h"
#include "mm/kheap.h"
#include "mm/uaccess.h"
#include "syscall/syscall_table.h"

SYSCALL_DEFINE3(write, frame, i64, fd, i8 *, buf, usize, count)
{
	DEBUG("SYS_WRITE: fd = %d buf = %p count = %d", fd, buf, count);
	UNUSED_ARG(frame);

	if (0 == count) {
		return (SyscallReturn){ .should_resched = false, .ret = EOK };
	}

	i8 *kernel_buf = kalloc(count);
	if (IS_ERR(kernel_buf)) {
		WARN("failed allocating while write syscall: %d", count);
		return (SyscallReturn){ .should_resched = false,
					.ret = -ENOMEM };
	}

	errno_t ret = copy_from_user(kernel_buf, buf, count);
	if (EOK != ret) {
		WARN("failed copying from user while write syscall: %p", buf);
		kfree(kernel_buf);
		return (SyscallReturn){ .should_resched = false, .ret = ret };
	}

	Process *proc = process_get_current();
	ASSERT(!IS_ERR(proc), "proc is null");

	File *file = fdtable_get_file(proc->files, (usize)fd);
	if (IS_ERR(file)) {
		kfree(kernel_buf);
		WARN("no fd = %d found", fd);
		return (SyscallReturn){ .should_resched = false,
					.ret = PTR_TO_ERR(file) };
	}

	i64 written = file_write(file, kernel_buf, count);

	file_put(file);
	kfree(kernel_buf);

	return (SyscallReturn){ .should_resched = false, .ret = written };
}
