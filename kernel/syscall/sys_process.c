#include "syscall/sys_process.h"
#include "core/cpu.h"
#include "core/process.h"
#include "debug/assert.h"

SYSCALL_DEFINE1(exit, i64, status)
{
	Cpu *cpu = this_cpu();
	Task *task = cpu->current;

	Process *proc = (Process *)task->process;
	process_exit(proc, status);

	DEBUG("exiting task = %s with status code = %d", proc->name, status);
	task_exit(task);
	UNREACHABLE();
}

SYSCALL_DEFINE0(yield)
{
	return (SyscallReturn){ .should_resched = true, .ret = EOK };
}

SYSCALL_DEFINE0(getpid)
{
	Task *task = this_cpu()->current;
	ASSERT(task != nullptr, "task cant be null");

	Process *proc = (Process *)task->process;
	ASSERT(proc != nullptr, "proc cant be null");

	return (SyscallReturn){ .ret = (i64)proc->pid,
				.should_resched = false };
}

SYSCALL_DEFINE0(getppid)
{
	Task *task = this_cpu()->current;
	ASSERT(task != nullptr, "task cant be null");

	Process *proc = (Process *)task->process;
	ASSERT(proc != nullptr, "proc cant be null");

	/* this can be null */
	Process *parent_proc = (Process *)proc->parent;

	DEBUG("parent_proc = %p", parent_proc);

	i64 ret = parent_proc == nullptr ? 0 : (i64)parent_proc->pid;

	return (SyscallReturn){ ret, .should_resched = false };
}
