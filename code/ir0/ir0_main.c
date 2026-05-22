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

    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0xfe },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x60, },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0xc0 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x61, },
    { .opcode = IR0_OPCODE_LDR16_RI, .r0 = 4, .imm = 0x60 },

    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x70 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x80 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x65 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x81 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x74 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x82 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x75 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x83 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0x68 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x84 },

    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 1, .imm = 1 },
    { .opcode = IR0_OPCODE_LABEL, .label = "loop" },
    { .opcode = IR0_OPCODE_ADD_RRR, .r0 = 2, .r1 = 0, .r2 = 1, },
    { .opcode = IR0_OPCODE_MOV_RR, .r0 = 0, .r1 = 1 },
    { .opcode = IR0_OPCODE_MOV_RR, .r0 = 1, .r1 = 2 },
    { .opcode = IR0_OPCODE_CMP_RI, .r0 = 1, .imm = 1000 },
    { .opcode = IR0_OPCODE_JL_L, .label = "loop" },

    { .opcode = IR0_OPCODE_CMP_RI, .r0 = 0, .imm = 0x3dc },
    { .opcode = IR0_OPCODE_SETE_R, .r0 = 8 },
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
            if ((instruction.opcode == IR0_OPCODE_JMP_L) ||
                (instruction.opcode == IR0_OPCODE_JL_L)  ||
                (instruction.opcode == IR0_OPCODE_JLE_L) ||
                (instruction.opcode == IR0_OPCODE_JG_L)  ||
                (instruction.opcode == IR0_OPCODE_JGE_L))
            {
                int label_index = 0;
                for (; label_index < label_count; label_index++)
                {
                    if (strcmp(labels[label_index].name, instruction.label) == 0)
                    {
                        /*
                            `-4` is because we also need to subtract the width of the current instruction,
                            but the width of the current instruction will be computed only when we call
                            bytecode_encode, so there's temporary `-4`, because bytecode width is always 4 bytes (for now?)
                            and I need to figure out a better way to compute this relative addressing,
                            or make the interpreter execute instruction first, and only then issue `rip += instruction_width`
                        */
                        instruction.imm = (int64_t) labels[label_index].address - (int64_t) instruction_address - 4;
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
