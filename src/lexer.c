#include "lexer.h"


char *token_type_to_cstring(token_type t)
{
    switch (t)
    {
        case TOKEN_INVALID: return "TOKEN_INVALID";
        case TOKEN_PAREN_OPEN: return "TOKEN_PAREN_OPEN";
        case TOKEN_PAREN_CLOSE: return "TOKEN_PAREN_CLOSE";
        case TOKEN_BRACKET_OPEN: return "TOKEN_BRACKET_OPEN";
        case TOKEN_BRACKET_CLOSE: return "TOKEN_BRACKET_CLOSE";
        case TOKEN_BRACE_OPEN: return "TOKEN_BRACE_OPEN";
        case TOKEN_BRACE_CLOSE: return "TOKEN_BRACE_CLOSE";
        case TOKEN_EQUALS: return "TOKEN_EQUALS";
        case TOKEN_COLON: return "TOKEN_COLON";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_INT_LITERAL: return "TOKEN_INT_LITERAL";
        case TOKEN_KW_RETURN: return "TOKEN_KW_RETURN";
        case TOKEN_EOF: return "TOKEN_EOF";
    }
    return 0;
}

bool is_ascii_space(char c)
{
    return (c == ' ');
}

bool is_ascii_whitespace(char c)
{
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

bool is_ascii_alpha(char c)
{
    return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}

bool is_ascii_digit(char c)
{
    return (c >= '0') && (c <= '9');
}

bool is_valid_identifier_head(char c)
{
    return (c == '_') || is_ascii_alpha(c);
}

bool is_valid_identifier_body(char c)
{
    return (c == '_') || is_ascii_alpha(c) || is_ascii_digit(c);
}


char lexer_get_char(lexer *l)
{
    char c = 0;
    if (l->index < l->buffer_size)
    {
        c = l->buffer[l->index];
    }
    return c;
}

char lexer_eat_char(lexer *l)
{
    char c = lexer_get_char(l);
    l->index += 1;
    if (c == '\n')
    {
        l->line += 1;
        l->column = 0;
    }
    l->column += 1;
    return c;
}

void consume_while(lexer *l, predicate *p)
{
    char c = lexer_get_char(l);
    while (p(c))
    {
        lexer_eat_char(l);
        c = lexer_get_char(l);
    }
}

void consume_until(lexer *l, predicate *p)
{
    char c = lexer_get_char(l);
    while (!p(c))
    {
        lexer_eat_char(l);
        c = lexer_get_char(l);
    }
}

token lexer_get_token(lexer *l)
{
    if (!l->next_token_valid)
    {
        consume_while(l, is_ascii_whitespace);
       
        token t = {0};

        char c = lexer_get_char(l);
        if (c == 0)
        {
            t.type = TOKEN_EOF;
            t.line = l->line;
            t.column = l->column;
            t.span = l->buffer + l->index;
            t.span_size = 1;
        }
        else if (is_valid_identifier_head(c))
        {
            t.type = TOKEN_IDENTIFIER;
            t.line = l->line;
            t.column = l->column;
            t.span = l->buffer + l->index;

            lexer_eat_char(l);
            c = lexer_get_char(l);
            int span_size = 1;

            while (is_valid_identifier_body(c))
            {
                lexer_eat_char(l);
                c = lexer_get_char(l);
                span_size += 1;
            }

            t.span_size = span_size;
        }

        l->next_token = t;
        l->next_token_valid = true;
    }

    return l->next_token;
}

token lexer_eat_token(lexer *l)
{
    token result = lexer_get_token(l);
    l->next_token_valid = false;
    return result;
}


