#include "lexer.h"


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
    lexer->cursor += 1;
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

void make_token_stream(struct lexer *lexer)
{
    while (lexer->token_count < lexer->token_stream_size)
    {
        consume_while(lexer, is_ascii_whitespace);

        struct token t = {};

        char c = get_char(lexer);
        if (c == 0)
        {
            t.type = TOKEN_EOF;
            break;
        }
        else if (is_valid_identifier_head(c))
        {
            t.type = TOKEN_IDENTIFIER;
            t.span.data = lexer->buffer + lexer->cursor;
            t.span.size = consume_while(lexer, is_valid_identifier_body);
        }
        else if (is_ascii_digit(c))
        {
            t.type = TOKEN_LITERAL_INT,
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
                t.type = TOKEN_ARROW_RIGHT;
                t.span.size = 2;
                eat_char(lexer);
            }
            else
            {
                t.type = (enum token_type) c;
                t.span.size = 1;
            }
        }

        lexer->token_stream[lexer->token_count++] = t;
    }
}

void debug_print_token(struct token t)
{
    switch (t.type)
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
        default: printf("Invalid!");
    }
}

void debug_print_token_stream(struct lexer *lexer)
{
    for (int i = 0; i < lexer->token_count; i++)
    {
        debug_print_token(lexer->token_stream[i]);
        printf("\n");
    }
}

int get_precedence(struct token operator)
{
    switch (operator.type)
    {
        case TOKEN_PLUS: return 1;
        case TOKEN_MINUS: return 1;
        case TOKEN_ASTERICS: return 2;
        case TOKEN_SLASH: return 2;
        default: return 0;
    }
    return 0;
}
