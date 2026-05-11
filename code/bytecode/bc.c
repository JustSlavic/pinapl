#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "interpreter.h"

#define ARRAY_COUNT(A) (sizeof(A) / sizeof(A[0]))

bytecode instruction_stream[] =
{
    { .opcode = BYTECODE_MOV_RI, .r0 = BYTECODE_R0, .imm = 0x01 },
    { .opcode = BYTECODE_MOV_RI, .r0 = BYTECODE_R1, .imm = 0xc0fe },

    { .opcode = BYTECODE_MOV_RI, .r0 = BYTECODE_R2, .imm = 0xff },
    { .opcode = BYTECODE_STR8_RI, .r0 = BYTECODE_R2, .imm = 0x70 },
};


int main()
{
    bytecode_interpreter interpreter = {};
    interpreter.memory = malloc(0x1000);
    interpreter.memory_size = 0x1000;

    uint64_t byte_offset = 0;
    uint64_t instruction_index = 0;
    for (; instruction_index < ARRAY_COUNT(instruction_stream); instruction_index++)
    {
        uint64_t advance = bytecode_encode(interpreter.memory + byte_offset, interpreter.memory_size, instruction_stream[instruction_index]);
        if (advance == 0)
        {
            break;
        }
        byte_offset += advance;
    }

    bytecode i = { .opcode = BYTECODE_MOV_RI, .r0 = BYTECODE_R8, .imm = 0x22 };
    bytecode_encode_x86_64(interpreter.memory + byte_offset, interpreter.memory_size, i);

    int32_t ec = 0;
    do
    {
        ec = bytecode_interpreter_step(&interpreter);
    }
    while (ec == 0);
    bytecode_interpreter_print_state(&interpreter);

    return 0;
}

#include "bytecode.c"
#include "interpreter.c"
