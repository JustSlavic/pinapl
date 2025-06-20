#include "ir0_parser.h"

#include <stdlib.h>

typedef struct keyword_pair
{
    char const *str;
    int32       tag;
    int32       ir0;
} keyword_pair;

static keyword_pair keywords[] =
{
    { "mov", Token_KeywordMov, Ir0_Mov },
    { "ldr", Token_KeywordLdr, Ir0_Ldr },
    { "str", Token_KeywordStr, Ir0_Str },
    { "add", Token_KeywordAdd, Ir0_Add },
    { "sub", Token_KeywordSub, Ir0_Sub },
    { "mul", Token_KeywordMul, Ir0_Mul },
    { "div", Token_KeywordDiv, Ir0_Div },
    { "ret", Token_KeywordRet, Ir0_Ret },
    { "cmp", Token_KeywordCmp, Ir0_Cmp },
    { "jmp", Token_KeywordJmp, Ir0_Jmp },
    { "jeq", Token_KeywordJeq, Ir0_Jeq },
    { "jne", Token_KeywordJne, Ir0_Jne },
    { "jlt", Token_KeywordJlt, Ir0_Jlt },
    { "jle", Token_KeywordJle, Ir0_Jle },
    { "jgt", Token_KeywordJgt, Ir0_Jgt },
    { "jge", Token_KeywordJge, Ir0_Jge },
};

int32 ir0_identifier_is_register(string_view s)
{
    if ((s.size == 2) && (s.data[0] == 'r') && is_ascii_digit(s.data[1]))
    {
        return (s.data[1] - '0');
    }
    if ((s.size == 3) && (s.data[0] == 'r') && is_ascii_digit(s.data[1]) && is_ascii_digit(s.data[2]))
    {
        return (s.data[1] - '0') * 10 + (s.data[2] - '0');
    }
    return -1;
}

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
