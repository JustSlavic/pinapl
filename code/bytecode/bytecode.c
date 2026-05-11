#include "bytecode.h"


uint64_t bytecode_encode(void *data, uint64_t size, bytecode bc)
{
    if (size < 4) return 0;

    uint32_t encoded = ((bc.opcode & 0xff) << 24);
    switch (bc.opcode)
    {
        /* No arguments */
        case BYTECODE_RET:
        case BYTECODE_SYSCALL:
        break;

        /* Register */
        case BYTECODE_SETE_R:
        case BYTECODE_SETNE_R:
        {
            encoded = encoded | ((bc.r0 & 0xf) << 20);
        }
        break;

        /* Register Register */
        case BYTECODE_MOV_RR:
        case BYTECODE_NOT_RR:
        {
            encoded = encoded | ((bc.r0 & 0xf) << 20);
            encoded = encoded | ((bc.r1 & 0xf) << 16);
        }
        break;

        /* Register Address */
        case BYTECODE_LDR8_RA:
        case BYTECODE_LDR16_RA:
        case BYTECODE_LDR32_RA:
        case BYTECODE_LDR64_RA:
        case BYTECODE_STR8_RA:
        case BYTECODE_STR16_RA:
        case BYTECODE_STR32_RA:
        case BYTECODE_STR64_RA:
        {
            encoded = encoded | ((bc.r0 & 0xf) << 20);
            encoded = encoded | ((bc.cc & 0x1) << 19);
            encoded = encoded | ((bc.cr & 0x1) << 18);
            encoded = encoded | ((bc.c  & 0x3) << 16);
            encoded = encoded | ((bc.r1 & 0xf) << 12);
            encoded = encoded | ((bc.r2 & 0xf) << 8);
            encoded = encoded |  (bc.a  & 0xff);
        }
        break;

        case BYTECODE_MOV_RI:
        case BYTECODE_LDR8_RI:
        case BYTECODE_LDR16_RI:
        case BYTECODE_LDR32_RI:
        case BYTECODE_LDR64_RI:
        case BYTECODE_STR8_RI:
        case BYTECODE_STR16_RI:
        case BYTECODE_STR32_RI:
        case BYTECODE_STR64_RI:
        {
            encoded = encoded | ((bc.r0 & 0xf) << 20);
            encoded = encoded |  (bc.imm & 0xfffff);
        }
        break;

        case BYTECODE_ADD_RRI:
        case BYTECODE_SUB_RRI:
        case BYTECODE_MUL_RRI:
        case BYTECODE_AND_RRI:
        case BYTECODE_OR_RRI:
        case BYTECODE_XOR_RRI:
        case BYTECODE_SHR_RRI:
        case BYTECODE_SHL_RRI:
        case BYTECODE_CMP_RRI:
        {
            encoded = encoded | ((bc.r0 & 0xf) << 20);
            encoded = encoded | ((bc.r1 & 0xf) << 16);
            encoded = encoded |  (bc.imm & 0xffff);
        }
        break;

        case BYTECODE_ADD_RRR:
        case BYTECODE_SUB_RRR:
        case BYTECODE_MUL_RRR:
        case BYTECODE_AND_RRR:
        case BYTECODE_OR_RRR:
        case BYTECODE_XOR_RRR:
        case BYTECODE_SHR_RRR:
        case BYTECODE_SHL_RRR:
        case BYTECODE_CMP_RRR:
        {
            encoded = encoded | ((bc.r0 & 0xf) << 20);
            encoded = encoded | ((bc.r1 & 0xf) << 16);
            encoded = encoded | ((bc.r2 & 0xf) << 12);
        }
        break;

        case BYTECODE_JMP_I:
        case BYTECODE_JE_I:
        case BYTECODE_JNE_I:
        case BYTECODE_JL_I:
        case BYTECODE_JLE_I:
        case BYTECODE_JG_I:
        case BYTECODE_JGE_I:
        case BYTECODE_CALL_I:
        {
            encoded = encoded | (bc.imm & 0xffffff);
        }
        break;

        case BYTECODE_INVALID:
        default:
            return 0;
    }

    *(uint32_t *) data = encoded;
    return 4;
}

