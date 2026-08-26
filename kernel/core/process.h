#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "ds/intrusivelist.h"
#include "mm/address_space.h"
#include "core/task.h"
#include "sync/wait_queue.h"
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

	IntrusiveNode parent_node;
	IntrusiveList children;
	WaitQueue child_waiters;

	bool exiting;
	i64 exit_code;
} Process;

void process_init(Process *proc, usize pid, const i8 *name, AddressSpace *as);

void process_exit(Process *proc, i64 exit_code);

void process_add_thread(Process *proc, Task *task);

void process_remove_thread(Process *proc, Task *thread);

usize process_thread_count(Process *proc);

/*
 * add child proc as as child of the given parent process
 */
void process_add_child(Process *parent, Process *child);

/*
 * remove child proc as as child of the given parent process
 */
void process_remove_child(Process *parent, Process *child);

/*
 * set parent of the given child process to the given parent process
 */
void process_set_parent(Process *parent, Process *child);

/*
 * get current process running
 */
Process *process_get_current(void);

#define process_foreach_child(parent, child, next_node)                    \
	for (IntrusiveNode *_process_child_node = (parent)->children.head; \
	     _process_child_node != nullptr &&                             \
	     (((next_node) = _process_child_node->next),                   \
	      ((child) = container_of(_process_child_node, Process,        \
				      parent_node)),                       \
	      true);                                                       \
	     _process_child_node = (next_node))

#endif // _PROCESS_H_
