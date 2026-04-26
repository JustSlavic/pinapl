.global _start

.data

hex_table:
    .rept 48
        .byte 0xFF
    .endr
    .byte 0,1,2,3,4,5,6,7,8,9
    .rept 7
        .byte 0xFF
    .endr
    .byte 10,11,12,13,14,15
    .rept 26
        .byte 0xFF
    .endr
    .byte 10,11,12,13,14,15
    .rept 153
        .byte 0xFF
    .endr

.bss

.align 8
struct_stat_buffer:
    .space 144

.text

# -------------------

_start:
    mov     r15b, 12
    mov     rdi, [rsp]
    lea     rsi, [rsp + 8]
    call    main
    mov     rdi, rax
    mov     rax, 0x3c   # SYSCALL_EXIT(0x3c)
    syscall

main: # (rdi = argc, rsi = argv)
    # r11 = r
    # r12 = buffer
    # r13 = size
    # r14 = w
    # r15 = argv
    mov     r15, rsi   # argv

    cmp     rdi, 3
    jl      L_main_return_failure

# ---- read file size
    mov     rdi, [r15 + 8]    # rdi = argv[1]

    mov     rax, 0x106        # rax = SYSCALL_NEWFSTATAT(0x106)
    mov     rdi, -100         # rdi = AT_FDCWD
    mov     rsi, [r15 + 8]    # rsi = argv[1]
    lea     rdx, [rip + struct_stat_buffer] # rdx = &struct_stat_buffer
    xor     r10, r10                        # r10 = flags(0)
    syscall
    mov     r13, [rip + struct_stat_buffer + 0x30] # size := *(((void *)struct_stat_buffer) + 48)

    cmp     r13, 0
    je      L_main_return_failure

# ---- allocate buffer
    mov     rax, 0x9          # SYS_MMAP(0x9)
    xor     rdi, rdi          # addr
    mov     rsi, r13          # size
    mov     rdx, 3            # prot = PROT_READ (1) | PROT_WRITE (2)
    mov     r10, 0x22         # flags = MAP_PRIVATE (0x02) | MAP_ANONYMOUS (0x20)
    mov     r8, -1            # fd = -1 (required for anonymous)
    xor     r9, r9            # offset = 0
    syscall
    mov     r12, rax          # buffer = rax

# ---- open file
    mov     rdi, [r15 + 8]    # rdi = argv[1]
    xor     rsi, rsi          # flags = O_RDONLY(0)
    xor     rdx, rdx          # mode = 0 don't need for readonly
    mov     rax, 0x2          # SYSCALL_OPEN(0x2)
    syscall

# ---- read file
    mov     rdi, rax          # rdi = fd
    mov     rsi, r12          # rsi = buffer
    mov     rdx, r13          # rdx = size
    xor     rax, rax          # SYSCALL_READ(0x0)
    syscall
    mov     r13, rax          # buffer_size = rax

# ---- main algorithm
    xor     rcx, rcx # is_comment
    xor     rax, rax # a
    xor     rbx, rbx # b
    xor     r11, r11 # r
    xor     r14, r14 # w

# ---- main loop
L_continue:
# ---- read a
    cmp     r11, r13                    # if (r >= buffer_size)
    jge     L_break                     #     break
    mov     al, byte ptr [r12 + r11]    # a = buffer[r];
    inc     r11                         # r = r + 1

# ---- comment logic
    mov     bl, cl              # was_comment = is_comment
    cmp     al, '\n'
    setne   dl                  # dl = (a != '\n')
    and     dl, cl              # dl = (was_comment && a != '\n')
    cmp     al, '#'
    sete    cl                  # cl = (a == '#')
    or      cl, dl              # cl = (a == '#') || (was_comment && a != '\n')
    test    bl, bl              # if (was_comment)
    jnz     L_continue          #     continue

# ---- get hex value of a
    mov     al, byte ptr [hex_table + rax] # a = hex_table[a]
    cmp     al, 0xFF                       # if (a == 0xFF)
    je      L_continue                     #     continue

# ---- read b
    cmp     r11, r13                       # if (r >= buffer_size)
    jge     L_main_return_failure          #     return FAILURE
    mov     bl, byte ptr [r12 + r11]       # b = buffer[r];
    inc     r11                            # r = r + 1

# ---- get hex value of b
    mov     bl, byte ptr [hex_table + rbx] # b = hex_table[b]
    cmp     bl, 0xFF                       # if (b == 0xFF)
    je      L_main_return_failure          #     return FAILURE

# ---- compute a = (a << 4) | b
    shl     rax, 4
    or      rax, rbx

# ---- write a into buffer
    mov     byte ptr [r12 + r14], al
    inc     r14
    jmp     L_continue
L_break:

# ---- open file
    mov     rdi, [r15 + 16]   # rdi = argv[2]
    mov     rsi, 577          # O_WRONLY(1) | O_CREAT(64) | O_TRUNC(512)
    mov     rdx, 0744         # 0744 (rwxr--r--)
    mov     rax, 0x2          # SYSCALL_OPEN(0x2)
    syscall
# ---- write file
    mov     rdi, rax         # rdi = fd
    mov     rsi, r12         # rsi = buffer
    mov     rdx, r14         # rdx = size
    mov     rax, 1           # SYSCALL_WRITE(0x1)
    syscall
# ----

    cmp     rax, 0
    je      L_main_return_failure

L_main_return_success:
    mov     rax, 0
    ret

L_main_return_failure:
    mov     rax, 1
    ret
