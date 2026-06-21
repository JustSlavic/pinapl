#include "ir0_parser.h"
#include <string_view.h>


enum
{
    TOKEN_KEYWORD = 0x10000,

    TOKEN_KEYWORD_MOV,
    TOKEN_KEYWORD_LDR,
    TOKEN_KEYWORD_STR,

    TOKEN_KEYWORD_ADD,
    TOKEN_KEYWORD_SUB,
    TOKEN_KEYWORD_MUL,
    TOKEN_KEYWORD_AND,
    TOKEN_KEYWORD_OR,
    TOKEN_KEYWORD_XOR,
    TOKEN_KEYWORD_NOT,
    TOKEN_KEYWORD_SHR,
    TOKEN_KEYWORD_SHL,

    TOKEN_KEYWORD_CMP,
    TOKEN_KEYWORD_JMP,
    TOKEN_KEYWORD_JE,
    TOKEN_KEYWORD_JNE,
    TOKEN_KEYWORD_JL,
    TOKEN_KEYWORD_JLE,
    TOKEN_KEYWORD_JG,
    TOKEN_KEYWORD_JGE,

    TOKEN_KEYWORD_SETE,
    TOKEN_KEYWORD_SETNE,

    TOKEN_KEYWORD_CALL,
    TOKEN_KEYWORD_RET,
    TOKEN_KEYWORD_SYSCALL,

    TOKEN_KEYWORD_BYTE,
    TOKEN_KEYWORD_WORD,
    TOKEN_KEYWORD_DWORD,
    TOKEN_KEYWORD_QWORD,

    TOKEN_KEYWORD_COUNT = (TOKEN_KEYWORD_SYSCALL - TOKEN_KEYWORD),
};

char const *ir0_token_tag_to_cstring(int tag)
{
    switch(tag)
    {
        case TOKEN_KEYWORD_MOV: return "TOKEN_KEYWORD_MOV";
        case TOKEN_KEYWORD_LDR: return "TOKEN_KEYWORD_LDR";
        case TOKEN_KEYWORD_STR: return "TOKEN_KEYWORD_STR";
        case TOKEN_KEYWORD_ADD: return "TOKEN_KEYWORD_ADD";
        case TOKEN_KEYWORD_SUB: return "TOKEN_KEYWORD_SUB";
        case TOKEN_KEYWORD_MUL: return "TOKEN_KEYWORD_MUL";
        case TOKEN_KEYWORD_AND: return "TOKEN_KEYWORD_AND";
        case TOKEN_KEYWORD_OR: return "TOKEN_KEYWORD_OR";
        case TOKEN_KEYWORD_XOR: return "TOKEN_KEYWORD_XOR";
        case TOKEN_KEYWORD_NOT: return "TOKEN_KEYWORD_NOT";
        case TOKEN_KEYWORD_SHR: return "TOKEN_KEYWORD_SHR";
        case TOKEN_KEYWORD_SHL: return "TOKEN_KEYWORD_SHL";
        case TOKEN_KEYWORD_CMP: return "TOKEN_KEYWORD_CMP";
        case TOKEN_KEYWORD_JMP: return "TOKEN_KEYWORD_JMP";
        case TOKEN_KEYWORD_JE: return "TOKEN_KEYWORD_JE";
        case TOKEN_KEYWORD_JNE: return "TOKEN_KEYWORD_JNE";
        case TOKEN_KEYWORD_JL: return "TOKEN_KEYWORD_JL";
        case TOKEN_KEYWORD_JLE: return "TOKEN_KEYWORD_JLE";
        case TOKEN_KEYWORD_JG: return "TOKEN_KEYWORD_JG";
        case TOKEN_KEYWORD_JGE: return "TOKEN_KEYWORD_JGE";
        case TOKEN_KEYWORD_SETE: return "TOKEN_KEYWORD_SETE";
        case TOKEN_KEYWORD_SETNE: return "TOKEN_KEYWORD_SETNE";
        case TOKEN_KEYWORD_CALL: return "TOKEN_KEYWORD_CALL";
        case TOKEN_KEYWORD_RET: return "TOKEN_KEYWORD_RET";
        case TOKEN_KEYWORD_SYSCALL: return "TOKEN_KEYWORD_SYSCALL";
        case TOKEN_KEYWORD_BYTE: return "TOKEN_KEYWORD_BYTE";
        case TOKEN_KEYWORD_WORD: return "TOKEN_KEYWORD_WORD";
        case TOKEN_KEYWORD_DWORD: return "TOKEN_KEYWORD_DWORD";
        case TOKEN_KEYWORD_QWORD: return "TOKEN_KEYWORD_QWORD";
    }
    return token_tag_to_cstring(tag);
}

