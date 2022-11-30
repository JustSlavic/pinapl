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
    mov     r7, #0x03
    svc     #0
    bx      lr

write:
    mov     r7, #0x04
    svc     #0
    bx      lr

open:
    mov     r7, #0x05
    svc     #0
    bx      lr

close:
    mov     r7, #0x06
    svc     #0
    bx      lr

mmap2:
    mov     r7, #0xc0
    svc     #0
    bx      lr

munmap:
    mov     r7, #0x5b
    svc     #0
    bx      lr

