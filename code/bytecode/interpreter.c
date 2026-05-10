#include "interpreter.h"
#include <stdio.h>


int32_t bytecode_interpreter_step(bytecode_interpreter *interp)
{

}

void bytecode_interpreter_print_state(bytecode_interpreter *interp)
{
    int i, j;

    printf("Registers:            Address      | Memory                  | Ascii\n");
    for (i = 0; i < 16; i++)
    {
        if (i < 10) printf(" ");
        printf("r%d = %10lu      ", i, interp->registers[i]);
        printf("0x%010x |", i * 8);
        for (j = 0; j < 8; j++)
        {
            printf(" %02x", interp->memory[i*8 + j] & 0xff);
        }
        printf(" | ");
        for (j = 0; j < 8; j++)
        {
            char c = interp->memory[i*8 + j];
            printf("%c", c < 'A' ? '.' : c);
        }
        printf("\n");
    }
}
