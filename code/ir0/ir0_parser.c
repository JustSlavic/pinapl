#include "ir0_parser.h"
#include <string_view.h>
#include <imparser.h>


enum
{
    TOKEN_KEYWORD_MOV = TOKEN_KEYWORD + 1,
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
    imparser_init(l);

    /* Safepoint. */
    lexer saved = *l;

    /* Try to parse label. */
    token ident = lexer_get_token(l);
    if (ident.tag == TOKEN_IDENTIFIER)
    {
        lexer_eat_token(l);
        token colon = lexer_get_token(l);
        if (colon.tag == ':')
        {
            lexer_eat_token(l);
            printf("LABEL: "STRING_VIEW_FMT"\n", STRING_VIEW_ARG(ident.span));
            return true;
        }
        else
        {
            printf("Parser Error: Recognized identifier at %d:%d (probably label), but it's not followed by the colon (:)\n"
                   "              Found something else at %d:%d instead.\n", ident.line, ident.column, colon.line, colon.column);
        }
    }

    /* Rollback */
    *l = saved;

    /* Try to parse instruction */
    token instruction_keyword;
    token bracket_open, bracket_close;

    instruction_keyword = lexer_get_token(l);
    if (instruction_keyword.tag & TOKEN_KEYWORD)
    {
        lexer_eat_token(l);
        token argument1 = lexer_get_token(l);
        if (argument1.tag == TOKEN_IDENTIFIER)
        {
            lexer_eat_token(l);
            int32 argument1_reg_number = ir0_identifier_is_register(argument1.span);
            if (argument1_reg_number > -1)
            {
                token comma = lexer_get_token(l);
                if (comma.tag == ',')
                {
                    lexer_eat_token(l);
                    token argument2 = lexer_get_token(l);
                    if (argument2.tag == TOKEN_IDENTIFIER)
                    {
                        lexer_eat_token(l);
                        int32 argument2_reg_number = ir0_identifier_is_register(argument2.span);
                        if (argument2_reg_number > -1)
                        {
                            comma = lexer_get_token(l);
                            if (comma.tag == ',')
                            {
                                lexer_eat_token(l);
                                token argument3 = lexer_get_token(l);
                                if (argument3.tag == TOKEN_IDENTIFIER)
                                {
                                    lexer_eat_token(l);
                                    int32 argument3_reg_number = ir0_identifier_is_register(argument3.span);
                                    if (argument3_reg_number > -1)
                                    {
                                        printf("INSTRUCTION: "STRING_VIEW_FMT_UNQUOTED" "STRING_VIEW_FMT_UNQUOTED", "STRING_VIEW_FMT_UNQUOTED", "STRING_VIEW_FMT_UNQUOTED"\n",
                                            STRING_VIEW_ARG(instruction_keyword.span),
                                            STRING_VIEW_ARG(argument1.span),
                                            STRING_VIEW_ARG(argument2.span),
                                            STRING_VIEW_ARG(argument3.span));
                                        return true;
                                    }
                                }
                            }
                            else
                            {
                                printf("INSTRUCTION: "STRING_VIEW_FMT_UNQUOTED" "STRING_VIEW_FMT_UNQUOTED", "STRING_VIEW_FMT_UNQUOTED"\n",
                                    STRING_VIEW_ARG(instruction_keyword.span),
                                    STRING_VIEW_ARG(argument1.span),
                                    STRING_VIEW_ARG(argument2.span));
                                return true;
                            }
                        }
                    }
                    else if (argument2.tag == TOKEN_LITERAL_INTEGER)
                    {
                        lexer_eat_token(l);
                        printf("INSTRUCTION: "STRING_VIEW_FMT_UNQUOTED" "STRING_VIEW_FMT_UNQUOTED", 0x%llx\n",
                            STRING_VIEW_ARG(instruction_keyword.span),
                            STRING_VIEW_ARG(argument1.span),
                            argument2.integer_value);
                        return true;
                    }
                    else if ((argument2.tag == TOKEN_KEYWORD_BYTE) ||
                             (argument2.tag == TOKEN_KEYWORD_WORD) ||
                             (argument2.tag == TOKEN_KEYWORD_DWORD) ||
                             (argument2.tag == TOKEN_KEYWORD_QWORD))
                    {
                        lexer_eat_token(l);
                        if (imparser_bracket_open(&bracket_open))
                        {
                            token address = lexer_get_token(l);
                            if (address.tag == TOKEN_LITERAL_INTEGER)
                            {
                                lexer_eat_token(l);
                                if (imparser_bracket_close(&bracket_close))
                                {
                                    printf("INSTRUCTION: "STRING_VIEW_FMT_UNQUOTED" "STRING_VIEW_FMT_UNQUOTED", "STRING_VIEW_FMT_UNQUOTED" [0x%llx]\n",
                                        STRING_VIEW_ARG(instruction_keyword.span),
                                        STRING_VIEW_ARG(argument1.span),
                                        STRING_VIEW_ARG(argument2.span), /* qualifier */
                                        address.integer_value);
                                    return true;
                                }
                            }
                        }
                    }
                    else
                    {
                        printf("Parser Error: Recognized comma at %d:%d, it should be followed by a [@todo: fill in what could go after],\n"
                               "but recognized something else at %d:%d\n", comma.line, comma.column, argument2.line, argument2.column);
                    }
                }
                else
                {
                    printf("INSTRUCTION: "STRING_VIEW_FMT_UNQUOTED" "STRING_VIEW_FMT_UNQUOTED"\n",
                        STRING_VIEW_ARG(instruction_keyword.span),
                        STRING_VIEW_ARG(argument1.span));
                    return true;
                }
            }
            else
            {
                /* 0 arguments */
                printf("INSTRUCTION: "STRING_VIEW_FMT_UNQUOTED"\n",
                    STRING_VIEW_ARG(instruction_keyword.span));
                return true;
            }
        }
        else
        {
            /* 0 arguments */
            printf("INSTRUCTION: "STRING_VIEW_FMT_UNQUOTED"\n",
                STRING_VIEW_ARG(instruction_keyword.span));
            return true;
        }
    }

    /* Rollback */
    *l = saved;

    printf("Parser Error: Could not recognize ir0 instruction\n");

    return false;
}
