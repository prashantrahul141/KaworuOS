#include "core/process.h"
#include "core/cpu.h"
#include "core/syscall_table.h"
#include "core/syscall.h"
#include "debug/assert.h"
#include "mm/address_space.h"
#include "core/task.h"
#include "sync/spinlock.h"

void process_init(Process *proc, usize pid, const i8 *name, AddressSpace *as)
{
	proc->pid = pid;
	proc->name = name;

	spinlock_init(&proc->lock, name);

	proc->state = PROCESS_RUNNING;
	proc->address_space = as;
	intrusivelist_init(&proc->threads);

	proc->exiting = false;
	proc->exit_code = 0;
}

void process_add_thread(Process *proc, Task *thread)
{
	spinlock_acquire_scoped(&proc->lock);
	intrusivelist_insert_tail(&proc->threads, &thread->process_node);
}

void process_remove_thread(Process *proc, Task *thread)
{
	spinlock_acquire_scoped(&proc->lock);
	intrusivelist_remove(&proc->threads, &thread->process_node);
}

usize process_thread_count(Process *proc)
{
	spinlock_acquire_scoped(&proc->lock);
	return intrusivelist_count(&proc->threads);
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
