#include "ir0_interpreter.h"


void ir0_interpreter_mov(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    ir0_register *dest = NULL;
    if (instruction.args[0].tag == Ir0_ArgumentRegister)
    {
        dest = interpreter->registers + instruction.args[0].u32;
    }
    else
    {
        printf("Interpreter Error: Could not use 'mov' instruction with first argument not register.\n");
        return;
    }

    uint32 value = 0;
    if (instruction.args[1].tag == Ir0_ArgumentRegister)
    {
        value = interpreter->registers[instruction.args[1].u32].r_u32;
    }
    else if (instruction.args[1].tag == Ir0_ArgumentImmediate)
    {
        value = instruction.args[1].u32;
    }
    else
    {
        printf("Interpreter Error: Could not use 'mov' instruction with second argument not register nor immediate.\n");
        return;
    }

    dest->r_u32 = value;
}

void ir0_interpreter_ldr(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    ir0_register *dest = NULL;
    if (instruction.args[0].tag == Ir0_ArgumentRegister)
    {
        dest = interpreter->registers + instruction.args[0].u32;
    }
    else
    {
        printf("Interpreter Error: Could not use 'ldr' instruction with the first argument not register.\n");
        return;
    }

    uint32 address = 0;
    if (instruction.args[1].tag == Ir0_ArgumentAddress)
    {
        address = instruction.args[1].addr.a;

        int32 r2 = instruction.args[1].addr.r2;
        if (r2 >= 0)
        {
            address += interpreter->registers[r2].r_u32;
        }

        int32 c = instruction.args[1].addr.c;
        int32 r1 = instruction.args[1].addr.r1;
        if (r1 >= 0)
        {
            address += c * interpreter->registers[r1].r_u32;
        }
    }
    else
    {
        printf("Interpreter Error: Could not use 'ldr' instruction with the second argument not effective address calculation.\n");
        return;
    }

    if (address < 0 || address >= interpreter->memory_size)
    {
        printf("Interpreter error! Trying to read memory at address 0x%x, "
               "out of allowed memory region [0x0-0x%llx]\n",
               address, interpreter->memory_size);
        return;
    }
    dest->r_u32 = *(uint32 *)(interpreter->memory + address);
}

void ir0_interpreter_str(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    uint32 src = 0;
    if (instruction.args[0].tag == Ir0_ArgumentRegister)
    {
        src = interpreter->registers[instruction.args[0].u32].r_u32;
    }
    else if (instruction.args[0].tag == Ir0_ArgumentImmediate)
    {
        src = instruction.args[0].u32;
    }
    else
    {
        printf("Interpreter Error: Could not use 'str' instruction with the first argument not register.\n");
        return;
    }

    uint32 address = 0;
    if (instruction.args[1].tag == Ir0_ArgumentAddress)
    {
        address = instruction.args[1].addr.a;

        int32 r2 = instruction.args[1].addr.r2;
        if (r2 >= 0)
        {
            address += interpreter->registers[r2].r_u32;
        }

        int32 c = instruction.args[1].addr.c;
        int32 r1 = instruction.args[1].addr.r1;
        if (r1 >= 0)
        {
            address += c * interpreter->registers[r1].r_u32;
        }
    }
    else
    {
        printf("Interpreter Error: Could not use 'ldr' instruction with the second argument not effective address calculation.\n");
        return;
    }

    printf("address = %u\n", address);
    if (address < 0 || address >= interpreter->memory_size)
    {
        printf("Interpreter error! Trying to write memory at address 0x%x, "
               "out of allowed memory region [0x0-0x%llx]\n",
               address, interpreter->memory_size);
        return;
    }
    *(uint32 *)(interpreter->memory + address) = src;
}

