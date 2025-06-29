#ifndef BYTECODE_H
#define BYTECODE_H

/*
    The instruction is 4 bytes.

    Opcode always takes all 8 bits of the first byte.

    The second byte is reserved for two registers 4 bits each.
    Therefore there could be only 16 registers.

    Depending on the bytecode instruction form, the next two bytes
    could be either effective address calculation, having
    two bytes of multiplication constnt C and addition constant A;
    or it can be one single 16-bit immediate value.

        1) <op>
    +---------+ +----+----+ +----+----+ +---------+
    | Opcode  | |    |    | |         | |         |
    +---------+ +----+----+ +----+----+ +---------+
    | 8 bits  | | 4b | 4b | | 8 bits  | | 8 bits  |
    +---------+ +----+----+ +----+----+ +---------+

        2) <op> imm
    +---------+ +----+----+ +---------------------+
    | Opcode  | | R1 |    | |  16-bit immediate   |
    +---------+ +----+----+ +---------------------+
    | 8 bits  | | 4b | 4b | |      16 bits        |
    +---------+ +----+----+ +---------------------+

        3) <op> rN
    +---------+ +----+----+ +----+----+ +---------+
    | Opcode  | | R1 |    | |         | |         |
    +---------+ +----+----+ +----+----+ +---------+
    | 8 bits  | | 4b | 4b | | 8 bits  | | 8 bits  |
    +---------+ +----+----+ +----+----+ +---------+

        4) <op> rN, rN
    +---------+ +----+----+ +----+----+ +---------+
    | Opcode  | | R1 | R2 | |         | |         |
    +---------+ +----+----+ +----+----+ +---------+
    | 8 bits  | | 4b | 4b | | 8 bits  | | 8 bits  |
    +---------+ +----+----+ +----+----+ +---------+

        5) <op> rN, imm
    +---------+ +----+----+ +---------------------+
    | Opcode  | | R1 |    | |  16-bit immediate   |
    +---------+ +----+----+ +---------------------+
    | 8 bits  | | 4b | 4b | |      16 bits        |
    +---------+ +----+----+ +---------------------+

        6) <op> rN, rN, rN
    +---------+ +----+----+ +----+----+ +---------+
    | Opcode  | | R1 | R2 | | R3 |    | |         |
    +---------+ +----+----+ +----+----+ +---------+
    | 8 bits  | | 4b | 4b | | 4b | 4b | | 8 bits  |
    +---------+ +----+----+ +----+----+ +---------+

        7) <op> rN, rN, imm
    +---------+ +----+----+ +---------------------+
    | Opcode  | | R1 | R2 | |  16-bit immediate   |
    +---------+ +----+----+ +---------------------+
    | 8 bits  | | 4b | 4b | |      16 bits        |
    +---------+ +----+----+ +---------------------+

        8) <op> rN, [effective address]
    +---------+ +----+----+ +----+----+ +---------+
    | Opcode  | | R1 | R2 | | R3 | C  | |    A    |
    +---------+ +----+----+ +----+----+ +---------+
    | 8 bits  | | 4b | 4b | | 4b | 4b | | 8 bits  |
    +---------+ +----+----+ +----+----+ +---------+

    The Effective Address calculation could be performed in
    the following forms:
        1. [A]
        2. [rN]
        3. [C*rN]
        4. [C*rN + A]
        5. [C*rN + rN]
        6. [C*rN + rN + A]
    Where C < 256, and A < 256
*/

#define BYTECODE_ADDRESS_MASK  0xe0000000
#define BYTECODE_OPCODE_MASK   0x1f000000
#define BYTECODE_REG1_MASK     0x00f00000
#define BYTECODE_REG2_MASK     0x000f0000
#define BYTECODE_REG3_MASK     0x0000f000
#define BYTECODE_C_MASK        0x00000f00
#define BYTECODE_A_MASK        0x000000ff
#define BYTECODE_IMM_MASK      0x0000ffff

#define BYTECODE_ADDRESS_SHIFT 29
#define BYTECODE_OPCODE_SHIFT  24
#define BYTECODE_REG1_SHIFT    20
#define BYTECODE_REG2_SHIFT    16
#define BYTECODE_REG3_SHIFT    12
#define BYTECODE_C_SHIFT       8
#define BYTECODE_A_SHIFT       0
#define BYTECODE_IMM_SHIFT     0

typedef uint32 bytecode_t;

enum
{
    BytecodeAddress_Invalid, // 000
    BytecodeAddress_A,       // 001 [A]
    BytecodeAddress_CR,      // 010 [C*rN]
    BytecodeAddress_CR_A,    // 011 [C*rN + A]
    BytecodeAddress_CR_R,    // 100 [C*rN + rN]
    BytecodeAddress_CR_R_A,  // 101 [C*rN + rN + A]
};

enum
{
    Bytecode_Invalid = 0,

    Bytecode_Ret    = 0b00001,  // @todo

    Bytecode_Mov_RR = 0b00010, // r1 := r2
    Bytecode_Mov_RI = 0b00011, // r1 := imm
    Bytecode_Ldr_RA = 0b00100, // r1 := [p]
    Bytecode_Str_RA = 0b00101, // [p] := r1

    Bytecode_Add_RR = 0b01000, // r1 := r2 + r3
    Bytecode_Add_RI = 0b01001, // r1 := r2 + imm
    Bytecode_Sub_RR = 0b01010, // r1 := r2 - r3
    Bytecode_Sub_RI = 0b01011, // r1 := r2 - imm
    Bytecode_Mul_RR = 0b01100, // r1 := r2 * r3
    Bytecode_Mul_RI = 0b01101, // r1 := r2 * imm
    Bytecode_Div_RR = 0b01110, // r1 := r2 / r3
    Bytecode_Div_RI = 0b01111, // r1 := r2 / imm

    Bytecode_Cmp_RR = 0b10000, // sets flags for value (r1 - r2)
    Bytecode_Cmp_RI = 0b10001, // sets flags for value (r1 - imm)

    Bytecode_Jmp    = 0b10011, // jumps unconditionally
    Bytecode_Jeq    = 0b10100, // jumps if
    Bytecode_Jne    = 0b10101, //
    Bytecode_Jlt    = 0b10110, //
    Bytecode_Jle    = 0b10111, //
    Bytecode_Jgt    = 0b11000, //
    Bytecode_Jge    = 0b11001, //

    Bytecode_Count,
};


#endif // BYTECODE_H
