.global _start

.data

error_invalid_argc:
    .asciz "Usage: ./ttb [input_filename] [output_filename]\n"

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

sys_newfstatat: # rdi = filename, rsi  (char *filename at rdi, struct stat *stat at rsi)
    mov     rax, 0x106  # rax = SYSCALL_NEWFSTATAT(0x106)
    mov     rsi, rdi    # rsi = filename
    mov     rdi, -100   # rdi = AT_FDCWD
    lea     rdx, [rip + struct_stat_buffer] # rdx = &struct_stat_buffer
    xor     r10, r10    # r10 = flags(0)
    syscall
    ret

# Helper functions

get_file_size: # (char *filename at rdi)
    # rdi is already filename
    lea     rsi, [rip + struct_stat_buffer] # rsi := &struct_stat_buffer
    call    sys_newfstatat # rax := sys_newfstatat(argv[1], &struct_stat_buffer)
    mov     rdi, rax
    xor     rax, rax
    cmp     rdi, 0
    jne     L_get_file_size_return
    mov     rax, [rip + struct_stat_buffer + 0x30] # rdi := *(((void *)struct_stat_buffer) + 48)
L_get_file_size_return:
    ret

# -------------------

main: # (rdi = argc, rsi = argv)
    push    rbp
    mov     rbp, rsp
    sub     rsp, 64

    # [rbp -  8] = uint64 argc
    # [rbp - 16] = char **argv
    # [rbp - 24] = uint64 buffer_size
    # [rbp - 32] = void *buffer
    # [rbp - 40]
    # [rbp - 48]
    # [rbp - 56]
    # [rbp - 64]
    mov     [rbp - 8], rdi    # argc
    mov     [rbp - 16], rsi   # argv

    cmp     rdi, 3
    jl      L_main_failure_wrong_number_of_arguments

# ---- read file size
    mov     rdi, [rsi + 8]    # rdi = argv[1]
    call    get_file_size     # rax = get_file_size(rdi)
    mov     [rbp - 24], rax   # file_size = rax

    cmp     rax, 0
    je      L_main_return_failure

# ---- allocate buffer
    mov     rax, 0x9          # SYS_MMAP(0x9)
    xor     rdi, rdi          # addr
    mov     rsi, [rbp - 24]   # size
    mov     rdx, 3            # prot = PROT_READ (1) | PROT_WRITE (2)
    mov     r10, 0x22         # flags = MAP_PRIVATE (0x02) | MAP_ANONYMOUS (0x20)
    mov     r8, -1            # fd = -1 (required for anonymous)
    xor     r9, r9            # offset = 0
    syscall
    mov     [rbp - 32], rax   # buffer = rax

# ---- open file
    mov     rdi, [rbp - 16]   # argv
    mov     rdi, [rdi + 8]    # rdi = argv[1]
    xor     rsi, rsi          # flags = O_RDONLY(0)
    xor     rdx, rdx          # mode = 0 don't need for readonly
    mov     rax, 0x2          # SYSCALL_OPEN(0x2)
    syscall
# ---- read file
    mov     rdi, rax          # rdi = fd
    mov     rsi, [rbp - 32]   # rsi = buffer
    mov     rdx, [rbp - 24]   # rdx = size
    xor     rax, rax          # SYSCALL_READ(0x0)
    syscall
    mov     [rbp - 24], rax   # buffer_size = rax

    # Main algorithm:
    # rax = a
    # rbx = b
    # rcx = comment
    # rdi = r
    # rsi = w
    # r12 = buffer
    # r13 = size

    mov     rbx, 0 # r
    mov     rcx, 0 # w
    mov     rdx, 0 # is_comment
    mov     r8,  0 # a
    mov     r9,  0 # b
    mov     r12, [rbp - 32] # buffer
    mov     r13, [rbp - 24] # buffer_size

L_continue:
    cmp     rbx, r13                    # if (r >= buffer_size)
    jge     L_break                     #     break
    mov     r8b, byte ptr [r12 + rbx]   # a = buffer[r];
    add     rbx, 1                      # r = r + 1

    cmp     rdx, 0                      # if (is_comment == 0)
    je      L_is_not_comment            #     goto L_is_not_comment
    cmp     r8b, '\n'                   # if (a != '\n')
    jne     L_is_not_new_line           #     goto L_is_not_new_line:
    mov     rdx, 0                      # is_comment = false
L_is_not_new_line:
    jmp     L_continue                  # continue
L_is_not_comment:
    cmp     r8b, '#'                    # if (a != '#')
    jne     L_is_not_hash_sign          #     goto L_is_not_hash_sign
    mov     rdx, 1                      # is_comment = true
    jmp     L_continue                  # continue
L_is_not_hash_sign:

    mov     r8b, byte ptr [hex_table + r8] # a = hex_table[a]
    cmp     r8, 0xFF                       # if (a == 0xFF)
    je      L_continue                     #     continue

    cmp     rbx, r13                       # if (r >= buffer_size)
    jge     L_main_return_failure          #     return FAILURE
    mov     r9b, byte ptr [r12 + rbx]      # b = buffer[r];
    add     rbx, 1                         # r = r + 1

    mov     r9b, byte ptr [hex_table + r9] # b = hex_table[b]
    cmp     r9, 0xFF                       # if (b == 0xFF)
    je      L_main_return_failure          #     return FAILURE

    shl     r8, 4
    or      r8, r9

    mov     byte ptr [r12 + rcx], r8b
    add     rcx, 1
    jmp     L_continue
L_break:

# ---- open file
    mov     rdi, [rbp - 16]   # argv
    mov     rdi, [rdi + 16]   # rdi = argv[2]
    mov     rsi, 577          # O_WRONLY(1) | O_CREAT(64) | O_TRUNC(512)
    mov     rdx, 0744         # 0744 (rwxr--r--)
    mov     rax, 0x2          # SYSCALL_OPEN(0x2)
    syscall
# ---- write file
    mov     rdi, rax         # rdi = fd
    mov     rsi, r12         # rsi = buffer
    mov     rdx, rcx         # rdx = size
    mov     rax, 1           # SYSCALL_WRITE(0x1)
    syscall
# ----

    cmp     rax, 0
    je      L_main_return_failure

L_main_return_success:
    mov     rax, 0
    mov     rsp, rbp
    pop     rbp
    ret

L_main_failure_wrong_number_of_arguments:
    mov     rax, 0x1 # SYS_WRITE(0x1)
    mov     rdi, 1 # stdout
    lea     rsi, [rip + error_invalid_argc]
    mov     rdx, 48
    syscall

L_main_return_failure:
    mov     rax, 1
    mov     rsp, rbp
    pop     rbp
    ret

_start:
    mov     rdi, [rsp]
    lea     rsi, [rsp + 8]
    call    main
    mov     rdi, rax
    mov     rax, 0x3c   # SYSCALL_EXIT(0x3c)
    syscall
