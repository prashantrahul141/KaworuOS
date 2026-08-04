#ifndef _CPU_H_
#define _CPU_H_

#include "core/task.h"
#include "core/timer.h"
#include "core/run_queue.h"
#include "types.h"
#include "config.h"

typedef struct {
	u32 cpuid;
	i32 lock_count;
	bool intrd_was_enabled;
	CpuTimer timer;
	Task *current;
	Task *idle;
	RunQueue tasks;
} Cpu;

u32 cpu_get_cpuid(void);
Cpu *this_cpu(void);
void wake_secondary_cpus(void);

/* saves current cpu struct mem address in TPIDR_EL1 */
void cpu_cache_current_cpu(void);

/* only called by arch secondary_entry */
void init_secondary_cpu(void);

#endif // _CPU_H_
