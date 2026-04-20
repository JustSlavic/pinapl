#define SYSCALL_WRITE          0x1
#define SYSCALL_EXIT           0x3c
#define SYSCALL_NEWFSTATAT     0x106

#define AT_FDCWD -100

.global _start

.data

message:
    .asciz "Hello, World!\n"
input_filename:
    .asciz "input.txt"
output_filename:
    .asciz "output"

.bss

.align 8
struct_stat_buffer:
    .space 144

.text
sys_read: # rdi = fd, rsi = buffer, rdx = size
    xor     rax, rax    # SYSCALL_READ(0x0)
    # rdi file_descriptor should be set already
    # rsi buffer pointer should be set already
    # rdx buffer size should be set already
    syscall
    ret

sys_write: # rdi = fd, rsi = buffer, rdx = size
    mov     rax, 1      # SYSCALL_WRITE(0x1)
    # rdi file_descriptor should be set already
    # rsi buffer pointer should be set already
    # rdx buffer size should be set already
    syscall
    ret

sys_open: # rdi = filename
    mov     rax, 0x2    # SYSCALL_OPEN(0x2)
    # rdi filename should be set already
    # rsi flags should be set already
    # rdx mode should be set already
    syscall
    ret

sys_close: # rdi = fd
    mov     rax, 0x3    # SYSCALL_CLOSE(0x3)
    # rdi file_descriptor should be set already
    syscall
    ret

sys_allocate: # rdi = size
    # void *mmap(NULL, 4096, PROT_READ|PROT_WRITE,
    #            MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    mov rsi, rdi            # requested_size

    mov rax, 0x9            # syscall: mmap
    xor rdi, rdi            # addr = NULL
    mov rdx, 3              # prot = PROT_READ (1) | PROT_WRITE (2)
    mov r10, 0x22           # flags = MAP_PRIVATE (0x02) | MAP_ANONYMOUS (0x20)
    mov r8, -1              # fd = -1 (required for anonymous)
    xor r9, r9              # offset = 0
    syscall
    ret

sys_exit: # rdi = ec
    mov     rax, 0x3c   # rax = SYSCALL_EXIT(0x3c)
    syscall

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

read_file_into_memory: # rdi = filename, rsi = buffer, rdx = size
    # Prologue
    push    rbp
    mov     rbp, rsp

    sub     rsp, 32
    mov     [rbp - 8], rsi   # char *buffer;
    mov     [rbp - 16], rdx  # uint64 size;

    # rdi is filename
    xor     rsi, rsi         # O_RDONLY(0)
    xor     rdx, rdx         # don't need for readonly
    call    sys_open         # rax = sys_open(filename, O_RDONLY, 0)
    mov     [rbp - 24], rax  # fd = rax

    mov     rdi, rax         # rdi = fd
    mov     rsi, [rbp - 8]   # rsi = buffer
    mov     rdx, [rbp - 16]  # rdx = size
    call    sys_read
    mov     [rbp - 32], rax  # bytes_read

    mov     rdi, [rbp - 24]
    call    sys_close

    mov     rax, [rbp - 32]

    # Epilogue
    mov     rsp, rbp
    pop     rbp
    ret

write_memory_into_file: # rdi = filename, rsi = buffer, rdx = size
    # Prologue
    push    rbp
    mov     rbp, rsp

    sub     rsp, 32
    mov     [rbp - 8], rsi   # char *buffer;
    mov     [rbp - 16], rdx  # uint64 size;

    # rdi is filename
    mov     rsi, 577         # O_WRONLY(1) | O_CREAT(64) | O_TRUNC(512)
    mov     rdx, 0744        # 0744 (rwxr--r--)
    call    sys_open         # rax = sys_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0744)
    mov     [rbp - 24], rax  # fd = rax

    mov     rdi, rax         # rdi = fd
    mov     rsi, [rbp - 8]   # rsi = buffer
    mov     rdx, [rbp - 16]  # rdx = size
    call    sys_write
    mov     [rbp - 32], rax  # bytes_written

    mov     rdi, [rbp - 24]
    call    sys_close

    mov     rax, [rbp - 32]

    # Epilogue
    mov     rsp, rbp
    pop     rbp
    ret

# -------------------

debug_write_character:
    sub     rsp, 8
    mov     [rsp], dil

    mov     rdi, 1
    mov     rsi, rsp
    mov     rdx, 1
    call    sys_write

    add     rsp, 8
    ret

# 1 - uppercase letter
# 2 - lowercase letter
# 3 - digit
# 0 - otherwise
is_ascii_hex: # (rdi = character)
    # 'A' <= c <= 'Z'
    cmp     rdi, 'A'
    jl      L_check_ascii_lowercase
    cmp     rdi, 'F'
    jg      L_check_ascii_lowercase
    mov     rax, 1
    ret

L_check_ascii_lowercase:
    # 'a' <= c <= 'z'
    cmp     rdi, 'a'
    jl      L_check_ascii_digit
    cmp     rdi, 'f'
    jg      L_check_ascii_digit
    mov     rax, 2
    ret

