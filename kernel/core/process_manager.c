#include "core/process_manager.h"
#include "core/task.h"
#include "core/task_manager.h"
#include "debug/log.h"
#include "mm/address_space.h"
#include "core/process.h"
#include "mm/kheap.h"
#include "memlayout.h"
#include "mm/paging.h"
#include "sched/scheduler.h"
#include "sync/spinlock.h"

typedef struct {
	SpinLock lock;
	IntrusiveList process_list;
	usize pid_count;
} ProcManager;

constexpr usize USER_TASK_STACK_SIZE = (1 << CONFIG_PER_TASK_STACK_SIZE_SHIFT);

static ProcManager proc_manager = {};

void proc_manager_init(void)
{
	spinlock_init(&proc_manager.lock, "ProcManager");
	intrusivelist_init(&proc_manager.process_list);
	proc_manager.pid_count = 1;
}

Process *proc_manager_create(const i8 *name)
{
	DEBUG("creating new empty process: %s", name);
	spinlock_acquire_scoped(&proc_manager.lock);

	Process *proc = kalloc(sizeof(Process));
	AddressSpace *as = address_space_create();

	process_init(proc, proc_manager.pid_count++, name, as);

	intrusivelist_insert_tail(&proc_manager.process_list,
				  &proc->manager_node);
	return proc;
}

Process *proc_manager_create_exec(const i8 *name, usize program_pa,
				  usize program_size, usize entry)
{
	DEBUG("creating new process: %s at %p of size %p", name, program_pa,
	      program_size);
	spinlock_acquire_scoped(&proc_manager.lock);

	Process *proc = kalloc(sizeof(Process));
	if (IS_ERR(proc)) {
		WARN("failed creating process %s at %p", name, program_pa);
		return proc;
	}

	AddressSpace *as = address_space_create();

	process_init(proc, proc_manager.pid_count++, name, as);

	void *ustack = address_space_alloc(as, USER_TASK_STACK_SIZE,
					   EL1_READ_WRITE_EL0_READ_WRITE,
					   NOT_EXECUTABLE);
	usize ustack_top = ((usize)ustack) + USER_TASK_STACK_SIZE;

	address_space_map(as, USER_PROGRAM_START_VM, program_pa, program_size,
			  EL1_READ_ONLY_EL0_READ_ONLY, EXECUTABLE);

	Task *task = task_manager_create_user((struct Process *)proc, entry,
					      ustack_top, name);
	if (IS_ERR(task)) {
		address_space_destroy(as);
		kfree(proc);
		return ERR_TO_PTR(-ENOMEM);
	}

	intrusivelist_insert_tail(&proc_manager.process_list,
				  &proc->manager_node);

	process_add_thread(proc, task);
	scheduler_enqueue(task);
	return proc;
}
