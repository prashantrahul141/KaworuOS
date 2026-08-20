.align 16
.data
    msg: .asciz "hello from user space via write syscall\n"


.align 16
.global user_init
user_init:
 /* write */
    mov x0, #1
    ldr x1, =msg
    mov x2, #40
    mov x8, #1
    svc #0
 /* exit */
    mov x0, #1
    mov x8, #60
    svc #0
