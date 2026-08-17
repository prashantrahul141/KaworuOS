#ifndef _INIT_H_
#define _INIT_H_

#include "aarch64/aarch64.h"
#include "boot/memmap.h"
#include "common_defs.h"
#include "io/console.h"
#include "core/cpu.h"
#include "config.h"
#include "sched/scheduler.h"
#include "core/process_manager.h"
#include "debug/log.h"
#include "limine.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/kheap.h"
#include "irq/irq_controller.h"
#include "debug/printf.h"
#include "common/manager.h"
#include "boot/limine_responses.h"
#include "boot/fdt.h"
#include "core/timer.h"
#include "framebuffer/framebuffer.h"
#include "debug/semihosting.h"
#include "core/task_manager.h"

/*
 * Kernel's entry point in C.
 */
void kernel_main(void);

#endif // _INIT_H_
