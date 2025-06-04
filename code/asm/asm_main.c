#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef enum ir0_arg_tag
{
    Ir0_ArgumentUnknown = 0,
    Ir0_ArgumentImmediate,
    Ir0_ArgumentRegister,
    Ir0_ArgumentLabel,
} ir0_arg_tag;

typedef struct ir0_arg
{
    ir0_arg_tag tag;
    union
    {
        uint32 u32;
        uint32 offset;
        char const *label_name;
    };
} ir0_arg;

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

    Ir0_Cmp,

    Ir0_Jmp,
    Ir0_Jeq,
    Ir0_Jne,
    Ir0_Jlt,
    Ir0_Jle,
    Ir0_Jgt,
    Ir0_Jge,
} ir0_tag;

typedef struct ir0_instruction
{
    ir0_tag tag;
    ir0_arg arg1, arg2;
} ir0_instruction;

typedef struct ir0
{
    ir0_instruction *stream;
    uint32 count;
    uint32 capacity;

    char   *label_buffer;
    uint32  label_buffer_count;
    uint32  label_buffer_capacity;
    uint32 *label_index_in_buffer;
    uint32 *label_at;
    uint32  label_count;
    uint32  label_capacity;
} ir0;

void ir0_push_instruction1u(ir0 *ir0, ir0_tag tag,
                            ir0_arg_tag arg_tag, uint32 arg_value)
{
    ir0_instruction instruction = {
        .tag = tag,
        .arg1 = {
            .tag = arg_tag,
            .u32 = arg_value,
        },
    };
    ir0->stream[ir0->count++] = instruction;
}

void ir0_push_instruction1l(ir0 *ir0, ir0_tag tag,
                            ir0_arg_tag arg_tag, char const *label_name)
{
    ir0_instruction instruction = {
        .tag = tag,
        .arg1 = {
            .tag = arg_tag,
            .label_name = label_name,
        },
    };
    ir0->stream[ir0->count++] = instruction;
}

void ir0_push_instruction2(ir0 *ir0, ir0_tag tag,
                           ir0_arg_tag arg1_tag, uint32 arg1_value,
                           ir0_arg_tag arg2_tag, uint32 arg2_value)
{
    ir0_instruction instruction = {
        .tag = tag,
        .arg1 = {
            .tag = arg1_tag,
            .u32 = arg1_value,
        },
        .arg2 = {
            .tag = arg2_tag,
            .u32 = arg2_value,
        },
    };
    ir0->stream[ir0->count++] = instruction;
}

void ir0_push_label(ir0 *stream, char const *label_name)
{
    if (stream->label_count < stream->label_capacity)
    {
        uint32 index_in_buffer = stream->label_buffer_count;
        uint32 index_of_label = stream->label_count;

        stream->label_index_in_buffer[index_of_label] = index_in_buffer;
        stream->label_at[index_of_label] = stream->count;
        stream->label_count += 1;

        while (*label_name)
        {
            stream->label_buffer[stream->label_buffer_count++] = *label_name++;
        }
    }
}

uint32 ir0_find_label(ir0 *stream, char const *label_name)
{
    for (uint32 label_index = 0; label_index < stream->label_count; label_index++)
    {
        uint32 index_in_buffer = stream->label_index_in_buffer[label_index];

        int found = 1;
        char const *name = label_name;
        char const *buffer_name = &stream->label_buffer[index_in_buffer];
        while (*name)
        {
            if (*name != *buffer_name)
            {
                found = 0;
                break;
            }
            name++;
            buffer_name++;
        }

        if (found) return label_index;
    }
    return -1;
}