L_check_ascii_digit:
    # '0' <= c <= '9'
    cmp     rdi, '0'
    jl      L_check_ascii_failure
    cmp     rdi, '9'
    jg      L_check_ascii_failure
    mov     rax, 3
    ret

L_check_ascii_failure:
    mov     rax, 0
    ret

main: # (rdi = argc, rsi = argv)
    push    rbp
    mov     rbp, rsp
    sub     rsp, 64

    # [rbp -  8] = uint64 argc
    # [rbp - 16] = char **argv
    # [rbp - 24] = uint64 file_size
    # [rbp - 32] = void *input_buffer
    # [rbp - 40] = void *output_buffer
    # [rbp - 48] = uint64 input_size
    # [rbp - 56]
    # [rbp - 64]

    mov     [rbp - 8], rdi    # argc
    mov     [rbp - 16], rsi   # argv

    cmp     rdi, 2
    jl      L_main_return_failure

    mov     rdi, [rsi + 8]    # rdi = argv[1]
    call    get_file_size     # rax = get_file_size(rdi)
    mov     [rbp - 24], rax   # file_size = rax

    cmp     rax, 0
    je      L_main_return_failure

    mov     rdi, [rbp - 24]   # rdi = file_size
    call    sys_allocate      # rax = sys_allocate(rdi)
    mov     [rbp - 32], rax   # input_buffer = rax

    mov     rdi, [rbp - 24]   # rdi = file_size
    call    sys_allocate      # rax = sys_allocate(rdi)
    mov     [rbp - 40], rax   # output_buffer = rax

    mov     rdi, [rbp - 16]   # argv
    mov     rdi, [rdi + 8]    # rdi = argv[1]
    mov     rsi, [rbp - 32]   # rsi = input_buffer
    mov     rdx, [rbp - 24]   # rdx = file_size
    call    read_file_into_memory # rax = read_file_into_memory(argv[1], input_buffer, file_size)
    mov     [rbp - 48], rax   # input_size = rax

    # Main algorithm:
    mov     rax, 0 # ret
    mov     rdi, 0 # arg
    mov     rbx, 0 # r
    mov     rcx, 0 # w
    mov     rdx, 0 # is_comment
    mov     r8,  0 # a
    mov     r9,  0 # b
    mov     r10, 0 # c
    mov     r11, [rbp - 32] # input_buffer
    mov     r12, [rbp - 40] # output_buffer
    mov     r13, [rbp - 48] # input_size

L_continue:
    cmp     rbx, r13                    # if (r >= input_size)
    jge     L_break                     #     break
    mov     r8b, byte ptr [r11 + rbx]   # a = input_buffer[r];
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

    mov     rdi, r8                     # arg = a
    call    is_ascii_hex                # rax = is_ascii_hex(arg)
    cmp     rax, 0                      # if (rax == 0)
    je      L_continue                  #     continue
    cmp     rax, 1                      # if (rax == 1)
    je      L_a_is_uppercase            #     goto L_a_is_uppercase
    cmp     rax, 2                      # if (rax == 2)
    je      L_a_is_lowercase            #     goto L_a_is_lowercase
    cmp     rax, 3                      # if (rax == 3)
    je      L_a_is_digit                #     goto L_a_is_digit
L_a_is_uppercase:
    sub     r8, 55                      # a = (a - 'A' + 10)
    jmp     L_a_end_check
L_a_is_lowercase:
    sub     r8, 87                      # a = (a - 'a' + 10)
    jmp     L_a_end_check
L_a_is_digit:
    sub     r8, 48                      # a = (a - '0')
    jmp     L_a_end_check
L_a_end_check:

    cmp     rbx, r13                    # if (r >= input_size)
    jge     L_main_return_failure       #     return FAILURE
    mov     r9b, byte ptr [r11 + rbx]   # b = input_buffer[r];
    add     rbx, 1                      # r = r + 1

    mov     rdi, r9                     # arg = b
    call    is_ascii_hex                # rax = is_ascii_hex(arg)
    cmp     rax, 0                      # if (rax == 0)
    je      L_main_return_failure       #     return FAILURE
    cmp     rax, 1
    je      L_b_is_uppercase
    cmp     rax, 2
    je      L_b_is_lowercase
    cmp     rax, 3
    je      L_b_is_digit
L_b_is_uppercase:
    sub     r9, 55
    jmp     L_b_end_check
L_b_is_lowercase:
    sub     r9, 87
    jmp     L_b_end_check
L_b_is_digit:
    sub     r9, 48
    jmp     L_b_end_check
L_b_end_check:

    shl     r8, 4
    or      r8, r9

    mov     byte ptr [r12 + rcx], r8b
    add     rcx, 1
    jmp     L_continue
L_break:

    lea     rdi, [rip + output_filename]
    mov     rsi, r12
    mov     rdx, rcx
    call    write_memory_into_file

    cmp     rax, 0
    je      L_main_return_failure

L_main_return_success:
    mov     rax, 0
    mov     rsp, rbp
    pop     rbp
    ret
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


/*

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
*/
