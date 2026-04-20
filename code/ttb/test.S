.global hello
.global exit

.data

message:
.ascii "Hello, World!\n"

.text

#                       x86_64 Registers
#
#   +-----+------+------+--------+---------------+---------------+
#   | rax | eax  |  ax  | ah, al | Return Value  |   Volatile    |
#   | rbx | ebx  |  bx  | bh, bl |               | Non-Volatile  |
#   | rcx | ecx  |  cx  | ch, cl | 4th Argument  |   Volatile    |
#   | rdx | edx  |  dx  | dh, dl | 3rd Argument  |   Volatile    |
#   +-----+------+------+--------+---------------+---------------+
#   | rsi | esi  |  si  | sil    | 2nd Argument  |   Volatile    |
#   | rdi | edi  |  di  | dil    | 1st Argument  |   Volatile    |
#   +-----+------+------+--------+---------------+---------------+
#   | rbp | ebp  |  bp  | bpl    | Frame Pointer | Do not change |
#   | rsp | esp  |  sp  | spl    | Stack Pointer | Do not change |
#   +-----+------+------+--------+---------------+---------------+
#   | r8  | r8d  | r8w  | r8b    | 5th Argument  |   Volatile    |
#   | r9  | r9d  | r9w  | r9b    | 6th Argument  |   Volatile    |
#   | r10 | r10d | r10w | r10b   |               |   Volatile    |
#   | r11 | r11d | r11w | r11b   |               |   Volatile    |
#   | r12 | r12d | r12w | r12b   |               | Non-Volatile  |
#   | r13 | r13d | r13w | r13b   |               | Non-Volatile  |
#   | r14 | r14d | r14w | r14b   |               | Non-Volatile  |
#   | r15 | r15d | r15w | r15b   |               | Non-Volatile  |
#   +-----+------+------+--------+---------------+---------------+
#

exit:
    mov     rax, 0x3c
    mov     edi, [rsp] # argc
    syscall

hello:
    # write(1, message, 14);
    mov     rax, 1
    mov     rdi, 1
    lea     rsi, [rip + message]
    mov     rdx, 14
    syscall
    ret
