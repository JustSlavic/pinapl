#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ascii.h"
#include "parse.h"
#include "ir0.h"
#include "ir0_parser.h"
#include "../lexer.h"
#include "../bytecode/interpreter.h"

ir0_label labels[64] = {};
ir0 instruction_stream[] =
{
    { .opcode = IR0_OPCODE_LABEL, .label = "main" },

    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x48 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x100 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x65 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x101 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x6c },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x102 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x6c },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x103 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x6f },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x104 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x2c },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x105 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x20 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x106 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x57 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x107 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x6f },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x108 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x72 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x109 },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x6c },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x10a },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x64 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x10b },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x21 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x10c },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0xa },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x10d },
    { .opcode = IR0_OPCODE_MOV_RI,  .r0 = 0, .imm = 0x0 },
    { .opcode = IR0_OPCODE_STR8_RI, .r0 = 0, .imm = 0x10e },

    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 0, .imm = 0 },
    { .opcode = IR0_OPCODE_MOV_RI, .r0 = 1, .imm = 1 },
    { .opcode = IR0_OPCODE_LABEL, .label = "loop" },
    { .opcode = IR0_OPCODE_ADD_RRR, .r0 = 2, .r1 = 0, .r2 = 1, },
    { .opcode = IR0_OPCODE_MOV_RR, .r0 = 0, .r1 = 1 },
    { .opcode = IR0_OPCODE_MOV_RR, .r0 = 1, .r1 = 2 },
    { .opcode = IR0_OPCODE_CMP_RI, .r0 = 1, .imm = 1000 },
    { .opcode = IR0_OPCODE_JL_L, .label = "loop" },

    { .opcode = IR0_OPCODE_CMP_RI, .r0 = 0, .imm = 0x3dc },
    { .opcode = IR0_OPCODE_SETE_R, .r0 = 8 },
};

static char const *input_filename = "code/ir0/fib.ir0";

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

int main()
{
    int ec;

    /* Tokenization */

    struct stat st;
    ec = stat(input_filename, &st);
    if (ec < 0)
    {
        printf("Error: (errno=%d) %s\n", errno, strerror(errno));
        return 1;
    }

    int input_file = open(input_filename, O_RDONLY, 0);
    if (input_file < 0)
    {
        printf("Error: (errno=%d) %s\n", errno, strerror(errno));
        return 1;
    }

    uint32 input_size = st.st_size;
    void *input_data = mmap(0, input_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (input_data == 0)
    {
        printf("Error: Could not allocate memory\n");
        return 1;
    }

    ssize_t bytes_read = read(input_file, input_data, input_size);
    if (bytes_read < input_size)
    {
        printf("Error: File have not been read correctly (%ld bytes read of %d)\n", bytes_read, input_size);
        return 1;
    }

    lexer lexer = {};
    lexer.data = input_data;
    lexer.size = input_size;
    lexer.keyword_count = ir0_get_keywords(&lexer.keywords, &lexer.keyword_tags);

    int i = 0;
    while (i < 600)
    {
        ir0_parse_instruction(&lexer);
        /*token t = lexer_eat_token(&lexer);
        if (t.tag == TOKEN_INVALID || t.tag == TOKEN_EOF) break;
        printf("Token %d: %s("STRING_VIEW_FMT")\n", i++, ir0_token_tag_to_cstring(t.tag), STRING_VIEW_ARG(t.span));*/
        i += 1;
    }

    /* Interpreter */

    uint32_t interpreter_memory_size = 0x1000; /* 4k page */

    interpreter interpreter = {};
    interpreter.memory = malloc(interpreter_memory_size);
    interpreter.memory_size = interpreter_memory_size;

    uint64_t instruction_address = 0;
    uint64_t instruction_index = 0;
    uint64_t label_count = 0;
    for (; instruction_index < ARRAY_COUNT(instruction_stream); instruction_index++)
    {
        ir0 instruction = instruction_stream[instruction_index];
        if (instruction.opcode == IR0_OPCODE_LABEL)
        {
            labels[label_count].name = instruction.label;
            labels[label_count].address = instruction_address;
            label_count += 1;
        }
        else
        {
            /* instruction->address = instruction_address; */
            if ((instruction.opcode == IR0_OPCODE_JMP_L) ||
                (instruction.opcode == IR0_OPCODE_JL_L)  ||
                (instruction.opcode == IR0_OPCODE_JLE_L) ||
                (instruction.opcode == IR0_OPCODE_JG_L)  ||
                (instruction.opcode == IR0_OPCODE_JGE_L))
            {
                int label_index = 0;
                for (; label_index < label_count; label_index++)
                {
                    if (strcmp(labels[label_index].name, instruction.label) == 0)
                    {
                        /*
                            `-4` is because we also need to subtract the width of the current instruction,
                            but the width of the current instruction will be computed only when we call
                            bytecode_encode, so there's temporary `-4`, because bytecode width is always 4 bytes (for now?)
                            and I need to figure out a better way to compute this relative addressing,
                            or make the interpreter execute instruction first, and only then issue `rip += instruction_width`
                        */
                        instruction.imm = (int64_t) labels[label_index].address - (int64_t) instruction_address - 4;
                        break;
                    }
                }
            }

            bytecode bc;
            bc.opcode = ir0_to_bytecode_opcode[instruction.opcode];
            bc.r0 = instruction.r0;
            bc.r1 = instruction.r1;
            bc.r2 = instruction.r2;
            bc.imm = instruction.imm;
            bc.cc = instruction.cc;
            bc.cr = instruction.cr;
            bc.c = instruction.c;
            bc.a = instruction.a;
            instruction_address += bytecode_encode(interpreter.memory + instruction_address, interpreter.memory_size - instruction_address, bc);
        }
    }

    ec = 0;
    do
    {
        ec = interpreter_step(&interpreter);
    }
    while (ec == 0);
    interpreter_print_state(&interpreter);

    return 0;
}

#include "ir0.c"
#include "parse.c"
#include "ir0_parser.c"
#include "../bytecode/interpreter.c"
#include "../bytecode/bytecode.c"
#include "../lexer.c"
#include "../ascii.c"
#include "../string_view.c"