#define DEFINE_INTERPRETER_ARITHMETIC_OPERATION(OpSymbol, OpName) \
    void ir0_interpreter_##OpName(ir0_interpreter *interpreter, ir0_instruction instruction) { \
        ir0_register *dest = NULL; \
        if (instruction.args[0].tag == Ir0_ArgumentRegister) { \
            dest = interpreter->registers + instruction.args[0].u32; \
        } else { \
            printf("Interpreter Error: Could not use '" #OpName "' instruction with the first argument not register.\n"); \
            return; \
        } \
        uint32 value1 = 0; \
        if (instruction.args[1].tag == Ir0_ArgumentRegister) { \
            value1 = interpreter->registers[instruction.args[1].u32].r_u32; \
        } else if (instruction.args[1].tag == Ir0_ArgumentImmediate) { \
            value1 = instruction.args[1].u32; \
        } else { \
            printf("Interpreter Error: Could not use '" #OpName "' instruction with the second argument not register nor immediate.\n"); \
            return; \
        } \
        uint32 value2 = 0; \
        if (instruction.args[2].tag == Ir0_ArgumentRegister) { \
            value2 = interpreter->registers[instruction.args[2].u32].r_u32; \
        } else if (instruction.args[2].tag == Ir0_ArgumentImmediate) { \
            value2 = instruction.args[2].u32; \
        } else { \
            printf("Interpreter Error: Could not use '" #OpName "' instruction with the third argument not register nor immediate.\n"); \
            return; \
        } \
        dest->r_u32 = value1 OpSymbol value2; \
    }

DEFINE_INTERPRETER_ARITHMETIC_OPERATION(+, add);
DEFINE_INTERPRETER_ARITHMETIC_OPERATION(-, sub);
DEFINE_INTERPRETER_ARITHMETIC_OPERATION(*, mul);
DEFINE_INTERPRETER_ARITHMETIC_OPERATION(/, div);

void ir0_interpreter_cmp(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    uint32 value1 = 0;
    uint32 value2 = 0;
    if (instruction.args[0].tag == Ir0_ArgumentRegister)
        value1 = interpreter->registers[instruction.args[0].u32].r_u32;
    if (instruction.args[0].tag == Ir0_ArgumentImmediate)
        value1 = instruction.args[0].u32;
    if (instruction.args[1].tag == Ir0_ArgumentRegister)
        value2 = interpreter->registers[instruction.args[1].u32].r_u32;
    if (instruction.args[1].tag == Ir0_ArgumentImmediate)
        value2 = instruction.args[1].u32;
    interpreter->cmp_result = (value1 == value2) ? Ir0_Equal
                            : (value1 < value2) ? Ir0_Less
                            : Ir0_More;
}

bool32 ir0_interpreter_jmp(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    if (instruction.args[0].tag == Ir0_ArgumentLabel)
    {
        uint32 label_index = instruction.args[0].u32;
        int32 at = interpreter->stream->label_at[label_index];
        if (at >= 0)
            interpreter->reg_ip.r_u32 = at;
        return at >= 0;
    }
    return false;
}

#define DEFINE_INTERPRETER_JUMP_OPERATION_1(CmpSymbol, CmpName) \
    bool32 ir0_interpreter_##CmpName(ir0_interpreter *interpreter, ir0_instruction instruction) { \
        if (interpreter->cmp_result == CmpSymbol) { \
            if (instruction.args[0].tag == Ir0_ArgumentLabel) { \
                uint32 label_index = instruction.args[0].u32; \
                int32 at = interpreter->stream->label_at[label_index]; \
                if (at >= 0) interpreter->reg_ip.r_u32 = at; \
                return at >= 0; \
            } \
        } \
        return false; \
    } \

DEFINE_INTERPRETER_JUMP_OPERATION_1(Ir0_Less, jlt)
DEFINE_INTERPRETER_JUMP_OPERATION_1(Ir0_Equal, jeq)
DEFINE_INTERPRETER_JUMP_OPERATION_1(Ir0_More, jgt)

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
                    ir0_interpreter_mov(interpreter, instruction);
                    printf("Mov\n");
                    break;
                case Ir0_Ldr:
                    // ir0_interpreter_ldr(interpreter, instruction);
                    printf("Ldr\n");
                    break;
                case Ir0_Str:
                    ir0_interpreter_str(interpreter, instruction);
                    printf("Str\n");
                    break;
                case Ir0_Add:
                    ir0_interpreter_add(interpreter, instruction);
                    printf("Add\n");
                    break;
                case Ir0_Sub:
                    ir0_interpreter_sub(interpreter, instruction);
                    printf("Sub\n");
                    break;
                case Ir0_Mul:
                    ir0_interpreter_mul(interpreter, instruction);
                    printf("Mul\n");
                    break;
                case Ir0_Div:
                    ir0_interpreter_div(interpreter, instruction);
                    printf("Div\n");
                    break;
                case Ir0_Ret:
                    printf("Ret\n");
                    break;
                case Ir0_Cmp:
                    ir0_interpreter_cmp(interpreter, instruction);
                    printf("Cmp\n");
                    break;
                case Ir0_Jmp:
                    printf("Jmp\n");
                    jumped = ir0_interpreter_jmp(interpreter, instruction);
                    break;
                case Ir0_Jeq:
                    jumped = ir0_interpreter_jeq(interpreter, instruction);
                    printf("Jeq\n");
                    break;
                case Ir0_Jlt:
                    jumped = ir0_interpreter_jlt(interpreter, instruction);
                    printf("Jlt\n");
                    break;
                case Ir0_Jgt:
                    jumped = ir0_interpreter_jgt(interpreter, instruction);
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
    printf("Registers:          Memory:\n");
    for (int i = 0; i < 8; i++)
    {
        printf("R%d = %8u      ", i, interpreter->registers[i].r_u32);
        for (int j = 0; j < 8; j++)
        {
            printf(" %02x", interpreter->memory[i*8 + j]);
        }
        printf(" | ");
        for (int j = 0; j < 8; j++)
        {
            char c = interpreter->memory[i*8 + j];
            printf("%c", c < 'A' ? '.' : c);
        }
        printf("\n");
    }
    printf("CmpResult = (x %c y)\n", interpreter->cmp_result == Ir0_Equal ? '='
                                   : interpreter->cmp_result == Ir0_Less ? '<'
                                   : interpreter->cmp_result == Ir0_More ? '>'
                                   : '?');
}
