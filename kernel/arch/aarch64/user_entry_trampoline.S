/*
 * user task entry trampoline
 */

/*
 * [sp] = user entry point (ELR_EL1)
 * [sp + 8] = user stack pointer (SP_EL0)
 */
.global user_entry_trampoline
user_entry_trampoline:
    ldp x0, x1, [sp]
    msr ELR_EL1, x0
    msr SP_EL0, x1
    mov x30, xzr
    msr SPSR_EL1, xzr
    eret
