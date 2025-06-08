#include "ir0_interpreter.h"


void ir0_interpreter_run(ir0_interpreter *interpreter)
{
    if (interpreter->stream)
    {
        while (interpreter->reg_ip.r_u32 < interpreter->stream->count)
        {
            ir0_instruction instruction = interpreter->stream->instructions[interpreter->reg_ip.r_u32];
            int jumped = 0;
            switch (instruction.tag)
            {
                case Ir0_Mov:
                    // ir0_interpreter_mov(interpreter, instruction);
                    printf("Mov\n");
                    break;
                case Ir0_Ldr:
                    // ir0_interpreter_ldr(interpreter, instruction);
                    printf("Ldr\n");
                    break;
                case Ir0_Str:
                    // ir0_interpreter_str(interpreter, instruction);
                    printf("Str\n");
                    break;
                case Ir0_Add:
                    // ir0_interpreter_add(interpreter, instruction);
                    printf("Add\n");
                    break;
                case Ir0_Sub:
                    // ir0_interpreter_sub(interpreter, instruction);
                    printf("Sub\n");
                    break;
                case Ir0_Mul:
                    // ir0_interpreter_mul(interpreter, instruction);
                    printf("Mul\n");
                    break;
                case Ir0_Div:
                    // ir0_interpreter_div(interpreter, instruction);
                    printf("Div\n");
                    break;
                case Ir0_Cmp:
                    // ir0_interpreter_cmp(interpreter, instruction);
                    printf("Cmp\n");
                    break;
                case Ir0_Jeq:
                    // jumped = ir0_interpreter_jeq(interpreter, instruction);
                    printf("Jeq\n");
                    break;
                case Ir0_Jlt:
                    // jumped = ir0_interpreter_jlt(interpreter, instruction);
                    printf("Jlt\n");
                    break;
                case Ir0_Jgt:
                    // jumped = ir0_interpreter_jgt(interpreter, instruction);
                    printf("Jgt\n");
                    break;
                default:
                    printf("Error: Unknown Instruction!\n");
                    return;
            }
            interpreter->reg_ip.r_u32 += 1 * (!jumped);
        }
    }
}

void ir0_interpreter_print_state(ir0_interpreter *interpreter)
{
    (void) interpreter;
}
