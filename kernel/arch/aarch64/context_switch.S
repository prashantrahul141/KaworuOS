/*
 * Context frame
 *
 *  offset |  register
 * -------------------------
 *    0    |  x19
 *    8    |  x20
 *   16    |  x21
 *   24    |  x22
 *   32    |  x23
 *   40    |  x24
 *   48    |  x25
 *   56    |  x26
 *   64    |  x27
 *   72    |  x28
 *   80    |  fp | x29
 *   88    |  lr | x30
 *   96    |  sp

 * total size = 104 bytes
 */

.global context_switch

/*
 * x0 contains old task
 * x1 contains new task
 */
context_switch:
    /* saving old context */
    stp x19, x20, [x0], #16
    stp x21, x22, [x0], #16
    stp x23, x24, [x0], #16
    stp x25, x26, [x0], #16
    stp x27, x28, [x0], #16
    stp x29, x30, [x0], #16
    mov x2, sp
    str x2, [x0]

    /* restoring new context */
    ldp x19, x20, [x1], #16
    ldp x21, x22, [x1], #16
    ldp x23, x24, [x1], #16
    ldp x25, x26, [x1], #16
    ldp x27, x28, [x1], #16
    ldp x29, x30, [x1], #16
    ldr x2, [x1]
    mov sp, x2

    ret
