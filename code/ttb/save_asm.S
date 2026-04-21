sys_read: # rdi = fd, rsi = buffer, rdx = size
    xor     rax, rax    # SYSCALL_READ(0x0)
    syscall
    ret

sys_write: # rdi = fd, rsi = buffer, rdx = size
    mov     rax, 1      # SYSCALL_WRITE(0x1)
    syscall
    ret

sys_exit: # rdi = ec
    mov     rax, 0x3c   # rax = SYSCALL_EXIT(0x3c)
    syscall

write_newline:
    sub     rsp, 16
    mov     byte ptr [rsp], 10
    mov     rdi, 1      # rdi := STDOUT(0x1)
    mov     rsi, rsp    # rsi := rdi
    mov     rdx, 1      # rdx := rax
    call    sys_write
    add     rsp, 16
    ret

write_cstring: # (pointer cstring rdi)
    call    cstring_size_no_zero # rax := cstring_size_no_zero(rdi)
    mov     rdx, rax    # rdx := rax
    mov     rsi, rdi    # rsi := rdi
    mov     rdi, 1      # rdi := STDOUT(0x1)
    call    sys_write
    ret

cstring_size_with_zero: # pointer rdi -> uint64 rax
    xor     rax, rax         # rax := 0
    xor     rcx, rcx         # rcx := 0
L_loop0:                     # L_loop0:
    mov     cl, [rdi + rax]  #     cl := *(rdi + rax)
    add     rax, 1           #     rax := rax + 1
    cmp     rcx, 0           #     if (rsi != 0)
    jne     L_loop0          #         goto L_loop0
    ret

cstring_size_no_zero: # pointer rdi -> uint64 rax
    call    cstring_size_with_zero
    sub     rax, 1
    ret

uint64_print: # (uint64 rdi)
    # Prologue
    push    rbp
    mov     rbp, rsp
    # Function
    sub     rsp, 48     # char buffer[48]
    mov     rax, rdi    # rax := rdi
    mov     rcx, 10

    # Determine the length required to print a number
    xor     rsi, rsi    # rsi := 0
    mov     byte ptr [rsp], 0 # buffer[0] := 0
loop1:
    add     rsi, 1      # rsi += 1
    mov     byte ptr [rsp + rsi], 0 # buffer[rsi] := 0

    xor     rdx, rdx    # Prepare rdx for div
    div     rcx
    cmp     rax, 0      # rax now holds quotient
    jne     loop1

    # Now the number of digits in the input number is in the rsi
    mov     rax, rdi

loop2:
    sub     rsi, 1

    xor     rdx, rdx
    div     rcx

    add     rdx, 48
    mov     byte ptr [rsp + rsi], dl

    cmp     rax, 0
    jne     loop2

    mov     rdi, rsp
    call    write_cstring

    # Epilogue
    mov     rsp, rbp
    pop     rbp
    ret
