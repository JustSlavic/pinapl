#ifndef PINAPL_BYTECODE_H_
#define PINAPL_BYTECODE_H_


/*
                            Bytecode

    The instruction is 4 bytes.

    First byte is opcode.

    The second byte is reserved for two registers 4 bits each.
    Therefore there could be only 16 registers. All registers are 64-bit.

    Depending on the instruction form, the next two bytes could be an immediate value,
    or effective address calculation.

    I implement very limited number of instructions in order to simplify everything.

    Having experience writing 'ttb' tool in binary, I think I can have only
    following instructions:

        +-------------------+--------+-----------+-----------+-----------+
        | Instruction       | Byte 1 | Byte 2    | Byte 3    | Byte 4    |
        +-------------------+--------+-----+-----+-----------+-----------+
        | MOV r64,imm20     |  0x01  | r64 |            imm20            |
        +-------------------+--------+-----+-----+-----------------------+
        | MOV r64,r64       |  0x02  | r64 | r64 |                       |
        +-------------------+--------+-----+-----+-----------------------+
        | LDR r64,imm20     |  0x03  | r64 |            imm20            |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | LDR r64,addr20    |  0x04  | r64 | xxC | r64 | r64 |  offset   |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | STR r64,imm20     |  0x05  | r64 |            imm20            |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | STR r64,addr20    |  0x06  | r64 | xxC | r64 | r64 |  offset   |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | LDR8 r64,imm20    |  0x07  | r64 |            imm20            |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | LDR8 r64,addr20   |  0x08  | r64 | xxC | r64 | r64 |  offset   |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | STR8 r64,imm20    |  0x09  | r64 |            imm20            |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | STR8 r64,addr20   |  0x0a  | r64 | xxC | r64 | r64 |  offset   |
        +-------------------+--------+-----+-----+-----+-----+-----------+
        | ADD r64,r64,imm16 |  0x0b  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------|
        | ADD r64,r64,r64   |  0x0c  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | SUB r64,r64,imm16 |  0x0d  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | SUB r64,r64,r64   |  0x0e  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | MUL r64,r64,imm16 |  0x0f  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | MUL r64,r64,r64   |  0x10  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | AND r64,r64,imm16 |  0x11  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | AND r64,r64,r64   |  0x12  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | OR r64,r64,imm16  |  0x13  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | OR r64,r64,r64    |  0x14  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | XOR r64,r64,imm16 |  0x15  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | XOR r64,r64,r64   |  0x16  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | NOT r64,r64       |  0x17  | r64 | r64 |                       |
        +-------------------+--------+-----+-----+-----------------------+
        | SHR r64,r64,imm16 |  0x18  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | SHR r64,r64,r64   |  0x19  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | SHL r64,r64,imm16 |  0x1a  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | SHL r64,r64,r64   |  0x1b  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | CMP r64,r64,imm16 |  0x1c  | r64 | r64 |        imm16          |
        +-------------------+--------+-----+-----+-----+-----------------+
        | CMP r64,r64,r64   |  0x1d  | r64 | r64 | r64 |                 |
        +-------------------+--------+-----+-----+-----+-----------------+
        | JMP rel24         |  0x1e  |               rel24               |
        +-------------------+--------+-----------------------------------+
        | JE rel24          |  0x1f  |               rel24               |
        +-------------------+--------+-----------------------------------+
        | JNE rel24         |  0x20  |               rel24               |
        +-------------------+--------+-----------------------------------+
        | JL rel24          |  0x21  |               rel24               |
        +-------------------+--------+-----------------------------------+
        | JLE rel24         |  0x22  |               rel24               |
        +-------------------+--------+-----------------------------------+
        | JG rel24          |  0x23  |               rel24               |
        +-------------------+--------+-----------------------------------+
        | JGE rel24         |  0x24  |               rel24               |
        +-------------------+--------+-----+-----------------------------+
        | SETE r64          |  0x25  | r64 |                             |
        +-------------------+--------+-----+-----------------------------+
        | SETNE r64         |  0x26  | r64 |                             |
        +-------------------+--------+-----+-----------------------------+
        | CALL rel24        |  0x27  |               rel24               |
        +-------------------+--------+-----------------------------------+
        | RET               |  0x28  |                                   |
        +-------------------+--------+-----------------------------------+
        | SYSCALL           |  0x29  |                                   |
        +-------------------+--------+-----------------------------------+

    The r15 register would be reserved for IP (instruction pointer).
    The r14 register would be reserved for BP (base pointer).
    The r13 register would be reserved for SP (stack pointer).

    Effective address calculation works like that: [cc*c*r1 + cr*r2 + a],
    the first two bits are coefficients for c and r2, made so you could skip them.
    If you need to skip a, set it to 0.

    The calling convention will be the following:
        - arguments are r0-r5
        - return value is on r0
        - registers r0-r5 are volatile
        - registers r6-r12 are non-volatile
*/