static string_view keyword_string_table[] =
{
    { .data = "mov",     .size = 3 },
    { .data = "ldr",     .size = 3 },
    { .data = "str",     .size = 3 },
    { .data = "add",     .size = 3 },
    { .data = "sub",     .size = 3 },
    { .data = "mul",     .size = 3 },
    { .data = "and",     .size = 3 },
    { .data = "or",      .size = 2 },
    { .data = "xor",     .size = 3 },
    { .data = "not",     .size = 3 },
    { .data = "shr",     .size = 3 },
    { .data = "shl",     .size = 3 },
    { .data = "cmp",     .size = 3 },
    { .data = "jmp",     .size = 3 },
    { .data = "je",      .size = 2 },
    { .data = "jne",     .size = 3 },
    { .data = "jl",      .size = 2 },
    { .data = "jle",     .size = 3 },
    { .data = "jg",      .size = 2 },
    { .data = "jge",     .size = 3 },
    { .data = "sete",    .size = 4 },
    { .data = "setne",   .size = 5 },
    { .data = "call",    .size = 4 },
    { .data = "ret",     .size = 3 },
    { .data = "syscall", .size = 7 },
    { .data = "byte",    .size = 4 },
    { .data = "word",    .size = 4 },
    { .data = "dword",   .size = 5 },
    { .data = "qword",   .size = 5 },
};

static int keyword_value_table[] =
{
    TOKEN_KEYWORD_MOV,
    TOKEN_KEYWORD_LDR,
    TOKEN_KEYWORD_STR,
    TOKEN_KEYWORD_ADD,
    TOKEN_KEYWORD_SUB,
    TOKEN_KEYWORD_MUL,
    TOKEN_KEYWORD_AND,
    TOKEN_KEYWORD_OR,
    TOKEN_KEYWORD_XOR,
    TOKEN_KEYWORD_NOT,
    TOKEN_KEYWORD_SHR,
    TOKEN_KEYWORD_SHL,
    TOKEN_KEYWORD_CMP,
    TOKEN_KEYWORD_JMP,
    TOKEN_KEYWORD_JE,
    TOKEN_KEYWORD_JNE,
    TOKEN_KEYWORD_JL,
    TOKEN_KEYWORD_JLE,
    TOKEN_KEYWORD_JG,
    TOKEN_KEYWORD_JGE,
    TOKEN_KEYWORD_SETE,
    TOKEN_KEYWORD_SETNE,
    TOKEN_KEYWORD_CALL,
    TOKEN_KEYWORD_RET,
    TOKEN_KEYWORD_SYSCALL,
    TOKEN_KEYWORD_BYTE,
    TOKEN_KEYWORD_WORD,
    TOKEN_KEYWORD_DWORD,
    TOKEN_KEYWORD_QWORD,
};

uint32 ir0_get_keywords(string_view **kst, int **kvt)
{
    *kst = keyword_string_table;
    *kvt = keyword_value_table;
    return ARRAY_COUNT(keyword_string_table);
}

int32 ir0_identifier_is_register(string_view s)
{
    if ((s.size == 2) && (s.data[0] == 'r') && ascii_is_digit(s.data[1]))
        return (s.data[1] - '0');
    if ((s.size == 3) && (s.data[0] == 'r') && ascii_is_digit(s.data[1]) && ascii_is_digit(s.data[2]))
        return (s.data[1] - '0') * 10 + (s.data[2] - '0');
    return -1;
}

