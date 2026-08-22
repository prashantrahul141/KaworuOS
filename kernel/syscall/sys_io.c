#include "syscall/sys_io.h"
#include "io/io.h"
#include "mm/kheap.h"
#include "mm/uaccess.h"
#include "syscall/syscall_table.h"

SYSCALL_DEFINE3(write, frame, i64, fd, i8 *, buf, usize, count)
{
	DEBUG("SYS_WRITE: fd = %d buf = %p count = %d", fd, buf, count);

	UNUSED_ARG(frame);
	UNUSED_ARG(fd);

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

	IOEvent ev = io_event_default(buf, count);
	ret = console_write(ev);
	if (EOK == ret) {
		ret = (i64)count;
	}

	kfree(kernel_buf);
	return (SyscallReturn){ .should_resched = false, .ret = ret };
}
