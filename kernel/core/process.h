#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "mm/address_space.h"
#include "core/syscall.h"
#include "core/task.h"
#include "types.h"
#include "sync/spinlock.h"

typedef enum {
	/* atleast one thread actively running */
	PROCESS_RUNNING,

	/* can be cleaned up */
	PROCESS_ZOMBIE
} ProcessState;

typedef struct {
	const i8 *name;
	usize pid;

	SpinLock lock;
	IntrusiveNode manager_node;

	AddressSpace *address_space;

	ProcessState state;
	IntrusiveList threads;
	struct Process *parent;

	bool exiting;
	i64 exit_code;
} Process;

void process_init(Process *proc, usize pid, const i8 *name, AddressSpace *as);

/* TODO: implement */
void process_destroy(Process *proc);

void process_add_thread(Process *proc, Task *task);

void process_remove_thread(Process *proc, Task *thread);

usize process_thread_count(Process *proc);

NORETURN SYSCALL_DEFINE1_H(exit, i32, status)

#endif // _PROCESS_H_
