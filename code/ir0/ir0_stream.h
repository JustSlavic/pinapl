#ifndef IR0_STREAM_H
#define IR0_STREAM_H

#include "common.h"
#include "string_view.h"

/*
    The instruction is 4 bytes:
    +--------+--------+--------+--------+--------+
    | OpCode | Reg  1 | Reg  2 | C|Imm1 | A|Imm2 |
    +--------+--------+--------+--------+--------+
    | 8 bits | 4 bits | 4 bits | 8 bits | 8 bits |
    +--------+--------+--------+--------+--------+
*/


#define BYTECODE_OPCODE_MASK 0xff000000
#define BYTECODE_REG1_MASK   0x00f00000
#define BYTECODE_REG2_MASK   0x000f0000
#define BYTECODE_IMM1_MASK   0x0000ff00
#define BYTECODE_IMM2_MASK   0x000000ff

typedef uint32 bytecode;

typedef enum bytecode_opcode
{
    Bytecode_Ret   = 0b00001,
    Bytecode_MovRR = 0b00010,
    Bytecode_MovRI = 0b00011,
    Bytecode_LdrRA = 0b00100,
    Bytecode_LdrIA = 0b00101,
    Bytecode_StrRA = 0b00110,
    Bytecode_StrIA = 0b00111,

    Bytecode_AddRR = 0b01000,
    Bytecode_AddRI = 0b01001,
    Bytecode_SubRR = 0b01010,
    Bytecode_SubRI = 0b01011,
    Bytecode_MulRR = 0b01100,
    Bytecode_MulRI = 0b01101,
    Bytecode_DivRR = 0b01110,
    Bytecode_DivRI = 0b01111,

    Bytecode_CmpRR = 0b10000,
    Bytecode_CmpRI = 0b10001,
    Bytecode_CmpIR = 0b10010,

    Bytecode_Jmp   = 0b10011,
    Bytecode_Jeq   = 0b10100,
    Bytecode_Jne   = 0b10101,
    Bytecode_Jlt   = 0b10110,
    Bytecode_Jle   = 0b10111,
    Bytecode_Jgt   = 0b11000,
    Bytecode_Jge   = 0b11001,
} bytecode_opcode;


typedef enum ir0_tag
{
    Ir0_Unknown = 0,
    Ir0_Mov,
    Ir0_Ldr,
    Ir0_Str,
    Ir0_Add,
    Ir0_Sub,
    Ir0_Mul,
    Ir0_Div,
    Ir0_Ret,

    Ir0_Cmp,

    Ir0_Jmp,
    Ir0_Jeq,
    Ir0_Jne,
    Ir0_Jlt,
    Ir0_Jle,
    Ir0_Jgt,
    Ir0_Jge,
} ir0_tag;

typedef enum ir0_arg_tag
{
    Ir0_ArgumentUnknown = 0,
    Ir0_ArgumentImmediate,
    Ir0_ArgumentRegister,
    Ir0_ArgumentAddress,
    Ir0_ArgumentLabel,
} ir0_arg_tag;

typedef struct ir0_address_calculation
{
    // c * r1 + r2 + a
    int32 c;
    int32 r1;
    int32 r2;
    int32 a;
} ir0_address_calculation;

typedef struct ir0_arg
{
    ir0_arg_tag tag;
    uint32 u32;
    ir0_address_calculation addr;
} ir0_arg;

typedef struct ir0_instruction
{
    ir0_tag tag;
    ir0_arg args[3];
    uint32 arg_count;
} ir0_instruction;

typedef struct ir0_stream
{
    ir0_instruction *instructions;
    uint32 count;
    uint32 capacity;

    char   *label_buffer;
    uint32  label_buffer_size;
    uint32  label_buffer_capacity;

    string_view *labels;
     int32      *label_at;
    uint32       label_count;
    uint32       label_capacity;
} ir0_stream;


void ir0_push_0(ir0_stream *stream, int32 tag);
// void ir0_push_1r(ir0_stream *stream);
// void ir0_push_1u(ir0_stream *stream);
// void ir0_push_1l(ir0_stream *stream);
// void ir0_push_2ru(ir0_stream *stream);
// void ir0_push_2rr(ir0_stream *stream);
// void ir0_push_3rrr(ir0_stream *stream);
// void ir0_push_3rru(ir0_stream *stream);
int32 ir0_push_label_at(ir0_stream *stream, string_view label_name, int32 at);
int32 ir0_push_label(ir0_stream *stream, string_view label_name);
int32 ir0_find_label(ir0_stream *stream, string_view label);

#endif // IR0_STREAM_H
