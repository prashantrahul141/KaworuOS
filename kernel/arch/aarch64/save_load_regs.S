/*
 * Exception register frame
 *
 *  offset |  register
 * -------------------------
 *    0    |  SP (SP_EL1 at exception entry)
 *    8    |  reserved
 *   16    |  x0   x1
 *   32    |  x2   x3
 *   48    |  x4   x5
 *   64    |  x6   x7
 *   80    |  x8   x9
 *   96    |  x10  x11
 *  112    |  x12  x13
 *  128    |  x14  x15
 *  144    |  x16  x17
 *  160    |  x19  x20
 *  176    |  x21  x22
 *  192    |  x23  x24
 *  208    |  x25  x26
 *  224    |  x27  x28
 *  240    |  x29  x30
 *  256    |  ELR_EL1  SPSR_EL1
 *  272    |  ESR_EL1  FAR_EL1
 *  288    |  SP_EL0
 *  296    |  reserved
 *
 * x18 is reserved as the exception entry scratch register and is not saved
 *
 * total size = 304 bytes
 */

.equ TF_SIZE, 304


/*
 * Save register state on stack.
 *
 * Returns:
 *      x0 = pointer to TrapFrame
 */
.macro save_registers
    mov x18, sp

    sub sp, sp, #TF_SIZE

    str x18, [sp, #0]

    stp x0, x1, [sp, #16]
    stp x2, x3, [sp, #32]
    stp x4, x5, [sp, #48]
    stp x6, x7, [sp, #64]
    stp x8, x9, [sp, #80]
    stp x10, x11, [sp, #96]
    stp x12, x13, [sp, #112]
    stp x14, x15, [sp, #128]
    stp x16, x17, [sp, #144]
    stp x19, x20, [sp, #160]
    stp x21, x22, [sp, #176]
    stp x23, x24, [sp, #192]
    stp x25, x26, [sp, #208]
    stp x27, x28, [sp, #224]
    stp x29, x30, [sp, #240]

    mrs x0, ELR_EL1
    mrs x1, SPSR_EL1
    stp x0, x1, [sp, #256]

    mrs x0, ESR_EL1
    mrs x1, FAR_EL1
    stp x0, x1, [sp, #272]

    mrs x0, SP_EL0
    str x0, [sp, #288]

    mov x0, sp
.endm

/*
 * Restore register state from stack.
 */
.macro restore_registers
    ldr x2, [sp, #288]
    msr SP_EL0, x2

    ldp x2, x3, [sp, #256]
    msr ELR_EL1, x2
    msr SPSR_EL1, x3

    ldp x2, x3, [sp, #272]

    ldp x29, x30, [sp, #240]
    ldp x27, x28, [sp, #224]
    ldp x25, x26, [sp, #208]
    ldp x23, x24, [sp, #192]
    ldp x21, x22, [sp, #176]
    ldp x19, x20, [sp, #160]
    ldp x16, x17, [sp, #144]
    ldp x14, x15, [sp, #128]
    ldp x12, x13, [sp, #112]
    ldp x10, x11, [sp, #96]
    ldp x8, x9, [sp, #80]
    ldp x6, x7, [sp, #64]
    ldp x4, x5, [sp, #48]
    ldp x2, x3, [sp, #32]
    ldp x0, x1, [sp, #16]

    add sp, sp, #TF_SIZE
.endm