#include <stdint.h>


enum
{
    BYTECODE_INVALID = 0,

    BYTECODE_MOV_RI   = 0x01,
    BYTECODE_MOV_RR   = 0x02,
    BYTECODE_LDR_RI   = 0x03,
    BYTECODE_LDR_RA   = 0x04,
    BYTECODE_STR_RI   = 0x05,
    BYTECODE_STR_RA   = 0x06,
    BYTECODE_LDR8_RI  = 0x07,
    BYTECODE_LDR8_RA  = 0x08,
    BYTECODE_STR8_RI  = 0x09,
    BYTECODE_STR8_RA  = 0x0a,
    BYTECODE_ADD_RRI  = 0x0b,
    BYTECODE_ADD_RRR  = 0x0c,
    BYTECODE_SUB_RRI  = 0x0d,
    BYTECODE_SUB_RRR  = 0x0e,
    BYTECODE_MUL_RRI  = 0x0f,
    BYTECODE_MUL_RRR  = 0x10,
    BYTECODE_AND_RRI  = 0x11,
    BYTECODE_AND_RRR  = 0x12,
    BYTECODE_OR_RRI   = 0x13,
    BYTECODE_OR_RRR   = 0x14,
    BYTECODE_XOR_RRI  = 0x15,
    BYTECODE_XOR_RRR  = 0x16,
    BYTECODE_NOT_RR   = 0x17,
    BYTECODE_SHR_RRI  = 0x18,
    BYTECODE_SHR_RRR  = 0x19,
    BYTECODE_SHL_RRI  = 0x1a,
    BYTECODE_SHL_RRR  = 0x1b,
    BYTECODE_CMP_RRI  = 0x1c,
    BYTECODE_CMP_RRR  = 0x1d,
    BYTECODE_JMP_I    = 0x1e,
    BYTECODE_JE_I     = 0x1f,
    BYTECODE_JNE_I    = 0x20,
    BYTECODE_JL_I     = 0x21,
    BYTECODE_JLE_I    = 0x22,
    BYTECODE_JG_I     = 0x23,
    BYTECODE_JGE_I    = 0x24,
    BYTECODE_SETE_R   = 0x25,
    BYTECODE_SETNE_R  = 0x26,
    BYTECODE_CALL_I   = 0x27,
    BYTECODE_RET      = 0x28,
    BYTECODE_SYSCALL  = 0x29,
};

enum
{
    BYTECODE_R0  = 0x0,
    BYTECODE_R1  = 0x1,
    BYTECODE_R2  = 0x2,
    BYTECODE_R3  = 0x3,
    BYTECODE_R4  = 0x4,
    BYTECODE_R5  = 0x5,
    BYTECODE_R6  = 0x6,
    BYTECODE_R7  = 0x7,
    BYTECODE_R8  = 0x8,
    BYTECODE_R9  = 0x9,
    BYTECODE_R10 = 0xa,
    BYTECODE_R11 = 0xb,
    BYTECODE_R12 = 0xc,
    BYTECODE_R13 = 0xd,
    BYTECODE_R14 = 0xe,
    BYTECODE_R15 = 0xf,

    BYTECODE_RSP = 0xd,
    BYTECODE_RBP = 0xe,
    BYTECODE_RIP = 0xf,
};


typedef struct
{
    uint8_t opcode;
    uint8_t r0, r1, r2;
    uint8_t cc, c, cr, a;
    uint32_t imm;
} bytecode;


uint64_t bytecode_encode(void *data, uint64_t size, bytecode bc);
uint64_t bytecode_decode(void *data, uint64_t size, bytecode *bc);


#endif /* PINAPL_BYTECODE_H_ */
