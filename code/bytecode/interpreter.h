#ifndef PINAPL_INTERPRETER_H_
#define PINAPL_INTERPRETER_H_

#include <stdint.h>
#include "bytecode.h"

enum
{
    INTERPRETER_FLAG_EQUAL = 0x1,
    INTERPRETER_FLAG_MORE  = 0x2,
    INTERPRETER_FLAG_LESS  = 0x4,
};

typedef struct
{
    uint8_t *memory;
    uint64_t memory_size;
    uint64_t registers[16];
    uint64_t flags;
} interpreter;


int32_t interpreter_step(interpreter *interp);
void interpreter_print_state(interpreter *interp);


#endif /* PINAPL_INTERPRETER_H_ */
