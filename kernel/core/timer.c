#include "core/timer.h"
#include "common/manager.h"
#include "common_defs.h"
#include "core/cpu.h"
#include "core/irq_controller.h"
#include "debug/panic.h"
#include "config.h"

static Timer timer = {
	nullptr,
	.frequency = 0,
};

void timer_init(void)
{
	INFO("Initializing timer");
	timer_global_init();
	timer_cpu_init(&this_cpu()->timer);
	timer_cpu_enable(&this_cpu()->timer);
}

void timer_global_init(void)
{
	timer.device = dmanager_get_by_class_and_ready(DEVICE_TIMER);
	if (IS_ERR(timer.device)) {
		panic("no timer device found");
	}

	timer.frequency = timer.device->timer_ops->frequency(timer.device);
	request_irq(timer.device->timer_ops->interrupt_id(timer.device),
		    timer_tick, nullptr);
}

void timer_cpu_init(CpuTimer *cpu_timer)
{
	cpu_timer->resched_after = ticks_from_miliseconds(
		timer.frequency, CONFIG_TIMER_RESCHED_MS);
}

static void timer_cpu_resched(const CpuTimer *cpu_timer)
{
	timer.device->timer_ops->fire_from_now(timer.device,
					       cpu_timer->resched_after);
}

void timer_tick(void *data)
{
	UNUSED_ARG(data);
	Cpu *cpu = this_cpu();
	// TODO: do something useful
	cpu->timer.ticks++;
	DEBUG("TICK! cpu id = %d ticks = %d", get_cpuid(), cpu->timer.ticks);
	timer_cpu_resched(&cpu->timer);
}

void timer_cpu_enable(CpuTimer *cpu_timer)
{
	UNUSED_ARG(cpu_timer);
	DEBUG("enabling timer");
	timer.device->timer_ops->fire_from_now(timer.device,
					       cpu_timer->resched_after);
	timer.device->timer_ops->enable(timer.device);
}