#define IR0_PUSH_RI(I, A, B) \
    ir0_push_instruction2(&stream, Ir0_##I, Ir0_ArgumentRegister, (A), Ir0_ArgumentImmediate, (B))
#define IR0_PUSH_RR(I, A, B) \
    ir0_push_instruction2(&stream, Ir0_##I, Ir0_ArgumentRegister, (A), Ir0_ArgumentRegister, (B))
#define IR0_PUSH_II(I, A, B) \
    ir0_push_instruction2(&stream, Ir0_##I, Ir0_ArgumentImmediate, (A), Ir0_ArgumentImmediate, (B))
#define IR0_PUSH_IR(I, A, B) \
    ir0_push_instruction2(&stream, Ir0_##I, Ir0_ArgumentImmediate, (A), Ir0_ArgumentRegister, (B))
#define IR0_PUSH_L(I, L) \
    ir0_push_instruction1l(&stream, Ir0_##I, Ir0_ArgumentLabel, (#L))

#define R0 0
#define R1 1
#define R2 2
#define R3 3

#define LABEL(L) ir0_push_label(&stream, #L);

#define MOV_RI(R, I) IR0_PUSH_RI(Mov, R, I);
#define MOV_RR(R, S) IR0_PUSH_RI(Mov, R, S);
#define LDR_RI(R, I) IR0_PUSH_RI(Ldr, R, I);
#define LDR_RR(R, S) IR0_PUSH_RR(Ldr, R, S);
#define STR_RI(R, I) IR0_PUSH_RI(Str, R, I);
#define STR_RR(R, S) IR0_PUSH_RR(Str, R, S);
#define STR_II(I, J) IR0_PUSH_II(Str, I, J);
#define STR_IR(I, R) IR0_PUSH_IR(Str, I, R);
#define ADD_RI(R, I) IR0_PUSH_RI(Add, R, I);
#define ADD_RR(R, S) IR0_PUSH_RR(Add, R, S);
#define SUB_RI(R, I) IR0_PUSH_RI(Sub, R, I);
#define SUB_RR(R, S) IR0_PUSH_RR(Sub, R, S);
#define MUL_RI(R, I) IR0_PUSH_RI(Mul, R, I);
#define MUL_RR(R, S) IR0_PUSH_RR(Mul, R, S);
#define DIV_RI(R, I) IR0_PUSH_RI(Div, R, I);
#define DIV_RR(R, S) IR0_PUSH_RR(Div, R, S);

#define CMP_RI(R, I) IR0_PUSH_RI(Cmp, R, I);
#define CMP_RR(R, S) IR0_PUSH_RR(Cmp, R, R);

#define JMP_L(L) IR0_PUSH_L(Jmp, L);
#define JEQ_L(L) IR0_PUSH_L(Jeq, L);
#define JNE_L(L) IR0_PUSH_L(Jne, L);
#define JLT_L(L) IR0_PUSH_L(Jlt, L);
#define JLE_L(L) IR0_PUSH_L(Jle, L);
#define JGT_L(L) IR0_PUSH_L(Jgt, L);
#define JGE_L(L) IR0_PUSH_L(Jge, L);

typedef enum ir0_cmp_result
{
    Ir0_Neither,
    Ir0_Equal,
    Ir0_Less,
    Ir0_More,
} ir0_cmp_result;

typedef union ir0_register
{
    uint32 r_u32;
} ir0_register;

typedef struct ir0_interpreter
{
    ir0_register registers[8];
    ir0_register reg_ip;
    ir0 *stream;
    ir0_cmp_result cmp_result;

    uint8 *memory;
    uint64 memory_size;
} ir0_interpreter;

void ir0_interpreter_mov(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    if (instruction.arg1.tag == Ir0_ArgumentRegister &&
        instruction.arg2.tag == Ir0_ArgumentRegister)
    {
        interpreter->registers[instruction.arg1.u32] = interpreter->registers[instruction.arg2.u32];
    }
    if (instruction.arg1.tag == Ir0_ArgumentRegister &&
        instruction.arg2.tag == Ir0_ArgumentImmediate)
    {
        interpreter->registers[instruction.arg1.u32].r_u32 = instruction.arg2.u32;
    }
}

void ir0_interpreter_ldr(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    uint32 addr = 0;
    if (instruction.arg1.tag == Ir0_ArgumentRegister &&
        instruction.arg2.tag == Ir0_ArgumentRegister)
    {
        addr = interpreter->registers[instruction.arg2.u32].r_u32;
    }
    if (instruction.arg1.tag == Ir0_ArgumentRegister &&
        instruction.arg2.tag == Ir0_ArgumentImmediate)
    {
        addr = instruction.arg2.u32;
    }
    interpreter->registers[instruction.arg1.u32].r_u32 = *(uint32 *)(interpreter->memory + addr);
}

void ir0_interpreter_str(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    uint32 value = 0;
    uint32 addr = 0;
    if (instruction.arg1.tag == Ir0_ArgumentRegister)
    {
        value = interpreter->registers[instruction.arg1.u32].r_u32;
    }
    if (instruction.arg1.tag == Ir0_ArgumentImmediate)
    {
        value = instruction.arg1.u32;
    }
    if (instruction.arg2.tag == Ir0_ArgumentRegister)
    {
        addr = interpreter->registers[instruction.arg2.u32].r_u32;
    }
    if (instruction.arg2.tag == Ir0_ArgumentImmediate)
    {
        addr = instruction.arg2.u32;
    }
    *(uint32 *)(interpreter->memory + addr) = value;
}

void ir0_interpreter_cmp(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    uint32 value1 = 0;
    uint32 value2 = 0;
    if (instruction.arg1.tag == Ir0_ArgumentRegister)
    {
        value1 = interpreter->registers[instruction.arg1.u32].r_u32;
    }
    if (instruction.arg1.tag == Ir0_ArgumentImmediate)
    {
        value1 = instruction.arg1.u32;
    }
    if (instruction.arg2.tag == Ir0_ArgumentRegister)
    {
        value2 = interpreter->registers[instruction.arg2.u32].r_u32;
    }
    if (instruction.arg2.tag == Ir0_ArgumentImmediate)
    {
        value2 = instruction.arg2.u32;
    }
    interpreter->cmp_result = (value1 == value2) ? Ir0_Equal
                            : (value1 < value2) ? Ir0_Less
                            : Ir0_More;
}

int ir0_interpreter_jeq(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    if (interpreter->cmp_result == Ir0_Equal)
    {
        if (instruction.arg1.tag == Ir0_ArgumentLabel)
        {
            uint32 index = ir0_find_label(interpreter->stream, instruction.arg1.label_name);
            uint32 at = interpreter->stream->label_at[index];
            interpreter->reg_ip.r_u32 = at;
            return 1;
        }
        else
        {
            printf("JEQ: Invalid instruction argument!\n");
        }
    }
    return 0;
}

int ir0_interpreter_jlt(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    if (interpreter->cmp_result == Ir0_Less)
    {
        if (instruction.arg1.tag == Ir0_ArgumentLabel)
        {
            uint32 index = ir0_find_label(interpreter->stream, instruction.arg1.label_name);
            uint32 at = interpreter->stream->label_at[index];
            interpreter->reg_ip.r_u32 = at;
            return 1;
        }
        else
        {
            printf("JLT: Invalid instruction argument!\n");
        }
    }
    return 0;
}

int ir0_interpreter_jgt(ir0_interpreter *interpreter, ir0_instruction instruction)
{
    if (interpreter->cmp_result == Ir0_More)
    {
        if (instruction.arg1.tag == Ir0_ArgumentLabel)
        {
            uint32 index = ir0_find_label(interpreter->stream, instruction.arg1.label_name);
            uint32 at = interpreter->stream->label_at[index];
            interpreter->reg_ip.r_u32 = at;
            return 1;
        }
        else
        {
            printf("JGT: Invalid instruction argument!\n");
        }
    }
    return 0;
}

#define DEFINE_INTERPRETER_ARITHMETIC_OPERATION(OpSymbol, OpName) \
    void ir0_interpreter_##OpName(ir0_interpreter *interpreter, ir0_instruction instruction) { \
        if (instruction.arg1.tag == Ir0_ArgumentRegister && instruction.arg2.tag == Ir0_ArgumentRegister) { \
            interpreter->registers[instruction.arg1.u32].r_u32 = interpreter->registers[instruction.arg1.u32].r_u32 OpSymbol interpreter->registers[instruction.arg2.u32].r_u32; \
        } \
        if (instruction.arg1.tag == Ir0_ArgumentRegister && instruction.arg2.tag == Ir0_ArgumentImmediate) { \
            interpreter->registers[instruction.arg1.u32].r_u32 OpSymbol##= instruction.arg2.u32; \
        } \
    }

DEFINE_INTERPRETER_ARITHMETIC_OPERATION(+, add);
DEFINE_INTERPRETER_ARITHMETIC_OPERATION(-, sub);
DEFINE_INTERPRETER_ARITHMETIC_OPERATION(*, mul);
DEFINE_INTERPRETER_ARITHMETIC_OPERATION(/, div);

void ir0_interpreter_run(ir0_interpreter *interpreter)
{
    if (interpreter->stream)
    {
        while (interpreter->reg_ip.r_u32 < interpreter->stream->count)
        {
            ir0_instruction instruction = interpreter->stream->stream[interpreter->reg_ip.r_u32];
            int jumped = 0;
            switch (instruction.tag)
            {
                case Ir0_Mov:
                    ir0_interpreter_mov(interpreter, instruction);
                    printf("Mov\n");
                    break;
                case Ir0_Ldr:
                    ir0_interpreter_ldr(interpreter, instruction);
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
                case Ir0_Cmp:
                    ir0_interpreter_cmp(interpreter, instruction);
                    printf("Cmp\n");
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

int main()
{
    ir0 stream =
    {
        .stream = malloc(sizeof(ir0_instruction) * 1000),
        .count  = 0,
        .capacity = 1000,
        .label_buffer = malloc(1024),
        .label_buffer_count = 0,
        .label_buffer_capacity = 1024,
        .label_index_in_buffer = malloc(32 * sizeof(uint32)),
        .label_at = malloc(32 * sizeof(uint32)),
        .label_count = 0,
        .label_capacity = 32,
    };

    MOV_RI(R0, 1)
    MOV_RI(R1, 1)
LABEL(L0)
    ADD_RR(R0, R1)
    ADD_RR(R1, R0)
    CMP_RI(R0, 6)
    JLT_L(L0)

    ir0_interpreter interpreter =
    {
        .stream = &stream,
        .memory = malloc(1 << 10),
        .memory_size = 1 << 10,
    };
    ir0_interpreter_run(&interpreter);
    ir0_interpreter_print_state(&interpreter);

    return 0;
}
