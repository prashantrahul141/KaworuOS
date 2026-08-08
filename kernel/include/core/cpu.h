#ifndef _CPU_H_
#define _CPU_H_

#include "core/task.h"
#include "core/timer.h"
#include "sync/run_queue.h"
#include "types.h"
#include "config.h"

typedef struct {
	bool online;
	u32 cpuid;

	i32 irq_disable_depth;
	bool irq_was_enabled;

	CpuTimer timer;

	Task *current;
	Task *idle;

	RunQueue runnable_tasks;
} Cpu;

u32 cpu_get_cpuid(void);
Cpu *this_cpu(void);
void wake_secondary_cpus_and_wait(void);

/* saves current cpu struct mem address in TPIDR_EL1 */
void cpu_cache_current_cpu(void);

/* only called by arch secondary_entry */
void init_secondary_cpu(void);

Cpu *cpu_all_cpus(void);

#define cpu_foreach(cpu)                                          \
	for (usize __i = 0; __i < CONFIG_MAX_CPU_COUNT &&         \
			    ((cpu) = &cpu_all_cpus()[__i], true); \
	     __i++)

#endif // _CPU_H_
