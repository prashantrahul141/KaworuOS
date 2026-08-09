#ifndef _MEMLAYOUT_H_
#define _MEMLAYOUT_H_

#include "config.h"
#include "types.h"
#include "common_defs.h"

/*
 * 0x0F0XXXXX   free mem top
 * ....
 * ....
 * ....       + MEMORY SIZE - kernel size
 * ....
 * ....
 * 0x040XXXXX   kernel_top = free mem bottom (kernel ends here)
 * ....
 * ....       + .bss
 * ....       + .data
 * ....       + .rodata -> __KERNEL_TEXT_END
 * ....       + .text
 * ....
 * 0x40100000   kernel_bottom (kernel starts here)
 *
 * 0x00000000   absolute zero
 */

extern symbol __kernel_end;
extern symbol __kernel_start;
extern symbol __kernel_text_start;
extern symbol __kernel_text_end;
extern symbol __kernel_data_start;
extern symbol __kernel_data_end;
extern symbol __kernel_text_drivers_start;
extern symbol __kernel_text_drivers_end;

#define __KERNEL_START		    ((usize)__kernel_start)
#define __KERNEL_END		    ((usize)__kernel_end)
#define __KERNEL_TEXT_START	    ((usize)__kernel_text_start)
#define __KERNEL_TEXT_END	    ((usize)__kernel_text_end)
#define __KERNEL_DATA_START	    ((usize)__kernel_data_start)
#define __KERNEL_DATA_END	    ((usize)__kernel_data_end)
#define __KERNEL_TEXT_DRIVERS_START ((usize)__kernel_text_drivers_start)
#define __KERNEL_TEXT_DRIVERS_END   ((usize)__kernel_text_drivers_end)

#define MB_TO_BYTES(MB) ((MB) * (usize)1000 * (usize)1000)

constexpr usize PAGE_SIZE = 4096;

/* is value page aligned */
#define IS_PAGE_ALIGNED(value) IS_ALIGNED((value), PAGE_SIZE)

/* should match with the one in linker file */
constexpr usize KERNEL_BASE = 0xffffffff80000000;

constexpr usize KERNEL_VM_RANGE_BASE = 0xffffffffa0000000;
constexpr usize KERNEL_VM_RANGE_SIZE = (1UL << 30); // ~1GB

constexpr usize KERNEL_MMIO_RANGE_START = 0xffffffff90000000UL;
constexpr usize KERNEL_MMIO_RANGE_SIZE = (256UL << 20); // ~256MB

#endif // _MEMLAYOUT_H_
