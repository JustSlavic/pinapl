#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "interpreter.h"

#define ARRAY_COUNT(A) (sizeof(A) / sizeof(A[0]))

bytecode instruction_stream[] =
{
    { .opcode = BYTECODE_MOV_RI, .r0 = BYTECODE_R0, .imm = 0xff },
    { .opcode = BYTECODE_MOV_RI, .r0 = BYTECODE_R1, .imm = 0xc0fe },
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

    bytecode_interpreter_print_state(&interpreter);

    return 0;
}

#include "bytecode.c"
#include "interpreter.c"