bool32 ir0_parse_instruction(lexer *l)
{
    token ident = lexer_get_token(l);
    if (ident.tag == TOKEN_IDENTIFIER)
    {
        lexer_eat_token(l);
        token semicolon_or_argument = lexer_get_token(l);
        if (semicolon_or_argument.tag == ':')
        {
            printf("LABEL: "STRING_VIEW_FMT"\n", STRING_VIEW_ARG(ident.span));
        }
    }
    if (ident.tag | TOKEN_KEYWORD)
    {
        lexer_eat_token(l);
        token argument = lexer_get_token(l);
        if (argument.tag == TOKEN_IDENTIFIER)
        {
            int32 reg_number = ir0_identifier_is_register(argument.span);
            if (reg_number > -1)
            {
                lexer_eat_token(l);
                printf("INSTRUCTION: "STRING_VIEW_FMT" "STRING_VIEW_FMT"\n", STRING_VIEW_ARG(ident.span), STRING_VIEW_ARG(argument.span));
                /* WIP */
                #if 0
                token comma = lexer_get_token(l);
                if (comma.tag == ',')
                {
                    lexer_eat_token(l);
                    token size_qualifier_or_argument = lexer_get_token(l);
                    if ((size_qualifier_or_argument.tag == TOKEN_KEYWORD_BYTE) ||
                        (size_qualifier_or_argument.tag == TOKEN_KEYWORD_WORD) ||
                        (size_qualifier_or_argument.tag == TOKEN_KEYWORD_DWORD) ||
                        (size_qualifier_or_argument.tag == TOKEN_KEYWORD_QWORD))
                    {
                        lexer_eat_token(l);
                        token bracket_open = lexer_get_token(l);

                    }
                }
                else
                {
                    /* 1 argument */
                }
                #endif
            }
            else
            {
                /* 0 arguments */
            }
        }
    }
    return false;
}


