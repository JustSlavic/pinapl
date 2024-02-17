.text
.global _start
.align 2

_start:
    bl main

_exit:
    mov x0, #42
    mov x16, #1
    svc #0
