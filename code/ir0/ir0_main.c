#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir0.h"
#include "../bytecode/interpreter.h"

#define ARRAY_COUNT(A) (sizeof(A) / sizeof(A[0]))

ir0_label labels[64] = {};
ir0 instruction_stream[] =
{
    { .opcode = IR0_OPCODE_LABEL, .label = "main" },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0xc0fe },
    { .opcode = IR0_OPCODE_MOV_RR, .r0 = 1, .r1 = 0 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x70 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x50 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x65 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x51 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x74 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x52 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x75 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x53 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x68 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x54 },
    /* { .opcode = IR0_OPCODE_JMP_L, .label = "main" }, */
};

int main()
{
    interpreter interpreter = {};
    interpreter.memory = malloc(0x1000);
    interpreter.memory_size = 0x1000;

    uint64_t instruction_address = 0;
    uint64_t instruction_index = 0;
    uint64_t label_count = 0;
    for (; instruction_index < ARRAY_COUNT(instruction_stream); instruction_index++)
    {
        ir0 instruction = instruction_stream[instruction_index];
        if (instruction.opcode == IR0_OPCODE_LABEL)
        {
            labels[label_count].name = instruction.label;
            labels[label_count].address = instruction_address;
            label_count += 1;
        }
        else
        {
            /* instruction->address = instruction_address; */
            if (instruction.opcode == IR0_OPCODE_JMP_L)
            {
                int label_index = 0;
                for (; label_index < label_count; label_index++)
                {
                    if (strcmp(labels[label_index].name, instruction.label) == 0)
                    {
                        instruction.imm = (int64_t) labels[label_index].address - (int64_t) instruction_address;
                        break;
                    }
                }
            }

            bytecode bc;
            bc.opcode = ir0_to_bytecode_opcode[instruction.opcode];
            bc.r0 = instruction.r0;
            bc.r1 = instruction.r1;
            bc.r2 = instruction.r2;
            bc.imm = instruction.imm;
            bc.cc = instruction.cc;
            bc.cr = instruction.cr;
            bc.c = instruction.c;
            bc.a = instruction.a;
            instruction_address += bytecode_encode(interpreter.memory + instruction_address, interpreter.memory_size - instruction_address, bc);
        }
    }
    interpreter_print_state(&interpreter);

    int32_t ec = 0;
    do
    {
        ec = interpreter_step(&interpreter);
    }
    while (ec == 0);
    interpreter_print_state(&interpreter);

    return 0;
}

#include "ir0.c"
#include "../bytecode/interpreter.c"
#include "../bytecode/bytecode.c"
