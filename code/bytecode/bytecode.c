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
        case BYTECODE_LDR_RA:
        case BYTECODE_STR_RA:
        case BYTECODE_LDR8_RA:
        case BYTECODE_STR8_RA:
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
        case BYTECODE_LDR_RI:
        case BYTECODE_STR_RI:
        case BYTECODE_LDR8_RI:
        case BYTECODE_STR8_RI:
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
        case BYTECODE_LDR_RA:
        case BYTECODE_STR_RA:
        case BYTECODE_LDR8_RA:
        case BYTECODE_STR8_RA:
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
        case BYTECODE_LDR_RI:
        case BYTECODE_STR_RI:
        case BYTECODE_LDR8_RI:
        case BYTECODE_STR8_RI:
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

    return 0;
}