uint64_t bytecode_decode(void *data, uint64_t size, bytecode *bc)
{
    if (size < 4) return 0;

    uint32_t encoded = *(uint32_t *) data;
    bc->opcode = (encoded >> 24) & 0xff;
    switch (bc->opcode)
    {
        /* No arguments */
        case BYTECODE_RET:
        case BYTECODE_SYSCALL:
        break;

        /* Register */
        case BYTECODE_SETE_R:
        case BYTECODE_SETNE_R:
        {
            bc->r0 = (encoded >> 20) & 0xf;
        }
        break;

        /* Register Register */
        case BYTECODE_MOV_RR:
        case BYTECODE_NOT_RR:
        {
            bc->r0 = (encoded >> 20) & 0xf;
            bc->r1 = (encoded >> 16) & 0xf;
        }
        break;

        /* Register Address */
        case BYTECODE_LDR8_RA:
        case BYTECODE_LDR16_RA:
        case BYTECODE_LDR32_RA:
        case BYTECODE_LDR64_RA:
        case BYTECODE_STR8_RA:
        case BYTECODE_STR16_RA:
        case BYTECODE_STR32_RA:
        case BYTECODE_STR64_RA:
        {
            bc->r0 = (encoded >> 20) & 0xf;
            bc->cc = (encoded >> 19) & 0x1;
            bc->cr = (encoded >> 18) & 0x1;
            bc->c  = (encoded >> 16) & 0x3;
            bc->r1 = (encoded >> 12) & 0xf;
            bc->r2 = (encoded >> 8) & 0xf;
            bc->a  =  encoded & 0xff;
        }
        break;

        case BYTECODE_MOV_RI:
        case BYTECODE_LDR8_RI:
        case BYTECODE_LDR16_RI:
        case BYTECODE_LDR32_RI:
        case BYTECODE_LDR64_RI:
        case BYTECODE_STR8_RI:
        case BYTECODE_STR16_RI:
        case BYTECODE_STR32_RI:
        case BYTECODE_STR64_RI:
        {
            bc->r0 = (encoded >> 20) & 0xf;
            bc->imm = encoded & 0xfffff;
        }
        break;

        case BYTECODE_ADD_RRI:
        case BYTECODE_SUB_RRI:
        case BYTECODE_MUL_RRI:
        case BYTECODE_AND_RRI:
        case BYTECODE_OR_RRI:
        case BYTECODE_XOR_RRI:
        case BYTECODE_SHR_RRI:
        case BYTECODE_SHL_RRI:
        case BYTECODE_CMP_RRI:
        {
            bc->r0 = (encoded >> 20) & 0xf;
            bc->r1 = (encoded >> 16) & 0xf;
            bc->imm = encoded & 0xffff;
        }
        break;

        case BYTECODE_ADD_RRR:
        case BYTECODE_SUB_RRR:
        case BYTECODE_MUL_RRR:
        case BYTECODE_AND_RRR:
        case BYTECODE_OR_RRR:
        case BYTECODE_XOR_RRR:
        case BYTECODE_SHR_RRR:
        case BYTECODE_SHL_RRR:
        case BYTECODE_CMP_RRR:
        {
            bc->r0 = (encoded >> 20) & 0xf;
            bc->r1 = (encoded >> 16) & 0xf;
            bc->r2 = (encoded >> 12) & 0xf;
        }
        break;

        case BYTECODE_JMP_I:
        case BYTECODE_JE_I:
        case BYTECODE_JNE_I:
        case BYTECODE_JL_I:
        case BYTECODE_JLE_I:
        case BYTECODE_JG_I:
        case BYTECODE_JGE_I:
        case BYTECODE_CALL_I:
        {
            bc->imm = encoded & 0xffffff;
        }
        break;

        case BYTECODE_INVALID:
        default:
            return 0;
    }

    return 4;
}

