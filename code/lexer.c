#include "lexer.h"


char *keywords[] = {
    "return",
    "bool",
    "true",
    "false",
};

enum token_type keyword_types[] = {
    TOKEN_KW_RETURN,
    TOKEN_KW_BOOL,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,
};

bool32 is_ascii_space(char c) { return (c == ' '); }
bool32 is_ascii_whitespace(char c) { return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'); }
bool32 is_ascii_alpha(char c) { return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')); }
bool32 is_ascii_digit(char c) { return (c >= '0') && (c <= '9'); }
bool32 is_valid_identifier_head(char c) { return (c == '_') || is_ascii_alpha(c); }
bool32 is_valid_identifier_body(char c) { return (c == '_') || is_ascii_alpha(c) || is_ascii_digit(c); }

char get_char(struct lexer *lexer)
{
    char c = 0;
    if (lexer->cursor < lexer->buffer_size) c = lexer->buffer[lexer->cursor];
    return c;
}

char eat_char(struct lexer *lexer)
{
    char c = get_char(lexer);
    if (c > 0)
    {
        lexer->cursor += 1;
        lexer->column += 1;

        if (c == '\n' || c == '\r')
        {
            lexer->column = 1;
            lexer->line += 1;
            char c2 = get_char(lexer);
            if ((c == '\n' && c2 == '\r') || (c == '\r' && c2 == '\n'))
            {
                lexer->cursor += 1;
            }
        }
    }
    return c;
}

int consume_while(struct lexer *lexer, bool32 (*predicate)(char))
{
    int count = 0;
    char c = get_char(lexer);
    while (predicate(c))
    {
        eat_char(lexer);
        count += 1;
        c = get_char(lexer);
    }
    return count;
}

int consume_until(struct lexer *lexer, bool32 (*predicate)(char))
{
    int count = 0;
    char c = get_char(lexer);
    while (!predicate(c))
    {
        eat_char(lexer);
        count += 1;
        c = get_char(lexer);
    }
    return count;
}

void lexer__tokenize(struct lexer *lexer)
{
    while (lexer->stream.count < lexer->stream.capacity)
    {
        consume_while(lexer, is_ascii_whitespace);

        token t = {
            .line = lexer->line,
            .column = lexer->column,
        };


        char c = get_char(lexer);
        if (c == 0)
        {
            t.kind = TOKEN_EOF;
            break;
        }
        else if (is_valid_identifier_head(c))
        {
            t.kind = TOKEN_IDENTIFIER;
            t.span.data = lexer->buffer + lexer->cursor;
            t.span.size = consume_while(lexer, is_valid_identifier_body);

            for (int keyword_index = 0; keyword_index < ARRAY_COUNT(keywords); keyword_index++)
            {
                char *keyword = keywords[keyword_index];
                if (cstring__compare_cn(keyword, t.span.data, t.span.size) == 0)
                {
                    t.kind = keyword_types[keyword_index];
                    break;
                }
            }
        }
        else if (is_ascii_digit(c))
        {
            t.kind = TOKEN_LITERAL_INT,
            t.span.data = lexer->buffer + lexer->cursor;

            isize integer_value = 0;
            while (is_ascii_digit(c))
            {
                integer_value *= 10;
                integer_value += (c - '0');
                eat_char(lexer);
                c = get_char(lexer);
                t.span.size += 1;
            }

            t.integer_value = integer_value;
        }
        else
        {
            t.span.data = lexer->buffer + lexer->cursor;

            eat_char(lexer);
            char c1 = get_char(lexer);

            if ((c == '-') && (c1 == '>'))
            {
                t.kind = TOKEN_ARROW_RIGHT;
                t.span.size = 2;
                eat_char(lexer);
            }
            else
            {
                t.kind = (enum token_type) c;
                t.span.size = 1;
            }
        }

        lexer->stream.tokens[lexer->stream.count++] = t;
    }
}

void debug_print_token(struct token t)
{
    switch (t.kind)
    {
        case TOKEN_EOF: printf("EOF"); break;
        case TOKEN_PAREN_OPEN: printf("("); break;
        case TOKEN_PAREN_CLOSE: printf(")"); break;

        case TOKEN_BRACKET_OPEN: printf("["); break;
        case TOKEN_BRACKET_CLOSE: printf("]"); break;

        case TOKEN_BRACE_OPEN: printf("{"); break;
        case TOKEN_BRACE_CLOSE: printf("}"); break;

        case TOKEN_EQUALS: printf("="); break;
        case TOKEN_COLON: printf(":"); break;
        case TOKEN_SEMICOLON: printf(";"); break;
        case TOKEN_PLUS: printf("+"); break;
        case TOKEN_MINUS: printf("-"); break;
        case TOKEN_ASTERICS: printf("*"); break;
        case TOKEN_SLASH: printf("/"); break;
        case TOKEN_COMMA: printf(","); break;

        case TOKEN_IDENTIFIER: printf("IDENTIFIER"); break;
        case TOKEN_LITERAL_INT: printf("LITERAL_INT"); break;

        case TOKEN_ARROW_RIGHT: printf("->"); break;

        case TOKEN_KW_RETURN: printf("return"); break;
        case TOKEN_KW_BOOL: printf("bool"); break;
        case TOKEN_KW_TRUE: printf("true"); break;
        case TOKEN_KW_FALSE: printf("false"); break;
        default: printf("Invalid!");
    }
}

void debug_print_token_stream(struct lexer *lexer)
{
    for (int i = 0; i < lexer->stream.count; i++)
    {
        debug_print_token(lexer->stream.tokens[i]);
        printf("\n");
    }
}

int get_precedence(struct token operator)
{
    switch (operator.kind)
    {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        default:
            return 0;
    }
    return 0;
}
