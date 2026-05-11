#include "interpreter.h"
#include <stdio.h>


int32_t bytecode_interpreter_step(bytecode_interpreter *interp)
{
    bytecode bc;
    uint64_t advance = bytecode_decode(interp->memory + interp->registers[BYTECODE_RIP], interp->memory_size - interp->registers[BYTECODE_RIP], &bc);
    if (advance == 0)
    {
        return 1;
    }

    interp->registers[BYTECODE_RIP] += advance;

    switch (bc.opcode)
    {
        case BYTECODE_MOV_RI:
        {
            printf("mov r%d, 0x%x\n", bc.r0, bc.imm);
            interp->registers[bc.r0] = bc.imm;
        }
        break;

        case BYTECODE_MOV_RR:
        {
            printf("mov r%d, r%d\n", bc.r0, bc.r1);
            interp->registers[bc.r0] = interp->registers[bc.r1];
        }
        break;

        case BYTECODE_LDR8_RI:
        {
            printf("ldr r%d, byte [0x%x]\n", bc.r0, bc.imm);
            interp->registers[bc.r0] = *(uint8_t *) (interp->memory + bc.imm);
        }
        break;

        case BYTECODE_LDR16_RI:
        {
            printf("ldr r%d, word [0x%x]\n", bc.r0, bc.imm);
            interp->registers[bc.r0] = *(uint16_t *) (interp->memory + bc.imm);
        }
        break;

        case BYTECODE_LDR32_RI:
        {
            printf("ldr r%d, dword [0x%x]\n", bc.r0, bc.imm);
            interp->registers[bc.r0] = *(uint32_t *) (interp->memory + bc.imm);
        }
        break;

        case BYTECODE_LDR64_RI:
        {
            printf("ldr r%d, qword [0x%x]\n", bc.r0, bc.imm);
            interp->registers[bc.r0] = *(uint64_t *) (interp->memory + bc.imm);
        }
        break;

        case BYTECODE_LDR8_RA:
        case BYTECODE_LDR16_RA:
        case BYTECODE_LDR32_RA:
        case BYTECODE_LDR64_RA:
        {
            printf("ldr r%d, %s [", bc.r0, bc.opcode == BYTECODE_LDR8_RA ? "byte" :
                                           bc.opcode == BYTECODE_LDR16_RA ? "word" :
                                           bc.opcode == BYTECODE_LDR32_RA ? "dword" :
                                           bc.opcode == BYTECODE_LDR64_RA ? "qword" : "???");
            if (bc.cc) printf("%d*r%d", (1 << bc.c), bc.r1);
            if (bc.cr)
            {
                if (bc.cc) printf(" + ");
                printf("r%d", bc.r2);
            }
            if (bc.a)
            {
                if (bc.cc || bc.cr) printf(" + ");
                printf(" %d", bc.a);
            }
            printf("]\n");

            uint32_t address = bc.cc * (1 << bc.c) * interp->registers[bc.r1] + bc.cr * interp->registers[bc.r2] + bc.a;
            if (bc.opcode == BYTECODE_LDR8_RA)
                interp->registers[bc.r0] = *(uint8_t *) (interp->memory + address);
            if (bc.opcode == BYTECODE_LDR16_RA)
                interp->registers[bc.r0] = *(uint16_t *) (interp->memory + address);
            if (bc.opcode == BYTECODE_LDR32_RA)
                interp->registers[bc.r0] = *(uint32_t *) (interp->memory + address);
            if (bc.opcode == BYTECODE_LDR64_RA)
                interp->registers[bc.r0] = *(uint64_t *) (interp->memory + address);
        }
        break;

        case BYTECODE_STR8_RI:
        {
            printf("str r%d, byte [0x%x]\n", bc.r0, bc.imm);
            *(uint8_t *) (interp->memory + bc.imm) = interp->registers[bc.r0];
        }
        break;

        case BYTECODE_STR16_RI:
        {
            printf("str r%d, word [0x%x]\n", bc.r0, bc.imm);
            *(uint16_t *) (interp->memory + bc.imm) = interp->registers[bc.r0];
        }
        break;

        case BYTECODE_STR32_RI:
        {
            printf("str r%d, dword [0x%x]\n", bc.r0, bc.imm);
            *(uint32_t *) (interp->memory + bc.imm) = interp->registers[bc.r0];
        }
        break;

        case BYTECODE_STR64_RI:
        {
            printf("str r%d, qword [0x%x]\n", bc.r0, bc.imm);
            *(uint64_t *) (interp->memory + bc.imm) = interp->registers[bc.r0];
        }
        break;

        case BYTECODE_STR8_RA:
        case BYTECODE_STR16_RA:
        case BYTECODE_STR32_RA:
        case BYTECODE_STR64_RA:
        {
            printf("str r%d, %s [", bc.r0, bc.opcode == BYTECODE_STR8_RA ? "byte" :
                                           bc.opcode == BYTECODE_STR16_RA ? "word" :
                                           bc.opcode == BYTECODE_STR32_RA ? "dword" :
                                           bc.opcode == BYTECODE_STR64_RA ? "qword" : "???");
            if (bc.cc) printf("%d*r%d", (1 << bc.c), bc.r1);
            if (bc.cr)
            {
                if (bc.cc) printf(" + ");
                printf("r%d", bc.r2);
            }
            if (bc.a)
            {
                if (bc.cc || bc.cr) printf(" + ");
                printf(" %d", bc.a);
            }
            printf("]\n");

            uint32_t address = bc.cc * (1 << bc.c) * interp->registers[bc.r1] + bc.cr * interp->registers[bc.r2] + bc.a;
            if (bc.opcode == BYTECODE_STR8_RA)
                *(uint8_t *) (interp->memory + address) = (uint8_t) interp->registers[bc.r0];
            if (bc.opcode == BYTECODE_STR16_RA)
                *(uint16_t *) (interp->memory + address) = (uint16_t) interp->registers[bc.r0];
            if (bc.opcode == BYTECODE_STR32_RA)
                *(uint32_t *) (interp->memory + address) = (uint32_t) interp->registers[bc.r0];
            if (bc.opcode == BYTECODE_STR64_RA)
                *(uint64_t *) (interp->memory + address) = (uint64_t) interp->registers[bc.r0];
        }
        break;

        case BYTECODE_ADD_RRI:
        {
            printf("add r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] + bc.imm;
        }
        break;

        case BYTECODE_ADD_RRR:
        {
            printf("add r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] + interp->registers[bc.r2];
        }
        break;

        case BYTECODE_SUB_RRI:
        {
            printf("sub r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] - bc.imm;
        }
        break;

        case BYTECODE_SUB_RRR:
        {
            printf("sub r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] - interp->registers[bc.r2];
        }
        break;

        case BYTECODE_MUL_RRI:
        {
            printf("mul r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] * bc.imm;
        }
        break;

        case BYTECODE_MUL_RRR:
        {
            printf("mul r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] * interp->registers[bc.r2];
        }
        break;

        case BYTECODE_AND_RRI:
        {
            printf("and r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] & bc.imm;
        }
        break;

        case BYTECODE_AND_RRR:
        {
            printf("and r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] & interp->registers[bc.r2];
        }
        break;

        case BYTECODE_OR_RRI:
        {
            printf("or r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] | bc.imm;
        }
        break;

        case BYTECODE_OR_RRR:
        {
            printf("or r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] | interp->registers[bc.r2];
        }
        break;

        case BYTECODE_XOR_RRI:
        {
            printf("xor r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] ^ bc.imm;
        }
        break;

        case BYTECODE_XOR_RRR:
        {
            printf("xor r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] ^ interp->registers[bc.r2];
        }
        break;

        case BYTECODE_NOT_RR:
        {
            printf("not r%d, r%d\n", bc.r0, bc.r1);
            interp->registers[bc.r0] = ~interp->registers[bc.r1];
        }
        break;

        case BYTECODE_SHR_RRI:
        {
            printf("shr r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] >> bc.imm;
        }
        break;

        case BYTECODE_SHR_RRR:
        {
            printf("shr r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] >> interp->registers[bc.r2];
        }
        break;

        case BYTECODE_SHL_RRI:
        {
            printf("shl r%d, r%d, 0x%x\n", bc.r0, bc.r1, bc.imm);
            interp->registers[bc.r0] = interp->registers[bc.r1] << bc.imm;
        }
        break;

        case BYTECODE_SHL_RRR:
        {
            printf("shl r%d, r%d, r%d\n", bc.r0, bc.r1, bc.r2);
            interp->registers[bc.r0] = interp->registers[bc.r1] << interp->registers[bc.r2];
        }
        break;

        case BYTECODE_CMP_RRI:
        case BYTECODE_CMP_RRR:
        case BYTECODE_JMP_I:
        case BYTECODE_JE_I:
        case BYTECODE_JNE_I:
        case BYTECODE_JL_I:
        case BYTECODE_JLE_I:
        case BYTECODE_JG_I:
        case BYTECODE_JGE_I:
        case BYTECODE_SETE_R:
        case BYTECODE_SETNE_R:
        case BYTECODE_CALL_I:
        case BYTECODE_RET:
        case BYTECODE_SYSCALL:

        case BYTECODE_INVALID:
        default:
            printf("Unknown instruction [0x%08x]\n", bc.opcode);
            return 1;
    }
    return 0;
}

void bytecode_interpreter_print_state(bytecode_interpreter *interp)
{
    int i, j;

    printf("Registers:            Address      | Memory                  | Ascii\n");
    for (i = 0; i < 16; i++)
    {
        if (i < 10) printf(" ");
        printf("r%d = %10lx      ", i, interp->registers[i]);
        printf("0x%010x |", i * 8);
        for (j = 0; j < 8; j++)
        {
            printf(" %02x", interp->memory[i*8 + j] & 0xff);
        }
        printf(" | ");
        for (j = 0; j < 8; j++)
        {
            char c = interp->memory[i*8 + j];
            printf("%c", c < 'A' ? '.' : c);
        }
        printf("\n");
    }
}
