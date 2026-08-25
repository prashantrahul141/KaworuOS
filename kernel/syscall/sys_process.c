#include "syscall/sys_process.h"
#include "core/cpu.h"
#include "core/process.h"
#include "core/process_manager.h"
#include "core/task.h"
#include "debug/assert.h"

SYSCALL_DEFINE0(fork, frame)
{
	DEBUG("FORK: ");

	Task *task = task_get_current();
	ASSERT(!IS_ERR(task), "task cant be null");

	Process *proc = process_get_current();
	ASSERT(!IS_ERR(proc), "proc cant be null");

	Process *new_proc =
		proc_manager_create_exec_child_from(proc, task, frame);
	if (IS_ERR(new_proc)) {
		return (SyscallReturn){ .ret = PTR_TO_ERR(new_proc),
					.should_resched = false };
	}

	return (SyscallReturn){ .ret = (i64)new_proc->pid,
				.should_resched = true };
}

SYSCALL_DEFINE1(exit, frame, i64, status)
{
	DEBUG("EXIT: status = %d", status);
	UNUSED_ARG(frame);

	Task *task = task_get_current();

	Process *proc = process_get_current();
	process_exit(proc, status);

	DEBUG("exiting task = %s with status code = %d", proc->name, status);
	task_exit(task);
	UNREACHABLE();
}

SYSCALL_DEFINE0(getpid, frame)
{
	DEBUG("GETPID: ");
	UNUSED_ARG(frame);
	const Task *task = task_get_current();
	ASSERT(!IS_ERR(task), "task cant be null");

	const Process *proc = process_get_current();
	ASSERT(!IS_ERR(proc), "proc cant be null");

	return (SyscallReturn){ .ret = (i64)proc->pid,
				.should_resched = false };
}

SYSCALL_DEFINE0(getppid, frame)
{
	DEBUG("GETPPID: ");
	UNUSED_ARG(frame);
	const Task *task = task_get_current();
	ASSERT(!IS_ERR(task), "task cant be null");

	const Process *proc = (Process *)task->process;
	ASSERT(!IS_ERR(proc), "proc cant be null");

	/* this can be null */
	const Process *parent_proc = (Process *)proc->parent;

	DEBUG("parent_proc = %p", parent_proc);

	i64 ret = parent_proc == nullptr ? 0 : (i64)parent_proc->pid;

	return (SyscallReturn){ ret, .should_resched = false };
}
