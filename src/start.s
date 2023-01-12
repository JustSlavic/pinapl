.text
.global _start
.global __aeabi_idiv
.global __aeabi_idivmod

__aeabi_idiv:
    sdiv    r0, r0, r1
    bx      lr

__aeabi_idivmod:
    push    {r2, r3}
    sdiv    r2, r0, r1  @ r2 = r0 / r1  r2 now is div result
    muls    r3, r1, r2  @ r3 = r1 * r2
    subs    r1, r0, r3  @ r1 = r0 - r3  r1 now is mod result
    movs    r0, r2
    pop     {r2, r3}
    bx      lr

_start:
    ldr     r0, [sp]
    add     r1, sp, #4
    mov     r3, #4
    mla     r2, r0, r3, r1
    add     r2, #4 
    bl      main

_exit:
    movs    r0, #0
    movs    r7, #1
    svc     #0x90

