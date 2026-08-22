#include "core/process_manager.h"
#include "core/task.h"
#include "core/task_manager.h"
#include "debug/assert.h"
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
	if (IS_ERR(proc)) {
		WARN("failed creating proc: %s", name);
		return proc;
	}

	AddressSpace *as = address_space_create();
	if (IS_ERR(as)) {
		kfree(proc);
		WARN("failed to create address space from proc: %s", name);
		return ERR_TO_PTR(-ENOMEM);
	}

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
	if (IS_ERR(as)) {
		kfree(proc);
		WARN("failed to create address space from proc: %s", name);
		return ERR_TO_PTR(-ENOMEM);
	}

	process_init(proc, proc_manager.pid_count++, name, as);

	void *ustack = address_space_alloc(as, USER_TASK_STACK_SIZE,
					   EL1_READ_WRITE_EL0_READ_WRITE,
					   NOT_EXECUTABLE);
	if (IS_ERR(ustack)) {
		address_space_destroy(as);
		kfree(proc);
		WARN("failed to allocate mem in this address space for proc: "
		     "%s",
		     name);
		return ERR_TO_PTR(-ENOMEM);
	}
	usize ustack_top = ((usize)ustack) + USER_TASK_STACK_SIZE;

	errno_t err = address_space_map(as, USER_PROGRAM_START_VM, program_pa,
					program_size,
					EL1_READ_ONLY_EL0_READ_ONLY,
					EXECUTABLE);
	if (EOK != err) {
		address_space_destroy(as);
		kfree(proc);
		WARN("failed to allocate mem in this address space for proc: "
		     "%s",
		     name);
		return ERR_TO_PTR(-ENOMEM);
	}

	Task *task = task_manager_create_user((struct Process *)proc, entry,
					      ustack_top, name);
	if (IS_ERR(task)) {
		address_space_destroy(as);
		kfree(proc);
		WARN("failed to create user task for user: %s", name);
		return ERR_TO_PTR(-ENOMEM);
	}

	intrusivelist_insert_tail(&proc_manager.process_list,
				  &proc->manager_node);

	process_add_thread(proc, task);
	scheduler_enqueue(task);
	return proc;
}

/*
 * destroys process
 */
void proc_manager_remove_destroy(Process *proc)
{
	spinlock_acquire_scoped(&proc_manager.lock);
	ASSERT(proc->state == PROCESS_ZOMBIE, "removing & destroying a non "
					      "zombie process");
	DEBUG("destroying process: %s", proc->name);

	usize thread_count = intrusivelist_count(&proc->threads);
	ASSERT(thread_count > 0, "destroying task which has %d (> 0) threads",
	       thread_count);

	intrusivelist_remove(&proc_manager.process_list, &proc->manager_node);

	if (proc->address_space != nullptr) {
		address_space_destroy(proc->address_space);
	}

	kfree(proc);
}
