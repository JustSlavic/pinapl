.text
.global _start

_start:
    ldr     r0, [sp]
    add     r1, sp, #4
    mov     r2, sp
get_env_loop:
    add     r2, #4
    ldr     r3, [r2]
    cmp     r3, #0
    bne     get_env_loop
    add     r2, #4
    bl      main

_exit:
    movs    r0, #0
    movs    r7, #1
    svc     #0

