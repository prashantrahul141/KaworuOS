#include "core/cpu.h"
#include "boot/limine_responses.h"
#include "core/task.h"
#include "core/timer.h"
#include "debug/log.h"
#include "limine.h"
#include "mm/vmm.h"
#include "types.h"
#include "arch/aarch64/secondary_entry.h"
#include "arch/aarch64/aarch64.h"
#include <stdatomic.h>
#include "config.h"

volatile _Atomic u64 cpus_enabled_count = 1;

static Cpu CPUS[CONFIG_MAX_CPU_COUNT];

u32 cpu_get_cpuid(void)
{
	return r_mpidr_el1() & 0xFF;
}

Cpu *this_cpu(void)
{
	return (Cpu *)r_tpidr_el1();
}

void cpu_cache_current_cpu(void)
{
	u32 id = cpu_get_cpuid();
	Cpu *cpu = &CPUS[id];
	w_tpidr_el1((u64)cpu);
}

void init_secondary_cpu(void)
{
	/* save this cpu's Cpu struct in register */
	cpu_cache_current_cpu();

	/* set kernel paging for this cpu */
	vm_set_kernel_page_table();

	CpuTimer *this_cpu_timer = &this_cpu()->timer;
	timer_cpu_init(this_cpu_timer);
	timer_cpu_enable(this_cpu_timer);

	/* increment count of secondary cpus enabled */
	atomic_fetch_add(&cpus_enabled_count, 1);

	/* TODO: more per cpu init here before unmasking interrupts */

	w_intrd_enable();

	for (;;) {
	}
}

static void wake_secondary_cpu(struct limine_mp_info *cpu)
{
	DEBUG("waking cpu = %d", cpu->mpidr);
	atomic_store((_Atomic u64 *)&cpu->goto_address, (u64)secondary_entry);
}

void wake_secondary_cpus(void)
{
	INFO("Waking secondary cpus");
	struct limine_mp_response *cpus = limine_mp();
	DEBUG("total cpu count = %d", cpus->cpu_count);
	u64 boot_cpu_mpidr = cpus->bsp_mpidr;
	for (usize i = 0; i < cpus->cpu_count; i++) {
		struct limine_mp_info *cpu = cpus->cpus[i];
		if (boot_cpu_mpidr == cpu->mpidr) {
			continue;
		}

		wake_secondary_cpu(cpu);
	}

	/* wait for all cpus to be enabled */
	while (cpus->cpu_count != atomic_load(&cpus_enabled_count))
		;
}
