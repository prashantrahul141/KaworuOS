#include "save_load_regs.S"

.extern exception_handler
.extern irq_handle
.extern unhandled_exception_handler
.extern scheduler_switch

.global preempt
preempt:
    /* mask irqs: eret brings SPSR (I=0) back */
    msr daifset, #0b0011

    /* switch */
    bl scheduler_switch

    /* restore frame */
    restore_registers

    eret

.macro vector_entry target
    b \target
.endm

/* Exception vector table */
.global vector_table_el1
.section .text
.balign 0x800
vector_table_el1:
/*
 * The exception handler for a synchronous
 * exception from the current EL using SP0.
 */
curr_el_sp0_sync:
    vector_entry curr_el_sp0_sync_handler

/*
 * The exception handler for an IRQ exception
 * from the current EL using SP0.
 */
.balign 0x80
curr_el_sp0_irq:
    vector_entry curr_el_sp0_irq_handler

/*
 *
 * The exception handler for an FIQ exception
 * from the current EL using SP0.
*/
.balign 0x80
curr_el_sp0_fiq:
    vector_entry curr_el_sp0_fiq_handler

/*
 * The exception handler for a System Error
 * exception from the current EL using SP0.
 */
.balign 0x80
curr_el_sp0_serror:
    vector_entry curr_el_sp0_serror_handler

/*
 * The exception handler for a synchrous
 * exception from the current EL using the current SP
 */

.balign 0x80
curr_el_spx_sync:
    vector_entry curr_el_spx_sync_handler

/*
 * The exception handler for an IRQ exception from
 * the current EL using the current SP.
 */
.balign 0x80
curr_el_spx_irq:
    vector_entry curr_el_spx_irq_handler

/*
 * The exception handler for an FIQ from
 * the current EL using the current SP.
 */
.balign 0x80
curr_el_spx_fiq:
    vector_entry curr_el_spx_fiq_handler

/*
 * The exception handler for a System Error
 * exception from the current EL using the current SP.
 */
.balign 0x80
curr_el_spx_serror:
    vector_entry curr_el_spx_serror_handler

/*
 * The exception handler for a synchronous
 * exception from a lower EL (AArch64).
 */
 .balign 0x80
lower_el_aarch64_sync:
    vector_entry lower_el_aarch64_sync_handler

/*
 * The exception handler for an IRQ from a lower EL
 * (AArch64).
 */
.balign 0x80
lower_el_aarch64_irq:
    vector_entry lower_el_aarch64_irq_handler

/*
 * The exception handler for an FIQ from a lower EL
 * (AArch64).
 */
.balign 0x80
lower_el_aarch64_fiq:
    vector_entry lower_el_aarch64_fiq_handler

/*
 * The exception handler for a System Error
 * exception from a lower EL(AArch64).
 */
.balign 0x80
lower_el_aarch64_serror:
    vector_entry lower_el_aarch64_serror_handler

/*
 * The exception handler for a synchronous
 * exception from a lower EL(AArch32).
 */
.balign 0x80
lower_el_aarch32_sync:
    vector_entry lower_el_aarch32_sync_handler

/*
 * The exception handler for an IRQ exception
 * from a lower EL (AArch32).
 */
.balign 0x80
lower_el_aarch32_irq:
    vector_entry lower_el_aarch32_irq_handler

/*
 * The exception handler for an FIQ exception from
 * a lower EL (AArch32).
 */
.balign 0x80
lower_el_aarch32_fiq:
    vector_entry lower_el_aarch32_fiq_handler

/*
 * The exception handler for a System Error
 * exception from a lower EL(AArch32).
 */
.balign 0x80
lower_el_aarch32_serror:
    vector_entry lower_el_aarch32_serror_handler


/* actual handlers here ------------- */
curr_el_sp0_sync_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret


curr_el_sp0_irq_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

curr_el_sp0_fiq_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

curr_el_sp0_serror_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

curr_el_spx_sync_handler:
    save_registers
    bl exception_handler
    restore_registers
    eret


curr_el_spx_irq_handler:
    save_registers
    bl irq_handle
    cbnz x0, preempt
    restore_registers
    eret

curr_el_spx_fiq_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret


curr_el_spx_serror_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

lower_el_aarch64_sync_handler:
    save_registers
    bl exception_handler
    cbnz x0, preempt
    restore_registers
    eret

lower_el_aarch64_irq_handler:
    save_registers
    bl irq_handle
    cbnz x0, preempt
    restore_registers
    eret

lower_el_aarch64_fiq_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

lower_el_aarch64_serror_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

lower_el_aarch32_sync_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

lower_el_aarch32_irq_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

lower_el_aarch32_fiq_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret

lower_el_aarch32_serror_handler:
    save_registers
    bl unhandled_exception_handler
    restore_registers
    eret
