#include "core/syscall.h"
#include "aarch64/exception.h"
#include "core/syscall_table.h"
#include "error.h"

bool syscall_dispatch(ExceptionFrame *frame)
{
	DEBUG("dispatching: %d", frame->x8);
	syscall_function_type fn = syscall_table_retrieve(frame->x8);
	if (nullptr == fn) {
		frame->x0 = (u64)-ENOSYS;
		WARN("unknown syscall: %d", frame->x8);
		return false;
	}

	SyscallReturn syscall_ret = fn(frame);
	frame->x0 = (u64)syscall_ret.ret;
	return syscall_ret.should_resched;
}
