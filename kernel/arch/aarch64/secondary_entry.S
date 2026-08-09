/*
 * called by all secondary cpus
 * x0 contains `limine_mp_info`
 */

#include "config.h"
#include "arch/aarch64/exception.S"

.extern init_secondary_cpu

.global secondary_entry
secondary_entry:
    /* disable all interrupts */
    /* mask (NOT D), A, I, F */
    msr daifclr, #0b1000
    msr daifset, #0b0111

    /* use current SP_ELx (1) for sp */
    mov x0, #1
    msr SPSel, x0
    isb

    /* setup this CPU's stack */
    mrs x0, MPIDR_EL1
    and x0, x0, #0xff
    add x0, x0, #1              // stack top of CPU n
    lsl x0, x0, #CONFIG_PER_CPU_BOOT_STACK_SIZE_SHIFT
    ldr x1, =cpu_stack
    add sp, x1, x0
    isb

    /* setup exception vector table */
    ldr x0, =vector_table_el1
    msr vbar_el1, x0
    isb

    /* System control register: enable fpu and simd */
    mrs x0, cpacr_el1
    orr x0, x0, #(0x3 << 20)
    msr cpacr_el1, x0
    isb

    /* call c entry point */
    bl init_secondary_cpu

    /* unreachable */
halt:
    wfe
    b halt