/*

bool32 ir0_parse_address_calculation(lexer *l, ir0_address_calculation *addr)
{
    token open_bracket = eat_token(l);
    if (open_bracket.tag != '[')
    {
        printf("Effective address parsing error. There should be '['\n"
               "Why did you even called this function? Use 'get_token' to test for '[' before calling this function\n");
        return false;
    }

    int32 sign = 1;
    addr->c = 1;
    addr->r1 = -1;
    addr->r2 = -1;
    addr->a = 0;

    token t1 = get_token(l);
    if (t1.tag == Token_LiteralInteger)
    {
        eat_token(l);
        token s1 = get_token(l);
        if (s1.tag == '*')
        {
            eat_token(l);
            addr->c = t1.integer_value;
            token t2 = get_token(l);
            if (t2.tag == Token_Identifier)
            {
                // Continue
            }
            else
            {
                // Error
            }
        }
        else if (s1.tag == ']')
        {
            addr->a = t1.integer_value;
        }
        else
        {
            // Error
        }
    }

    {
        token t = get_token(l);
        if (t.tag == Token_Identifier)
        {
            eat_token(l);
            addr->r1 = ir0_identifier_is_register(t.span);
            token s = get_token(l);
            if (s.tag == '+' || s.tag == '-')
            {
                eat_token(l);
                if (s.tag == '-') sign = -1;
                // Continue
            }
            else if (s.tag == ']')
            {
                // Continue
            }
            else
            {
                // Error
            }
        }
    }

    {
        token t = get_token(l);
        if (t.tag == Token_Identifier)
        {
            eat_token(l);
            addr->r2 = ir0_identifier_is_register(t.span);
            token s = get_token(l);
            if (s.tag == '+' || s.tag == '-')
            {
                eat_token(l);
                if (s.tag == '-') sign = -1;
                // Continue
            }
            else if (s.tag == ']')
            {
                // Continue
            }
            else
            {
                // Error
            }
        }
    }

    {
        token t = get_token(l);
        if (t.tag == Token_LiteralInteger)
        {
            eat_token(l);
            addr->a = sign * t.integer_value;
        }
        else
        {
            // Error
        }
    }

    token bracket_close = eat_token(l);
    if (bracket_close.tag != ']')
    {
        printf("Effective address parsing error. There should be ']' or '+'\n");
        return false;
    }

    return true;
}


ir0_stream ir0_parse_text(char const *source, int32 source_size)
{
    lexer lexer =
    {
        .data = (uint8 *) source,
        .size = source_size,

        .cursor = 0,
        .line = 1,
        .column = 1,

        .keywords = malloc(sizeof(string_view) * ARRAY_COUNT(keywords)),
        .keyword_tags = malloc(sizeof(int32) * ARRAY_COUNT(keywords)),
        .keyword_count = ARRAY_COUNT(keywords),

        .is_valid_identifier_head = ir0_is_valid_identifier_head,
        .is_valid_identifier_body = ir0_is_valid_identifier_body,
    };

    for (uint32 keyword_index = 0; keyword_index < ARRAY_COUNT(keywords); keyword_index++)
    {
        lexer.keywords[keyword_index] = make_string_view_from_cstring(keywords[keyword_index].str);
        lexer.keyword_tags[keyword_index] = keywords[keyword_index].tag;
    }

    int32 stream_instruction_count = 1024;
    int32 stream_label_buffer_size = 1024;
    int32 stream_label_count = 32;
    ir0_stream stream =
    {
        .instructions = malloc(sizeof(ir0_instruction) * stream_instruction_count),
        .count  = 0,
        .capacity = stream_instruction_count,

        .label_buffer = malloc(stream_label_buffer_size),
        .label_buffer_size = 0,
        .label_buffer_capacity = stream_label_buffer_size,

        .labels = malloc(stream_label_count * sizeof(string_view)),
        .label_at = malloc(stream_label_count * sizeof(uint32)),
        .label_count = 0,
        .label_capacity = stream_label_count,
    };

    int error = 0;
    while (true)
    {
        token t = get_token(&lexer);
        if ((t.tag == Token_Invalid) || (t.tag == Token_Eof))
            break;

        if (t.tag == Token_Identifier)
        {
            // Label
            eat_token(&lexer); // eat label
            token t1 = get_token(&lexer); // get colon
            if (t1.tag == ':')
            {
                eat_token(&lexer); // eat colon
                int32 label_index = ir0_find_label(&stream, t.span);
                if (label_index >= 0)
                    stream.label_at[label_index] = stream.count;
                else
                    label_index = ir0_push_label_at(&stream, t.span, stream.count);
            }
        }
        else if ((t.tag & Token_Keyword) > 0)
        {
            // Instruction
            eat_token(&lexer);
            ir0_tag instruction_tag = Ir0_Unknown;
            for (uint32 keyword_index = 0; keyword_index < ARRAY_COUNT(keywords); keyword_index++)
            {
                string_view kw = make_string_view_from_cstring(keywords[keyword_index].str);
                if (string_view_equal(t.span, kw))
                {
                    instruction_tag = keywords[keyword_index].ir0;
                    break;
                }
            }

            int32 at_line = t.line;

            ir0_arg args[3] = {};
            uint32 count = 0;
            for (int i = 0; i < 3; i++)
            {
                token t1 = get_token(&lexer);
                if (t1.line != at_line) break;

                if (t1.tag == Token_Identifier ||
                    t1.tag == Token_LiteralInteger ||
                    t1.tag == '[')
                {
                    count += 1;
                    if (t1.tag == Token_Identifier)
                    {
                        eat_token(&lexer);
                        int32 is_reg = ir0_identifier_is_register(t1.span);
                        if (is_reg < 0)
                        {
                            args[i].tag = Ir0_ArgumentLabel;
                            int32 label_index = ir0_find_label(&stream, t1.span);
                            if (label_index < 0) label_index = ir0_push_label(&stream, t1.span);
                            args[i].u32 = label_index;
                        }
                        else
                        {
                            args[i].tag = Ir0_ArgumentRegister;
                            args[i].u32 = is_reg;
                        }
                    }
                    else if (t1.tag == Token_LiteralInteger)
                    {
                        eat_token(&lexer);
                        args[i].tag = Ir0_ArgumentImmediate;
                        args[i].u32 = t1.integer_value;
                    }
                    else if (t1.tag == '[')
                    {
                        ir0_address_calculation addr_calc = {};
                        bool32 success = ir0_parse_address_calculation(&lexer, &addr_calc);
                        if (success)
                        {
                            args[i].tag = Ir0_ArgumentAddress;
                            args[i].addr = addr_calc;
                        }
                        else
                        {
                            printf("Parser error: could not parse address calculation.\n");
                            error = 1;
                            break;
                        }
                    }

                    token t2 = get_token(&lexer);
                    if ((t2.line == at_line) && (t2.tag = ','))
                    {
                        eat_token(&lexer);
                    }
                }
                else
                {
                    printf("Could not parse argument for the instruction!\n");
                    printf("    token: " STRING_VIEW_FMT "\n", STRING_VIEW_PRINT(t1.span));
                    error = 1;
                    break;
                }
            }

            if (error) break;

            {
                ir0_instruction instruction = { .tag = instruction_tag };
                for (uint32 i = 0; i < count; i++)
                {
                    instruction.args[i] = args[i];
                }
                instruction.arg_count = count;
                stream.instructions[stream.count++] = instruction;
            }
        }
        else
        {
            break;
        }
    }

    if (error) stream.count = 0;
    return stream;
}
*/
