#include "core/syscall.h"
#include "aarch64/exception.h"
#include "core/syscall_table.h"
#include "error.h"

bool syscall_dispatch(ExceptionFrame *frame)
{
	i64 ret = EOK;
	bool should_resched = false;

	syscall_function_type fn = syscall_table_retrieve(frame->x8);
	if (nullptr == fn) {
		ret = -ENOSYS;
		WARN("unknown syscall: %d", frame->x8);
		return should_resched;
	}

	SyscallReturn syscall_ret = fn(frame);
	ret = syscall_ret.ret;
	should_resched = syscall_ret.should_resched;

	frame->x0 = (u64)ret;
	return should_resched;
}
