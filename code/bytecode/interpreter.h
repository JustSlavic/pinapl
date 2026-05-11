#ifndef PINAPL_BYTECODE_INTERPRETER_H_
#define PINAPL_BYTECODE_INTERPRETER_H_

#include <stdint.h>
#include "bytecode.h"

enum
{
    BYTECODE_INTERPRETER_FLAG_EQUAL = 0x1,
};

typedef struct
{
    uint8_t *memory;
    uint64_t memory_size;
    uint64_t registers[16];
    uint64_t flags;
} bytecode_interpreter;


int32_t bytecode_interpreter_step(bytecode_interpreter *interp);
void bytecode_interpreter_print_state(bytecode_interpreter *interp);


#endif /* PINAPL_BYTECODE_INTERPRETER_H_ */
