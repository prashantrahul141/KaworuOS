#include "core/task_manager.h"
#include "ds/intrusivelist.h"
#include "sched/scheduler.h"
#include "debug/panic.h"
#include <stdatomic.h>
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
#include "config.h"

volatile _Atomic u64 cpus_enabled_count = 1;

static Cpu CPUS[CONFIG_MAX_CPU_COUNT] = { 0 };

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

/* these are to be done for boot and all secondary cpus */
static void common_cpu_init_tasks(void)
{
	Cpu *cpu = this_cpu();
	cpu->cpuid = cpu_get_cpuid();

	intrusivelist_init(&cpu->runnable_tasks);

	/* create idle task */
	cpu->idle = task_manager_create_new(task_idle, nullptr, "idle");
	cpu->current = nullptr;
}

void init_secondary_cpu(void)
{
	/* save this cpu's Cpu struct in register */
	cpu_cache_current_cpu();

	/* set kernel paging for this cpu */
	vm_set_kernel_page_table();

	Cpu *cpu = this_cpu();
	cpu->cpuid = cpu_get_cpuid();

	CpuTimer *this_cpu_timer = &cpu->timer;
	timer_cpu_init(this_cpu_timer);
	timer_cpu_enable(this_cpu_timer);

	common_cpu_init_tasks();

	/* enable interrupts before switching */
	w_intrd_enable();

	/* increment count of secondary cpus enabled */
	atomic_fetch_add(&cpus_enabled_count, 1);

	/* schedule */
	schedule();

	panic("scheduler for secondary cpu = %d returned", cpu->cpuid);
}

static void wake_secondary_cpu(struct limine_mp_info *cpu)
{
	DEBUG("waking cpu = %d", cpu->mpidr);
	atomic_store((_Atomic u64 *)&cpu->goto_address, (u64)secondary_entry);
}

void wake_secondary_cpus_and_wait(void)
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

	common_cpu_init_tasks();

	/* wait for all cpus to be enabled */
	while (cpus->cpu_count != atomic_load(&cpus_enabled_count))
		;

	INFO("All cpus running");
}
