#ifndef PINAPL_INTERPRETER_H_
#define PINAPL_INTERPRETER_H_

#include <stdint.h>
#include "bytecode.h"

enum
{
    INTERPRETER_FLAG_EQUAL = 0x1,
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
