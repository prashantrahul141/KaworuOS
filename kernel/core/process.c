#include "core/process.h"
#include "core/cpu.h"
#include "mm/address_space.h"
#include "core/task.h"
#include "sync/spinlock.h"
#include "core/syscall.h"

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

NORETURN SYSCALL_DEFINE1(exit, i32, status)
{
	Cpu *cpu = this_cpu();
	Task *task = cpu->current;

	Process *proc = (Process *)task->process;
	proc->exit_code = status;
	proc->exiting = true;
	proc->state = PROCESS_ZOMBIE;

	INFO("exiting task = %s with status code = %d", proc->name, status);
	task_exit(task);
}
