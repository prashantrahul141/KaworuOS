#include "core/task.h"
#include "aarch64/aarch64.h"
#include "core/cpu.h"
#include "sched/scheduler.h"
#include "debug/log.h"
#include "debug/panic.h"
#include "mm/kheap.h"
#include "string.h"

typedef void (*task_fn)(void *arg);

/* TODO: make this a kconfig option */
constexpr usize TASK_STACK_SIZE = (1 << 14);

void task_trampoline(void)
{
	Cpu *cpu = this_cpu();
	Task *task = cpu->current;
	task->entry(task->arg);
	DEBUG("task = %s exited", task->name);
	task_exit(task);
	panic("trampoline ended?");
}

Task *task_create(task_fn_type task_fn, void *arg, const i8 *name)
{
	Task *task = kalloc(sizeof(Task));
	task->name = name;
	task->stack = kalloc(TASK_STACK_SIZE);

	memset(&task->context, 0, sizeof(task->context));
	task->context.sp = (usize)task->stack + TASK_STACK_SIZE;
	task->context.lr = (u64)task_trampoline;

	task->state = TASK_READY;

	task->entry = task_fn;
	task->arg = arg;

	return task;
}

void task_exit(Task *task)
{
	task->state = TASK_DEAD;
	schedule();
}

void task_destroy(Task *task)
{
	TRACE("destroying task = %s", task->name);
	kfree(task->stack);
	kfree(task);
}

void task_idle(void *arg)
{
	UNUSED_ARG(arg);
	for (;;) {
		TRACE("idle task | cpuid = %d", this_cpu()->cpuid);
		wfi();
	}
}