uint64_t bytecode_encode_x86_64(void *data, uint64_t size, bytecode bc)
{
    /*
        Registers:
            r0  -> rax  0.000
            r1  -> rbx  0.011
            r2  -> rcx  0.001
            r3  -> rdx  0.010
            r4  -> rdi  0.111
            r5  -> rsi  0.110
            r6  -> r8   1.000
            r7  -> r9   1.001
            r8  -> r10  1.010
            r9  -> r11  1.011
            r10 -> r12  1.100
            r11 -> r13  1.101
            r12 -> r14  1.110
            r13 -> rsp  0.100
            r14 -> rbp  0.101
            r15 -> rip  0.101
    */
    uint8_t reg_codes[16] =
    {
        0x0, 0x3, 0x1, 0x2, 0x7, 0x6, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x4, 0x5, 0x5,
    };
    uint8_t *output = (uint8_t *) data;
    switch (bc.opcode)
    {
        case BYTECODE_MOV_RI:
        {
            uint8_t rex_byte = 0x48;
            if (bc.r0 >= BYTECODE_R6 && bc.r0 <= BYTECODE_R12)
                rex_byte = rex_byte | 0x1;
            uint8_t opcode = 0xc7;
            uint8_t mod_rm = 0xc0 | reg_codes[bc.r0];

            *output++ = rex_byte;
            *output++ = opcode;
            *output++ = mod_rm;
            *(uint32_t *) output = bc.imm;
        }
        break;

        case BYTECODE_MOV_RR:
        {
        }
        break;

        case BYTECODE_LDR8_RI:
        {
        }
        break;

        case BYTECODE_LDR8_RA:
        {
        }
        break;

        case BYTECODE_LDR16_RI:
        {
        }
        break;

        case BYTECODE_LDR16_RA:
        {
        }
        break;

        case BYTECODE_LDR32_RI:
        {
        }
        break;

        case BYTECODE_LDR32_RA:
        {
        }
        break;

        case BYTECODE_LDR64_RI:
        {
        }
        break;

        case BYTECODE_LDR64_RA:
        {
        }
        break;

        case BYTECODE_STR8_RI:
        {
        }
        break;

        case BYTECODE_STR8_RA:
        {
        }
        break;

        case BYTECODE_STR16_RI:
        {
        }
        break;

        case BYTECODE_STR16_RA:
        {
        }
        break;

        case BYTECODE_STR32_RI:
        {
        }
        break;

        case BYTECODE_STR32_RA:
        {
        }
        break;

        case BYTECODE_STR64_RI:
        {
        }
        break;

        case BYTECODE_STR64_RA:
        {
        }
        break;


        case BYTECODE_ADD_RRI:
        {
        }
        break;

        case BYTECODE_ADD_RRR:
        {
        }
        break;

        case BYTECODE_SUB_RRI:
        {
        }
        break;

        case BYTECODE_SUB_RRR:
        {
        }
        break;

        case BYTECODE_MUL_RRI:
        {
        }
        break;

        case BYTECODE_MUL_RRR:
        {
        }
        break;

        case BYTECODE_AND_RRI:
        {
        }
        break;

        case BYTECODE_AND_RRR:
        {
        }
        break;

        case BYTECODE_OR_RRI:
        {
        }
        break;

        case BYTECODE_OR_RRR:
        {
        }
        break;

        case BYTECODE_XOR_RRI:
        {
        }
        break;

        case BYTECODE_XOR_RRR:
        {
        }
        break;

        case BYTECODE_NOT_RR:
        {
        }
        break;

        case BYTECODE_SHR_RRI:
        {
        }
        break;

        case BYTECODE_SHR_RRR:
        {
        }
        break;

        case BYTECODE_SHL_RRI:
        {
        }
        break;

        case BYTECODE_SHL_RRR:
        {
        }
        break;

        case BYTECODE_CMP_RRI:
        {
        }
        break;

        case BYTECODE_CMP_RRR:
        {
        }
        break;

        case BYTECODE_JMP_I:
        {
        }
        break;

        case BYTECODE_JE_I:
        {
        }
        break;

        case BYTECODE_JNE_I:
        {
        }
        break;

        case BYTECODE_JL_I:
        {
        }
        break;

        case BYTECODE_JLE_I:
        {
        }
        break;

        case BYTECODE_JG_I:
        {
        }
        break;

        case BYTECODE_JGE_I:
        {
        }
        break;

        case BYTECODE_SETE_R:
        {
        }
        break;

        case BYTECODE_SETNE_R:
        {
        }
        break;

        case BYTECODE_CALL_I:
        {
        }
        break;

        case BYTECODE_RET:
        {
        }
        break;

        case BYTECODE_SYSCALL:
        {
        }
        break;

        case BYTECODE_INVALID:
        default:
            return 0;
    }

    return 4;
}
