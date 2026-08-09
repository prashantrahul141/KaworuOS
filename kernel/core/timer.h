#ifndef _TIMER_H_
#define _TIMER_H_

#include "common/manager.h"
#include "types.h"

/*
 * Global timer
 */
typedef struct {
	Device *device;
	usize frequency;
} Timer;

/*
 * per cpu timer
 */
typedef struct {
	usize ticks;
	usize resched_after;
} CpuTimer;

/*
 * Initialize timer for boot cpu
 */
void timer_init(void);

/*
 * Initialize global timer
 */
void timer_global_init(void);

/*
 * Initialize per cpu timer
 */
void timer_cpu_init(CpuTimer *cpu_timer);

/*
 * Enable per cpu timer and start receiving interrupts
 */
void timer_cpu_enable(CpuTimer *cpu_timer);

/*
 * This is called when timer interrupt arrives
 */
void timer_tick(void *data);

static inline usize ticks_from_seconds(usize frequency, usize seconds)
{
	return frequency * seconds;
}

static inline usize ticks_from_miliseconds(usize frequency, usize ms)
{
	usize one_milisecond = frequency / 1000;
	return one_milisecond * ms;
}

#endif // _TIMER_H_
