.text
.global _start

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
    svc     #0

