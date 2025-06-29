#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "common.h"


typedef struct interpreter
{
    byte *memory;
    usize memory_size;

    uint32 r[16];
    uint32 ip;
} interpreter;


int32 interpreter_step(interpreter *interp);
void  interpreter_print_state(interpreter *interp);


#endif // INTERPRETER_H
