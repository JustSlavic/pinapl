.text
.global exit
.global read
.global write
.global open
.global close


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
