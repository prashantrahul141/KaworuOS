#ifndef _CPU_H_
#define _CPU_H_

#include "core/timer.h"
#include "types.h"
#include "config.h"

typedef struct {
	u32 cpuid;
	i32 count;
	bool intrd_was_enabled;
	CpuTimer timer;
} Cpu;

u32 get_cpuid(void);
Cpu *this_cpu(void);
void wake_secondary_cpus(void);

/* only called by arch secondary_entry */
void init_secondary_cpu(void);

#endif // _CPU_H_
