#ifndef IR0_STREAM_H
#define IR0_STREAM_H

#include "common.h"
#include "string_view.h"


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
