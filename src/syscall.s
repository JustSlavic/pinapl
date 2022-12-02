.text
.global exit
.global read
.global write
.global open
.global close
.global mmap2
.global munmap


exit:
    mov     r7, #0x01
    svc     #0

read:
    push    {r7}
    mov     r7, #0x03
    svc     #0
    pop     {r7}
    bx      lr

write:
    push    {r7}
    mov     r7, #0x04
    svc     #0
    pop     {r7}
    bx      lr

open:
    push    {r7}
    mov     r7, #0x05
    svc     #0
    pop     {r7}
    bx      lr

close:
    push    {r7}
    mov     r7, #0x06
    svc     #0
    pop     {r7}
    bx      lr

mmap2:
    push    {r7}
    mov     r7, #0xc0
    svc     #0
    pop     {r7}
    bx      lr

munmap:
    push    {r7}
    mov     r7, #0x5b
    svc     #0
    pop     {r7}
    bx      lr

