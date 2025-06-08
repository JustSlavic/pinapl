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
        .label_buffer_count = 0,
        .label_buffer_capacity = stream_label_buffer_size,
        .label_index_in_buffer = malloc(stream_label_count * sizeof(uint32)),
        .label_at = malloc(stream_label_count * sizeof(uint32)),
        .label_count = 0,
        .label_capacity = stream_label_count,
    };

    while (true)
    {
        token t = get_token(&lexer);
        if ((t.tag == Token_Invalid) || (t.tag == Token_Eof))
            break;

        printf("   token " STRING_VIEW_FMT "\n", STRING_VIEW_PRINT(t.span));

        if ((t.tag == Token_Identifier) || (t.tag == '.'))
        {
            // Label
            if (t.tag == Token_Identifier)
            {
                eat_token(&lexer); // eat label
                token t1 = get_token(&lexer); // get colon
                if (t1.tag == ':')
                {
                    eat_token(&lexer); // eat colon
                    printf("Label 1\n");
                    // ir0_push_label_string_view(&stream, t.span);
                }
            }
            if (t.tag == '.')
            {
                eat_token(&lexer); // eat period
                token t1 = get_token(&lexer); // get label
                if ((t1.tag == Token_Identifier) && ((t.column + 1) == t1.column))
                {
                    eat_token(&lexer); // eat label
                    token t2 = get_token(&lexer); // get colon
                    if (t2.tag == ':')
                    {
                        eat_token(&lexer); // eat colon
                        printf("Label 2\n");
                        // ir0_push_label_string_view(&stream, t.span);
                    }
                }
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

            {
                ir0_instruction instruction = { .tag = instruction_tag };
                for (uint32 i = 0; i < count; i++)
                {
                    instruction.args[0] = args[0];
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

    return stream;
}
