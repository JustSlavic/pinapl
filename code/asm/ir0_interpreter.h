#ifndef IR0_INTERPRETER_H
#define IR0_INTERPRETER_H

#include "common.h"


typedef enum ir0_cmp_result
{
    Ir0_Neither,
    Ir0_Equal,
    Ir0_Less,
    Ir0_More,
} ir0_cmp_result;

typedef struct ir0_register
{
    uint32 r_u32;
} ir0_register;

typedef struct ir0_interpreter
{
    ir0_register registers[8];
    ir0_register reg_ip;
    ir0_stream *stream;
    ir0_cmp_result cmp_result;

    uint8 *memory;
    uint64 memory_size;
} ir0_interpreter;


void ir0_interpreter_run(ir0_interpreter *);
void ir0_interpreter_print_state(ir0_interpreter *);


#endif // IR0_INTERPRETER_H
