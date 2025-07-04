#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "ascii.h"
#include "ir0_stream.h"
#include "ir0_lexer.h"


void ir0_push_instruction1u(ir0 *ir0, ir0_tag tag,
                            ir0_arg_tag arg_tag, uint32 arg_value)
{
    ir0_instruction instruction = { .tag = tag };
    instruction.args[0] = (ir0_arg)
    {
        .tag = arg_tag,
        .u32 = arg_value,
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
            .label = make_string_view_from_cstring(label_name),
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

void ir0_push_label_string_view(ir0 *stream, string_view label_name)
{
    if (stream->label_count < stream->label_capacity)
    {
        uint32 index_in_buffer = stream->label_buffer_count;
        uint32 index_of_label = stream->label_count;

        stream->label_index_in_buffer[index_of_label] = index_in_buffer;
        stream->label_at[index_of_label] = stream->count;
        stream->label_count += 1;

        for (uint32 i = 0; i < label_name.size; i++)
        {
            stream->label_buffer[stream->label_buffer_count++] = label_name.data[i];
        }
    }
}

uint32 ir0_find_label(ir0 *stream, string_view label)
{
    for (uint32 label_index = 0; label_index < stream->label_count; label_index++)
    {
        uint32 index_in_buffer = stream->label_index_in_buffer[label_index];
        string_view buffer_substring =
        {
            .data = stream->label_buffer + index_in_buffer,
            .size = label.size,
        };

        if (string_view_equal(label, buffer_substring))
            return label_index;
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

#define DEFINE_INTERPRETER_JUMP_OPERATION_1(CmpName, OpName) \
    int ir0_interpreter_##OpName(ir0_interpreter *interpreter, ir0_instruction instruction) { \
        if (interpreter->cmp_result == (CmpName)) { \
            if (instruction.arg1.tag == Ir0_ArgumentLabel) { \
                uint32 index = ir0_find_label(interpreter->stream, instruction.arg1.label); \
                uint32 at = interpreter->stream->label_at[index]; \
                interpreter->reg_ip.r_u32 = at; \
                return 1; \
            } else { \
                printf("JEQ: Invalid instruction argument!\n"); \
            } \
        } \
        return 0; \
    }

DEFINE_INTERPRETER_JUMP_OPERATION_1(Ir0_Equal, jeq)
DEFINE_INTERPRETER_JUMP_OPERATION_1(Ir0_Less,  jlt)
DEFINE_INTERPRETER_JUMP_OPERATION_1(Ir0_More,  jgt)

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

// LABEL(fib);
//     MOV_RI(R0, 1)
//     MOV_RI(R1, 1)
// LABEL(L0)
//     ADD_RR(R0, R1)
//     STR_RR(R0, R3)
//     ADD_RI(R3, 2)

//     ADD_RR(R1, R0)
//     STR_RR(R1, R3)
//     ADD_RI(R3, 2)

//     CMP_RI(R0, 100)
//     JLT_L(L0)

    ir0_interpreter interpreter =
    {
        .stream = &stream,
        .memory = malloc(1 << 10),
        .memory_size = 1 << 10,
    };

    char source[] =
        "mov     r0, 10\n"
        "mov     r1, 11\n"
        "mov     r2, 12\n"
        "mov     r3, 13\n"
        "mov     r4, 14\n"
        ;

    uint32 keyword_count = 3;
    keyword_pair *keywords = malloc(keyword_count * sizeof(keyword_pair));
    keywords[0].keyword = make_string_view_from_cstring("mov");
    keywords[0].tag = Token_KeywordMov;
    keywords[1].keyword = make_string_view_from_cstring("add");
    keywords[1].tag = Token_KeywordAdd;
    keywords[2].keyword = make_string_view_from_cstring("sub");
    keywords[2].tag = Token_KeywordSub;

    lexer lexer =
    {
        .data = (uint8 *) source,
        .size = sizeof(source),

        .cursor = 0,
        .line = 1,
        .column = 1,

        .keywords = {
            .pairs = keywords,
            .count = keyword_count,
        },
    };

    while (true)
    {
        token t = get_token(&lexer);
        if (t.tag == Token_Identifier)
        {
            // Label
            eat_token(&lexer);
            t = get_token(&lexer);
            if (t.tag == ':')
            {
                ir0_push_label_string_view(&stream, t.span);
            }
        }
        else if ((t.tag & Token_Keyword) > 0)
        {
            // Instruction
            eat_token(&lexer);
            ir0_tag instruction_tag = Ir0_Unknown;
            if ((t.span.size == 3) &&
                (t.span.data[0] == 'm') &&
                (t.span.data[1] == 'o') &&
                (t.span.data[2] == 'v'))
            {
                instruction_tag = Ir0_Mov;
            }

            int32 at_line = t.line;

            ir0_arg args[3] = {};
            uint32 count = 0;
            for (int i = 0; i < 3; i++)
            {
                token t1 = get_token(&lexer);
                if (t1.line != at_line) break;
                eat_token(&lexer);
                token t2 = get_token(&lexer);

                if (t1.tag == Token_Identifier || t1.tag == Token_LiteralInteger)
                {
                    count += 1;
                    if (t1.tag == Token_Identifier)
                    {
                        if ((t1.span.size == 2)
                            && (t1.span.data[0] == 'r')
                            && is_ascii_digit(t1.span.data[1]))
                        {
                            args[i].tag = Ir0_ArgumentRegister;
                            args[i].u32 = (t1.span.data[1] - '0');
                        }
                        else if ((t1.span.size == 3)
                                 && (t1.span.data[0] == 'r')
                                 && is_ascii_digit(t1.span.data[1])
                                 && is_ascii_digit(t1.span.data[2]))
                        {
                            args[i].tag = Ir0_ArgumentRegister;
                            args[i].u32 = (t1.span.data[1] - '0') * 10
                                        + (t1.span.data[2] - '0');
                        }
                        else
                        {
                            args[i].tag = Ir0_ArgumentLabel;
                        }
                    }
                    if (t1.tag == Token_LiteralInteger)
                    {
                        args[i].tag = Ir0_ArgumentImmediate;
                        args[i].u32 = t1.integer_value;
                    }
                    if ((t2.line == at_line) && (t2.tag = ','))
                    {
                        eat_token(&lexer);
                    }
                }
            }

            if (count == 2)
            {
                ir0_push_instruction2(&stream,
                    instruction_tag,
                    args[0].tag,
                    args[0].u32,
                    args[1].tag,
                    args[1].u32);
            }
        }
        else
        {
            break;
        }
    }
    ir0_interpreter_run(&interpreter);
    ir0_interpreter_print_state(&interpreter);

    return 0;
}

#include "ascii.c"
#include "lexer.c"
#include "ir0_lexer.c"
#include "string_view.c"
