#include "core/process.h"
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
