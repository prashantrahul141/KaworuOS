#include "aarch64/aarch64.h"
#include "boot/fdt.h"
#include "common/manager.h"
#include "common_defs.h"
#include "types.h"

static void enable_timer(Device *device)
{
	UNUSED_ARG(device);
	w_counter_enable(true);
}

static void disable_timer(Device *device)
{
	UNUSED_ARG(device);
	w_counter_enable(false);
}

static usize frequency(const Device *device)
{
	UNUSED_ARG(device);
	return (usize)r_counter_timer_freq();
}

static usize counter(const Device *device)
{
	UNUSED_ARG(device);
	return r_cntpct_el0();
}

static void fire_from_now(Device *device, usize ticks)
{
	UNUSED_ARG(device);
	w_cntp_tval_el0(ticks);
}

static void fire_at(Device *device, usize ticks)
{
	UNUSED_ARG(device);
	w_cntp_cval_el0(ticks);
}

static u32 interrupt_id(Device *device)
{
	FDTInterrupt interrupt_cells[4] = { 0 };
	if (!fdt_get_interrupt_cells(device->fdt_node_offset, interrupt_cells,
				     4)) {
		return 0;
	}

	// TODO: instead of hard coding indexes here, find them via their type.
	return interrupt_cells[1].cells[1] + 16;
}

static const TimerOps timer_ops = { .enable = enable_timer,
				    .disable = disable_timer,
				    .frequency = frequency,
				    .counter = counter,
				    .fire_from_now = fire_from_now,
				    .fire_at = fire_at,
				    .interrupt_id = interrupt_id };

static errno_t armv8_timer_probe(Device *device)
{
	device->name = "armv8-timer";
	device->timer_ops = &timer_ops;
	return EOK;
}

static errno_t armv8_timer_remove(Device *device)
{
	disable_timer(device);
	return EOK;
}

static const i8 *armv8_timer_compat[] = {
	"arm,armv8-timer",
	"arm,armv7-timer",
	nullptr,
};

static const Driver armv8_timer_driver = { .name = "armv8-timer",
					   .probe = armv8_timer_probe,
					   .remove = armv8_timer_remove,
					   .compatible = armv8_timer_compat,
					   .device_class = DEVICE_TIMER };

REGISTER_DEVICE_DRIVER(armv8_timer_driver);
