#include "core/syscall.h"
#include "core/process.h"
#include "error.h"
#include "io/io.h"

bool syscall_dispatch(ExceptionFrame *frame)
{
	i64 ret = 0;
	bool should_resched = false;
	switch (frame->x8) {
	case SYS_READ:
	case SYS_WRITE: {
		// x0 contains fd
		// x1 contains buf
		// x2 contains buf count
		IOEvent e = { .bg = IO_DEFAULT_COLOR_BG,
			      .fg = IO_DEFAULT_COLOR_FG,
			      .msg = (i8 *)frame->x1,
			      .len = frame->x2 };
		console_write(e);
		break;
	}

	case SYS_OPEN:
	case SYS_CLOSE:
	case SYS_CLONE:
	case SYS_FORK:
	case SYS_VFORK:
	case SYS_EXECVE:
	case SYS_WAIT4:
	case SYS_KILL:
	case SYS_UNAME:
		break;
	case SYS_YIELD:
		should_resched = true;
		break;
	case SYS_EXIT:
		sys_exit((i32)frame->x0);
		break;
	default:
		ret = -ENOSYS;
		WARN("unknown syscall: %d", frame->x8);
	}

	frame->x0 = (u64)ret;
	return should_resched;
}
