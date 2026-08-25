#include "syscall/sys_sched.h"

SYSCALL_DEFINE0(yield, frame)
{
	DEBUG("YIELD: ");
	UNUSED_ARG(frame);
	return (SyscallReturn){ .should_resched = true, .ret = EOK };
}
